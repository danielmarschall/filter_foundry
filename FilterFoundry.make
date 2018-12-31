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

## MPW Makefile

EXEC = FilterFoundry

EXEC_UNI = :dist:{EXEC}

# Macintosh type/creator code for plugin file
PLUGINTYPE = 8BFM
PLUGINCREATOR = 8BIM
FILETYPE = -t ¶'{PLUGINTYPE}¶' -c ¶'{PLUGINCREATOR}¶'

# variables controlling fat build
PLUGIN68K = #68k.rsrc # uncomment this VALUE (after =) to build 68K (fat)
BUILD68K = #-d BUILD68K # uncomment this VALUE (after =) to build 68K (fat)

# these two variables reflect my personal directory layout
# and are not used by distribution
COMMONA = ::common:adobeplugin: 
COMMONT = ::common:tt:

# where the Photoshop API header files are located
PSAPI = ::PhotoshopAPI:
PSHEADERS = {PSAPI}Photoshop:,{PSAPI}Pica_sp:,{PSAPI}Resources:
REZHEADERS = -i {PSAPI}Photoshop: -i {PSAPI}Resources:

# list object files in project
# each kind of object file (Classic, Carbon, 68K) has its own list,
# of the same series of input files, but with the respective extension (.cl.x, .ca.x, .o) to differentiate
# "implicit" rules are used to build these object files from corresponding .c source; see below

OBJ_CLASSIC = main.cl.x ui_mac.cl.x ui.cl.x ui_build.cl.x ¶
	preview.cl.x read.cl.x save.cl.x preview_mac.cl.x make_mac.cl.x load_mac.cl.x make.cl.x scripting.cl.x ¶
	y.tab.cl.x lex.yy.cl.x node.cl.x symtab.cl.x funcs.cl.x process.cl.x ¶
	{COMMONT}dbg_mac.cl.x {COMMONT}str.cl.x {COMMONT}ui_compat_mac.cl.x ¶
	{COMMONT}choosefile_nav.cl.x ¶
	::MoreFiles:Sources:MoreFilesExtras.cl.x ¶
	::MoreFiles:Sources:MoreFiles.cl.x ¶
	::MoreFiles:Sources:MoreDesktopMgr.cl.x ¶
	::MoreFiles:Sources:FileCopy.cl.x

OBJ_CARBON =  main.ca.x ui_mac.ca.x ui.ca.x ui_build.ca.x ¶
	preview.ca.x read.ca.x save.ca.x preview_mac.ca.x make_mac.ca.x load_mac.ca.x make.ca.x scripting.ca.x ¶
	y.tab.ca.x lex.yy.ca.x node.ca.x symtab.ca.x funcs.ca.x process.ca.x ¶
	{COMMONT}dbg_mac.ca.x {COMMONT}str.ca.x {COMMONT}ui_compat_mac.ca.x ¶
	{COMMONT}choosefile_nav.ca.x ¶
	::MoreFiles:Sources:MoreFilesExtras.ca.x ¶
	::MoreFiles:Sources:MoreFiles.ca.x ¶
	::MoreFiles:Sources:MoreDesktopMgr.ca.x ¶
	::MoreFiles:Sources:FileCopy.ca.x

OBJ_68K = main.68k.o #{COMMONA}dbg_mac.o {COMMONT}str.o {COMMONT}sprintf_tiny.o
REZFILES = ui_mac.r scripting.r

# this is a hack to work around a missing UNIX include
'sys/cdefs.h' Ä FilterFoundry.make
	echo '#define __IDSTRING(a,b)' > 'sys/cdefs.h'
	echo '#define __P(protos)     protos          /* full-blown ANSI C */' >> 'sys/cdefs.h'

y.tab.c Ä parser.y 
	echo "y.tab.c is out of date; in UNIX, 'make y.tab.c' before retrying MPW build"
lex.yy.c Ä lexer.l
	echo "lex.yy.c is out of date; in UNIX, 'make lex.yy.c' before retrying MPW build"

y.tab.cl.x ÄÄ 'sys/cdefs.h'
y.tab.ca.x ÄÄ 'sys/cdefs.h'

funcs.cl.x ÄÄ y.tab.h
funcs.ca.x ÄÄ y.tab.h
lex.yy.cl.x ÄÄ y.tab.h
lex.yy.ca.x ÄÄ y.tab.h
node.cl.x ÄÄ y.tab.h
node.ca.x ÄÄ y.tab.h
preview.cl.x ÄÄ y.tab.h
preview.ca.x ÄÄ y.tab.h
ui.cl.x ÄÄ y.tab.h
ui.ca.x ÄÄ y.tab.h

# look for include files in these directories
ACCESSPATHS = -i :,{PSHEADERS},"{COMMONT}","{COMMONA}",::MoreFiles:CHeaders:,::MoreFiles:Sources:

