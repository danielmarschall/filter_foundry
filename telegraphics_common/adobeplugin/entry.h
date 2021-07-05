/*
	This file is part of a common library for Adobe(R) plugins
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License  
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// AI CS2 SDK must be compiled with C++
// but we still require C linkage on entry point.

#ifdef MAC_ENV
  #ifdef __cplusplus
    #define DLLEXPORT extern "C"
  #else
    #define DLLEXPORT
  #endif
  #define MACPASCAL pascal
  #define ENTRYPOINT main
#else
  #ifdef __cplusplus
    #define DLLEXPORT extern "C" __declspec(dllexport) 
  #else
    #define DLLEXPORT __declspec(dllexport)
  #endif
  #define MACPASCAL 
  #define ENTRYPOINT PluginMain
#endif

// C++ is strict about main() returning int
#ifdef __cplusplus
  //typedef int PluginReturn_t;
  #define PLUGINRETURN int
#else
  //typedef SPErr PluginReturn_t;
  #define PLUGINRETURN SPErr
#endif
