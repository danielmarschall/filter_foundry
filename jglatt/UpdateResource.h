#ifndef __UPDATERESOURCE_H__
#define __UPDATERESOURCE_H__

#include <windows.h>	//WORD, DWORD, HANDLE...

#define ERROR_CANT_MOVE_SECTION  (1 | (1 << 29))

#ifdef __cplusplus
extern "C" {
#endif

HANDLE WINAPI _BeginUpdateResourceA(LPCSTR, BOOL);
typedef HANDLE WINAPI BeginUpdateResourceAPtr(LPCSTR, BOOL);
HANDLE WINAPI _BeginUpdateResourceW(LPCWSTR, BOOL);
typedef HANDLE WINAPI BeginUpdateResourceWPtr(LPCWSTR, BOOL);
#define BeginUpdateResourceNameA "_BeginUpdateResourceA"
#define BeginUpdateResourceNameW "_BeginUpdateResourceW"

BOOL WINAPI _UpdateResourceA(HANDLE, LPCSTR, LPCSTR, WORD, LPVOID, DWORD);
typedef BOOL WINAPI UpdateResourceAPtr(HANDLE, LPCSTR, LPCSTR, WORD, LPVOID, DWORD);
BOOL WINAPI _UpdateResourceW(HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD);
typedef BOOL WINAPI UpdateResourceWPtr(HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD);
#define UpdateResourceNameA "_UpdateResourceA"
#define UpdateResourceNameW "_UpdateResourceW"

LPBYTE WINAPI _GetRsrcDataA(HANDLE, LPCSTR, LPCSTR, WORD, DWORD *);
typedef LPBYTE WINAPI GetRsrcDataAPtr(HANDLE, LPCSTR, LPCSTR, WORD, DWORD *);
LPBYTE WINAPI _GetRsrcDataW(HANDLE, LPCWSTR, LPCWSTR, WORD, DWORD *);
typedef LPBYTE WINAPI GetRsrcDataWPtr(HANDLE, LPCWSTR, LPCWSTR, WORD, DWORD *);
#define GetRsrcDataNameA "_GetRsrcDataA"
#define GetRsrcDataNameW "_GetRsrcDataW"

BOOL WINAPI _EndUpdateResource(HANDLE, BOOL);
typedef BOOL WINAPI EndUpdateResourcePtr(HANDLE, BOOL);
#define EndUpdateResourceName "_EndUpdateResource"

#ifdef _UNICODE
# define _BeginUpdateResource		_BeginUpdateResourceW
# define _UpdateResource			_UpdateResourceW
# define _GetRsrcData				_GetRsrcDataW
# define BeginUpdateResourcePtr		BeginUpdateResourceWPtr
# define UpdateResourcePtr			UpdateResourceWPtr
# define GetRsrcDataPtr				GetRsrcDataWPtr
# define BeginUpdateResourceName	BeginUpdateResourceNameW
# define UpdateResourceName			UpdateResourceNameW
# define GetRsrcDataName			GetRsrcDataNameW
#else
# define _BeginUpdateResource		_BeginUpdateResourceA
# define _UpdateResource			_UpdateResourceA
# define _GetRsrcData				_GetRsrcDataA
# define BeginUpdateResourcePtr		BeginUpdateResourceAPtr
# define UpdateResourcePtr			UpdateResourceAPtr
# define GetRsrcDataPtr				GetRsrcDataAPtr
# define BeginUpdateResourceName	BeginUpdateResourceNameA
# define UpdateResourceName			UpdateResourceNameA
# define GetRsrcDataName			GetRsrcDataNameA
#endif

#ifdef __cplusplus
}
#endif

#endif //__UPDATERESOURCE_H__