# executable files to build (the Classic executable will be a fat binary containing both PPC and 68K code)
EXEC_CLASSIC = :debug:{EXEC}.classic
EXEC_CARBON = :debug:{EXEC}.carbon

# build "universal" plugin with both Classic and Carbon binaries in one file 
{EXEC_UNI} ÄÄ {EXEC_CARBON} {EXEC_CLASSIC} PiPL_universal.r PiPL_common.r version.h
	catenate {EXEC_CARBON} {EXEC_CLASSIC} > temp && duplicate -y -d temp {EXEC_UNI} # copy just data fork, without junk resources
	ls -x b {EXEC_CARBON} {EXEC_CLASSIC} > temp
	target temp
	find ¥
	find /[0-9]+/ && set CARBON_LENGTH `catenate "{target}".¤`
	find /[0-9]+/ && set CLASSIC_LENGTH `catenate "{target}".¤`
	close
	Rez PiPL_universal.r -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS}  ¶
		-d CARBON_LENGTH={CARBON_LENGTH} -d CLASSIC_LENGTH={CLASSIC_LENGTH}

{EXEC} Ä {EXEC_UNI}
	files -x brtc {EXEC_UNI}

# libraries linked with project
LIBS_CLASSIC = ¶
		"{SharedLibraries}InterfaceLib" ¶
		"{SharedLibraries}StdCLib" ¶
		"{SharedLibraries}MathLib" ¶
		"{SharedLibraries}NavigationLib" ¶
		"{SharedLibraries}AppearanceLib" ¶
		"{SharedLibraries}WindowsLib" ¶
		"{SharedLibraries}ControlsLib" ¶
		"{PPCLibraries}StdCRuntime.o" ¶
		"{PPCLibraries}PPCCRuntime.o" ¶
		#"{PPCLibraries}"CarbonAccessors.o
		#"{PPCLibraries}PPCToolLibs.o" ¶

LIBS_CARBON = ¶
		"{SharedLibraries}CarbonLib" ¶
		"{SharedLibraries}StdCLib" ¶
		"{PPCLibraries}PPCCRuntime.o"

LIBS_68K = ¶
		"{Libraries}MathLib.o" ¶
		#"{CLibraries}Complex.o" ¶
		"{CLibraries}StdCLib.o" ¶
		"{Libraries}MacRuntime.o" ¶
		#"{Libraries}IntEnv.o" ¶
		#"{Libraries}ToolLibs.o" ¶
		"{Libraries}Interface.o"

### little below should need to change ###

COPT = -tb on -opt speed -d MAC_ENV {ACCESSPATHS} -proto strict -w 2,35 ¶
	-d YY_USE_PROTOS #-typecheck relaxed -w 2,17,29,35
	
PPCLINKOPTS = -t {PLUGINTYPE} -c {PLUGINCREATOR} -m main
REZOPTS = -a -s : -d SystemSevenOrLater=1

# implicit build rules for each variety of object file
.ca.x Ä .c # Carbon PowerPC object
	mrc {depdir}{default}.c -o {targ} {copt} -d TARGET_CARBON
.cl.x Ä .c # Classic PowerPC object
	mrc {depdir}{default}.c -o {targ} {copt}
.68k.o Ä .c # 68K object
	sc {depdir}{default}.c -o {targ} -b {copt}

{EXEC_CLASSIC} ÄÄ {OBJ_CLASSIC}
	PPCLink -o {Targ} {PPCLINKOPTS} {OBJ_CLASSIC} {LIBS_CLASSIC}
{EXEC_CARBON} ÄÄ {OBJ_CARBON}
	PPCLink -o {Targ} {PPCLINKOPTS} -d {OBJ_CARBON} {LIBS_CARBON}

# put 68K code resource in intermediate file
68k.rsrc ÄÄ {OBJ_68K}
	Link -o {TARG} -t 'rsrc' -c 'RSED' -sg {EXEC} -rt {PLUGINTYPE}=16000 -m MAIN {OBJ_68K} {LIBS_68K}
	rezdet -l {targ}

{EXEC_CLASSIC} ÄÄ PiPL_classic.r PiPL_common.r version.h {PLUGIN68K}
	Rez PiPL_classic.r -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS}  {BUILD68K}
{EXEC_CARBON} ÄÄ PiPL_carbon.r PiPL_common.r version.h
	Rez PiPL_carbon.r -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS} 

{EXEC_CLASSIC} ÄÄ {REZFILES} ui.h version.h 
	Rez {REZFILES} -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS}
{EXEC_CARBON} ÄÄ {REZFILES} ui.h version.h 
	Rez {REZFILES} -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS}
{EXEC_UNI} ÄÄ {REZFILES} ui.h version.h 
	Rez {REZFILES} -o {Targ} {FILETYPE} {REZHEADERS} {REZOPTS}
