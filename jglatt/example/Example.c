/* Tests the resource.dll Dynamic Link Library, containing the functions
 * _BeginUpdateResource(), _UpdateResource(), _EndUpdateResource(), and
 * _GetRsrcData().
 */

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

/* Must include this special .h for our resource.dll */
#include "..\UpdateResource.h"

int main(int argc, char *argv[], char *envp[])
{
	HINSTANCE				ResourceDll;
	BeginUpdateResourcePtr	*BeginUpdateResource;
	EndUpdateResourcePtr	*EndUpdateResource;
	UpdateResourcePtr		*UpdateResource;
	GetRsrcDataPtr			*GetRsrcData;

	HANDLE					rsc;

	/* Check if any parameters were given, if not, display some help info */
	if (argc < 2)
	{
		_cputs("Tests resource.dll.\r\n\nSyntax is '");
		_cputs(argv[0]);
		_cputs( " name'.\r\n    where name is the name of some exe/dll whose resources are to be listed.\r\n");
		return(-1);
	}

	/* Open resource.dll */
	if (!(ResourceDll = LoadLibrary("..\\..\\debug\\resource.dll")))
	{
		_cputs("Can't find resource.dll!\r\n");
		return(-2);
	}

	/* Get the address of the _BeginUpdateResource() function */
	if (!(BeginUpdateResource = (BeginUpdateResourcePtr *)GetProcAddress(ResourceDll, BeginUpdateResourceName)) ||

		/* Get the address of the _EndUpdateResource() function */
		!(EndUpdateResource = (EndUpdateResourcePtr *)GetProcAddress(ResourceDll, EndUpdateResourceName)) ||

		/* Get the address of the _UpdateResource() function */
		!(UpdateResource = (UpdateResourcePtr *)GetProcAddress(ResourceDll, UpdateResourceName)) ||

		/* Get the address of the _GetRsrcData() function */
		!(GetRsrcData = (GetRsrcDataPtr *)GetProcAddress(ResourceDll, GetRsrcDataName)))
	{
		FreeLibrary((HMODULE)ResourceDll);
		_cputs("Not the resource.dll version we need!\r\n");
		return(-3);
	}

	/* Load the resources for the specified exe/dll */
	if (!(rsc = BeginUpdateResource(argv[1], FALSE)))
	{
		FreeLibrary((HMODULE)ResourceDll);
		_cputs("Can't load the resources for \"");
		_cputs(argv[1]);
		_cputs("\"!\r\n");
		return(-4);
	}

	/* End the resource.dll usage */
	if (!EndUpdateResource(rsc, FALSE))
	{
		UCHAR	strError[256];
		DWORD	errornum;

		errornum = GetLastError();

		// Get the error message associated with this error # from Windows
		// except for ERROR_CANT_MOVE_SECTION
		if (errornum != ERROR_CANT_MOVE_SECTION)
		{
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errornum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &strError[0], 256, 0);
		}
		else
			strcpy(&strError[0], "This exe/dll is in a format that can't be processed by resource.dll!");

		MessageBox(0, &strError[0], "ERROR", MB_OK);
	}

	/* Free the library */
	FreeLibrary((HMODULE)ResourceDll);

	return(0);
}