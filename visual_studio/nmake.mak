# This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
# Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
# Copyright (C) 2018-2019 Daniel Marschall, ViaThinkSoft

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

# NMAKE Makefile

# Tested with Visual C++ 2008 Express Edition
#	http://www.microsoft.com/express/vc/
# and Windows SDK for Windows Server 2008
#	http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en

# Set environment variables as follows (fix paths for
# *your* actual installations and ignore line breaks :)
#	SET INCLUDE=H:\Microsoft Visual Studio 9.0\VC\include;
#               C:\Program Files\Microsoft SDKs\Windows\v6.1\Include
#	SET LIB=H:\Microsoft Visual Studio 9.0\Common7\IDE;
#           H:\Microsoft Visual Studio 9.0\VC\lib;
#           C:\Program Files\Microsoft SDKs\Windows\v6.1\Lib
#	SET PATH=H:\Microsoft Visual Studio 9.0\Common7\IDE;
#            H:\Microsoft Visual Studio 9.0\VC\bin;
#            C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin

# To do a 64-bit build,
# - set LIB to {VC}\lib\amd64;Microsoft SDKs\Windows\v6.1\Lib\x64
#   and PATH to {VC}\bin\x86_amd64;{VC}\bin
#   where {VC} is the vc directory of the Visual Studio distribution.
# - add  -d_WIN64  to RFLAGS

EXEC = FilterFoundry

# define location of Photoshop SDK headers
PSAPI = ..\photoshop_sdk\pluginsdk\photoshopapi

# C compiler flags
CPPFLAGS = -DWIN32 -DWIN_ENV -DYY_SKIP_YYWRAP \
           -DUNICODE -D_UNICODE \
           -I$(PSAPI)\Pica_sp -I$(PSAPI)\Photoshop -I$(PSAPI)\General \
           -I..\telegraphics_common\adobeplugin -I..\telegraphics_common\tt -I..
# /O2 = Max optimization (prefer performance). /O2 implies /Ot
CFLAGS = -O2 $(CPPFLAGS)

# Note since VS8 it is necessary to link with multithreaded standard lib, using /MT
LDFLAGS = /LD /MT user32.lib gdi32.lib comdlg32.lib shell32.lib

# resource compiler flags
RFLAGS = -i$(PSAPI)\Photoshop -d_WIN32 -d_X86_ -dUNICODE -d_UNICODE

# Path to flex and bison (you can download them at https://sourceforge.net/projects/winflexbison/ )
FLEX = win_flex_bison\win_flex
BISON = win_flex_bison\win_bison

OBJ = ..\main.obj ..\funcs.obj ..\process.obj ..\node.obj ..\symtab.obj \
	..\ui.obj ..\ui_build.obj ..\preview.obj ..\read.obj ..\save.obj ..\make.obj ..\obfusc.obj ..\ff_misc.obj ..\language.obj ..\slider_win.obj \
	..\scripting.obj ..\lex.yy.obj ..\manifest.obj ..\ui_win.obj ..\make_win.obj ..\versioninfo_modify_win.obj ..\load_win.obj \
	..\telegraphics_common\tt\dbg_win.obj ..\telegraphics_common\tt\ui_compat_win.obj \
	..\telegraphics_common\tt\choosefile_win.obj ..\ui_build_win.obj \
	..\telegraphics_common\tt\compat_string.obj ..\telegraphics_common\tt\compat_win.obj ..\telegraphics_common\tt\compat_win_resource.obj \
	..\telegraphics_common\tt\file_compat_win.obj ..\telegraphics_common\tt\str.obj \
	..\telegraphics_common\adobeplugin\dllmain.obj

all : parser lexer $(EXEC).8bf

clean :
	-del *.obj *.asm *.cod win_res.res $(EXEC).8bf $(EXEC).exp $(EXEC).lib $(EXEC).map

win_res.res : ..\win_res.rc ..\Scripting.rc ..\PiPL.rc ..\PiPL_body.rc ..\manifest.rc ..\version_win.rc ..\ui_win.rc ..\caution.ico ..\ui.h ..\ui_win.h ..\version.h
	$(RC) $(RFLAGS) $(CPPFLAGS) -fowin_res.res ..\win_res.rc
	
parser : ..\parser.y
	rem $(BISON) ..\parser.y -d -y
	rem We need to switch the path, because otherwise y.tab.c will be created in visual_studio and not the source directory
	cd ..
	$(BISON) parser.y -d -y
	cd visual_studio
	
lexer : ..\lexer.l
	rem $(FLEX) --never-interactive ..\lexer.l y.tab.c
	rem We need to switch the path, because otherwise lex.yy.c will be created in visual_studio and not the source directory
	cd ..
	$(FLEX) --never-interactive lexer.l y.tab.c
	cd visual_studio

$(EXEC).8bf : $(OBJ) win_res.res
	$(CC) /Fe$@ $(**F) $(LDFLAGS)
