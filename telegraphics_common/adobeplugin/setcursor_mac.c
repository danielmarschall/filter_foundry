#include "ASConfig.h"

#include <quickdraw.h>

#include "astypes.h"
#include "spplugs.h"

#include "common.h"

void SetPlatformCursor(SPPluginRef pluginRef, int nCursorID)
{
	ASErr e = kNoErr;
	CursHandle cursor; 

	if ( cursor = GetCursor( nCursorID ) )
		SetCursor( *cursor );
}
