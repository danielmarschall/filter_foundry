# This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
# Copyright (C) 2003-7 Toby Thain, toby@telegraphics.com.au

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License  
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# GNU Makefile
# builds Win32 DLL and CS2/Mac Mach-O plugin bundle

# by Toby Thain <toby@telegraphics.com.au>


# ---------- variables & flags ----------

EXEC = FilterFoundry

VERSION = $(shell perl -n -e 'm/^.*VERSION_STR[[:blank:]]+\"([^"]*)\"/ && print $$1;' version.h)

MINGW_CC = i386-mingw32msvc-gcc
DLLWRAP  = i386-mingw32msvc-dllwrap
WINDRES  = i386-mingw32msvc-windres

# use GNU flex and bison
# these lines can be commented to use system lex and yacc
# although this may result in a larger overall executable
LEX = flex
YACC = bison -y
YFLAGS = -d

PSAPI = "../Adobe Photoshop CS3 Public Beta SDK/photoshopapi"

CFLAGS += -O2 -W -Wall -Wno-main -Wno-unused-parameter -Wno-multichar
CPPFLAGS += -DYY_SKIP_YYWRAP \
            -I$(PSAPI)/pica_sp -I$(PSAPI)/photoshop -I$(PSAPI)/general \
            -I../common/adobeplugin -I../common/tt


# ---------- source & object files ----------

# where to find .c source files
vpath %.c ../common/tt ../common/adobeplugin ../MoreFiles/Sources

# list of source files
SRC_COMMON = main.c funcs.c process.c node.c symtab.c \
	ui.c ui_build.c preview.c read.c save.c make.c \
	scripting.c y.tab.c lex.yy.c str.c
SRC_OSX = dbg_mac.c ui_mac.c make_mac.c load_mac.c ui_compat_mac.c \
	choosefile_nav.c preview_mac.c \
	MoreFilesExtras.c MoreFiles.c MoreDesktopMgr.c FileCopy.c Search.c
SRC_W32 = dbg_win.c ui_win.c make_win.c versioninfo_modify_win.c load_win.c ui_compat_win.c \
	choosefile_win.c ui_build_win.c compat_string.c compat_win.c \
	file_compat_win.c dllmain.c

# derive lists of object files, separate for each platform
OBJ_OSX := $(patsubst %.c, obj/%.o,     $(SRC_COMMON) $(SRC_OSX))
OBJ_W32 := $(patsubst %.c, obj_w32/%.o, $(SRC_COMMON) $(SRC_W32)) obj_w32/res.o


# ---------- executables ----------

# parts of Mac OS X plugin bundle to build
# Adobe's plugs use .plugin extension
BUNDLE = $(EXEC).plugin
PLUGIN_OSX  = $(BUNDLE)/Contents/MacOS/$(EXEC)
PLUGIN_RSRC = $(BUNDLE)/Contents/Resources/$(EXEC).rsrc
PLUGIN_PARTS = $(PLUGIN_OSX) $(PLUGIN_RSRC) $(BUNDLE)/Contents/Info.plist $(BUNDLE)/Contents/PkgInfo
DISTDMG = dist/$(EXEC)-$(VERSION).dmg

$(PLUGIN_OSX) : CPPFLAGS += -DMAC_ENV -DMACMACHO -Dmacintosh \
	-I/Developer/Headers/FlatCarbon \
	-I../MoreFiles/CHeaders -I../MoreFiles/Sources

# Win32 plugin DLL to build
PLUGIN_W32 = $(EXEC).8bf
DISTZIP = dist/$(EXEC)-$(VERSION)-win.zip

$(PLUGIN_W32) : CPPFLAGS += -DWIN_ENV


# ---------- targets ----------

# build everything
all : dll osx

dll : $(PLUGIN_W32)

osx fat : $(BUNDLE) $(PLUGIN_PARTS)

# See: http://developer.apple.com/documentation/Porting/Conceptual/PortingUnix/compiling/chapter_4_section_3.html#//apple_ref/doc/uid/TP40002850-BAJCFEBA
fat : CFLAGS += -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386
fat : LDFLAGS += -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386
fat : REZFLAGS += -arch ppc -arch i386

$(BUNDLE) :
	mkdir -p $@
	/Developer/Tools/SetFile -a B $@

# insert correct executable name and version string in bundle's Info.plist
$(BUNDLE)/Contents/Info.plist : Info.plist $(BUNDLE) version.h
	mkdir -p $(dir $@)
	V=`sed -n -E 's/^.*VERSION_STR[[:blank:]]+\"([^"]*)\"/\1/p' version.h` ;\
		sed -e s/VERSION_STR/$$V/ -e s/EXEC/$(EXEC)/ $< > $@

$(BUNDLE)/Contents/PkgInfo : $(BUNDLE)
	mkdir -p $(dir $@)
	echo -n 8BFM8BIM > $@

clean :
	rm -fr *.[ox] $(OBJ_OSX) $(OBJ_W32) $(PLUGIN_W32) $(BUNDLE) \
	       lex.yy.[ch] y.tab.[ch] temp


dmg : $(DISTDMG)

