#include "common.h"

#include <windows.h>

void SetPlatformCursor(SPPluginRef pluginRef, int nCursorID)
{
	ASErr e = kNoErr;
	HCURSOR cursor;
	SPAccessRef access;
	SPPlatformAccessInfo spAccessInfo;

	e = sSPAccess->GetPluginAccess(pluginRef, &access);
	if( kNoErr == e)
		e = sSPAccess->GetAccessInfo(access, &spAccessInfo);
	if( kNoErr == e)
	{
		cursor = LoadCursor((HINSTANCE)spAccessInfo.defaultAccess, MAKEINTRESOURCE(nCursorID));
		if ( cursor )
			SetCursor(cursor);
	}
}
