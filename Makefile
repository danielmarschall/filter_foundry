# This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
# Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

# mingw32 Makefile

CC = i386-mingw32msvc-gcc
DLLWRAP = i386-mingw32msvc-dllwrap
WINDRES = i386-mingw32msvc-windres
#CC = mingw32-gcc
#DLLWRAP = mingw32-dllwrap
#WINDRES = mingw32-windres

# use GNU flex and bison
# these lines can be commented to use system lex and yacc
# although this may result in a larger overall executable
LEX = flex
YACC = bison -y

YFLAGS = -d

#PSAPI = "c:/program files/adobe/adobe photoshop 6.0 sdk/photoshopapi"
PSAPI = "../PhotoshopAPI"
CFLAGS += -O2 -DWIN_ENV \
	-I$(PSAPI)/Pica_sp -I$(PSAPI)/Photoshop -I$(PSAPI)/General \
	-I../common/adobeplugin -I../common/tt

OBJ = main.o ui_win.o ui.o ui_build.o ui_build_win.o preview.o read.o save.o \
	parser.o lexer.o node.o symtab.o funcs.o trigtab.o process.o \
	load_win.o make_win.o make.o scripting.o \
	../common/tt/str.o ../common/tt/sprintf_tiny.o \
	../common/tt/ui_compat_win.o ../common/tt/file_compat_win.o \
	../common/tt/choosefile_win.o ../common/tt/compat_string.o \
	../common/tt/compat_win.o \
	../common/tt/dbg_win.o ../common/adobeplugin/dllmain.o \
	res.o

EXEC = dist/FilterFoundry.8bf
DISTARCHIVE = dist/FilterFoundry-win.zip
SRCARCHIVE = dist/FilterFoundry-src.zip

all : $(EXEC)

funcs.o : ff.h funcs.h symtab.h ui.h PARM.h parser.c
lexer.o : node.h symtab.h parser.c
load_mac.o : ff.h funcs.h symtab.h ui.h PARM.h
load_win.o : ff.h funcs.h symtab.h ui.h PARM.h
main.o : ff.h funcs.h symtab.h ui.h PARM.h node.h parser.h scripting.h
make.o : ff.h funcs.h symtab.h ui.h PARM.h
make_mac.o : ff.h funcs.h symtab.h ui.h PARM.h
make_win.o : ff.h funcs.h symtab.h ui.h PARM.h
node.o : node.h parser.c funcs.h symtab.h ui.h PARM.h
parser.o : node.h parser.h
preview.o : ff.h funcs.h symtab.h ui.h PARM.h node.h parser.h parser.c
process.o : ff.h funcs.h symtab.h ui.h PARM.h node.h parser.h
read.o : ff.h funcs.h symtab.h ui.h PARM.h
save.o : ff.h funcs.h symtab.h ui.h PARM.h
scripting.o : ff.h funcs.h symtab.h ui.h PARM.h scripting.h
symtab.o : symtab.h
ui.o : ff.h funcs.h symtab.h ui.h PARM.h node.h parser.h parser.c
ui_build.o : ff.h funcs.h symtab.h ui.h PARM.h
ui_build_win.o : ff.h funcs.h symtab.h ui.h PARM.h version.h
ui_mac.o : ff.h funcs.h symtab.h ui.h PARM.h
ui_win.o : ff.h funcs.h symtab.h ui.h PARM.h version.h

parser.o : node.h parser.h

clean :
	rm -f $(OBJ) $(EXEC) parser.c lexer.c $(DISTARCHIVE) $(SRCARCHIVE)

dist : $(DISTARCHIVE)

$(DISTARCHIVE) : $(EXEC) dist/README.txt dist/COPYING.txt dist/examples
	zip -9 -r $@ $^
	ls -l $@

src : $(SRCARCHIVE)

$(SRCARCHIVE) : \
		dist/README.txt dist/COPYING.txt dist/examples BUILDING.txt \
		Makefile FilterFoundry.make cw7/filterfoundry_cw7.mcp \
		funcs.c gentab.c load_mac.c load_win.c main.c make.c make_mac.c make_win.c \
		node.c preview.c process.c read.c save.c symtab.c trigtab.c \
		ui.c ui_build.c ui_build_win.c ui_mac.c ui_win.c scripting.[chr] \
		lexer.[lc] parser.[yc] y.tab.h \
		ff.h funcs.h node.h PARM.h parser.h resource.h symtab.h \
		ui.h version.h \
		68k.r carbon.r classic.r universal.r pipl_common.r ui_mac.r  \
		PiPL.rc PiPL_body.rc ui_win.rc win_res.rc \
		caution.ico zoomin.ico zoomout.ico \
		../common/tt/str.[ch] ../common/tt/sprintf_tiny.[ch] \
		../common/tt/dlg.[ch] ../common/tt/menu.[ch] \
		../common/tt/ui_compat.h ../common/tt/ui_compat_win.c ../common/tt/ui_compat_mac.c \
		../common/tt/file_compat.h ../common/tt/file_compat_win.c \
		../common/tt/compat_string.[ch] \
		../common/tt/compat_win.[ch] \
		../common/tt/dbg.h ../common/tt/dbg_mac.c ../common/tt/dbg_win.c \
		../common/tt/carbonstuff.h \
		../common/tt/choosefile.h ../common/tt/choosefile_win.c ../common/tt/choosefile_nav.c \
		../common/adobeplugin/dllmain.c ../common/adobeplugin/entry.h ../common/adobeplugin/world.h
	zip -r -9 $@ $^ -x \*/.DS_Store \*/.FBC\*
	ls -l $@

$(EXEC) : exports.def $(OBJ)
	$(DLLWRAP) -o $@ -def exports.def $(OBJ) -mwindows -s -Xlinker -M > dist/FilterFoundry.map
	date
	ls -l $@

test : $(EXEC)
	cp $(EXEC) ../../pcshare

res.o : win_res.rc PiPL.rc PiPL_body.rc ui_win.rc caution.ico
	$(WINDRES) --language=0 --include-dir=$(PSAPI)/Photoshop --include-dir=$(PSAPI)/Pica_sp -i win_res.rc -o $@

lexer.o : lexer.l parser.c

parser.o : parser.y

gentab : gentab.c funcs.h
	gcc -o gentab gentab.c -lm

trigtab.c : gentab
	./gentab > $@

#CC=gcc
#CFLAGS=-DDEBUG -DPARSERTEST
parsertest : lexer.o parser.o node.o testmain.o symtab.o funcs.o
	$(CC) -o $@ $^ -lm