# create an Apple disk image (dmg) archive of the distribution kit
$(DISTDMG) : $(PLUGIN_PARTS) dist/README.html dist/gpl.html
	@ DIR=`mktemp -d $(EXEC)-XXXX`; \
	cp -Rp dist/README.html dist/gpl.html $(BUNDLE) $$DIR; \
	mkdir -p $$DIR/examples; \
	cp dist/examples/*.afs $$DIR/examples; \
	/Developer/Tools/SetFile -t TEXT -c ttxt $$DIR/examples/*; \
	hdiutil create -srcfolder $$DIR -ov -volname "$(EXEC) $(VERSION)" $@; \
	rm -fr $$DIR
	@ ls -l $@


zip : $(DISTZIP)

$(DISTZIP) : $(PLUGIN_W32) dist/README.html dist/gpl.html dist/examples/*.afs
	T=`mktemp -d`; \
		D=$$T/FilterFoundry-$(VERSION); \
		mkdir -p $$D/examples; \
		cp $^ $$D; \
		mv $$D/*.afs $$D/examples; \
		cd $$T; \
		zip -9 -r temp.zip FilterFoundry-$(VERSION); \
		mv temp.zip $(PWD)/$@; \
		rm -fr $$T
	ls -l $@

dist/gpl.html : 
	curl http://www.gnu.org/licenses/gpl.html | \
		sed -e 's%</HEAD>%<BASE HREF="http://www.gnu.org/"> </HEAD>%' \
			-e 's%</head>%<BASE HREF="http://www.gnu.org/"> </HEAD>%' > $@


# ---------- compile rules ----------

obj/%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS) -fpascal-strings
obj_w32/%.o : %.c
	$(MINGW_CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)

# note dependencies on version.h:

obj_w32/res.o : win_res.rc PiPL.rc PiPL_body.rc manifest.rc version_win.rc ui_win.rc caution.ico ui.h version.h
	$(WINDRES) -o $@ -i $< --language=0 $(CPPFLAGS)

lex.yy.c : lexer.l y.tab.h
	$(LEX) $<
y.tab.c y.tab.h : parser.y
	$(YACC) $< $(YFLAGS)

obj_w32/funcs.o : ff.h funcs.h symtab.h ui.h PARM.h y.tab.h
obj_w32/lex.yy.o : node.h symtab.h y.tab.h
obj_w32/load_win.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/main.o : ff.h funcs.h symtab.h ui.h PARM.h node.h scripting.h y.tab.h 
obj_w32/make.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/make_win.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/versioninfo_modify_win.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/node.o : node.h y.tab.h funcs.h symtab.h ui.h PARM.h
obj_w32/y.tab.o : node.h y.tab.h
obj_w32/preview.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj_w32/process.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj_w32/read.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/save.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/scripting.o : ff.h funcs.h symtab.h ui.h PARM.h scripting.h
obj_w32/symtab.o : symtab.h
obj_w32/ui.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj_w32/ui_build.o : ff.h funcs.h symtab.h ui.h PARM.h
obj_w32/ui_build_win.o : ff.h funcs.h symtab.h ui.h PARM.h version.h
obj_w32/ui_win.o : ff.h funcs.h symtab.h ui.h PARM.h version.h

obj/funcs.o : ff.h funcs.h symtab.h ui.h PARM.h y.tab.h
obj/lex.yy.o : node.h symtab.h y.tab.h
obj/load_mac.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/main.o : ff.h funcs.h symtab.h ui.h PARM.h node.h scripting.h y.tab.h 
obj/make.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/make_mac.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/node.o : node.h y.tab.h funcs.h symtab.h ui.h PARM.h
obj/y.tab.o : node.h symtab.h y.tab.h
obj/preview.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj/process.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj/read.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/save.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/scripting.o : ff.h funcs.h symtab.h ui.h PARM.h scripting.h
obj/symtab.o : symtab.h
obj/ui.o : ff.h funcs.h symtab.h ui.h PARM.h node.h y.tab.h
obj/ui_build.o : ff.h funcs.h symtab.h ui.h PARM.h
obj/ui_mac.o : ff.h funcs.h symtab.h ui.h PARM.h

# compile Mac resources (into data fork of .rsrc file)
$(PLUGIN_RSRC) : $(BUNDLE) PiPL_macho.r ui_mac.r scripting.r ui.h version.h
	mkdir -p $(dir $@)
	/Developer/Tools/Rez -o $@ -useDF $(REZFLAGS) $(filter %.r,$^) \
		-i /Developer/Headers/FlatCarbon \
		-i $(PSAPI)/Resources \
		-i $(PSAPI)/Photoshop
	ls -l $@


# ---------- link rules ----------

# link OS X Mach-O executable
$(PLUGIN_OSX) : $(BUNDLE) exports.exp $(OBJ_OSX)
	mkdir -p $(dir $@)
	$(CC) -bundle -o $@ $(OBJ_OSX) \
		$(LDFLAGS) -exported_symbols_list exports.exp \
		-framework Carbon -framework System
	ls -l $@
	file $@

# link Win32 DLL
$(PLUGIN_W32) : exports.def $(OBJ_W32)
	$(DLLWRAP) -o $@ -def $^ -mwindows -s
	ls -l $@


# --------------------
