/* this code by Jeff Glatt, http://www.borg.com/~jglatt/ */

/* Based on work by Erik Kallen. His original sources were in C++.
 * These have been converted and rewritten in plain C, and turned into
 * a Dynamic Link Library.
 *
 * There are 2 important limitations to this code:
 *
 * 1) It doesn't work on an EXE/DLL that doesn't already have an ".rsrc"
 * section. In order to do otherwise, we would need to add features to
 * rewrite the PE header and perhaps fix up relocations in code sections.
 *
 * 2) It can't modify the resources of an EXE/DLL whose ".rsrc" section
 * comes before relocatable code. In order to do otherwise, we would
 * need to add features to fix-up the code sections of the EXE/DLL. (If
 * someone wants to add them, go right ahead).
 *
 * The benefits of plain C code:
 *
 * 1) No hassle with exceptions, especially raised by memory allocations
 *    such as C++'s new, nor no need for C++ language extensions nor
 *    C++ compiler library concerns.
 * 2) Because of 1, it was very easy to convert this to a DLL for use in
 *    a program written in any language.
 * 3) It can be compiled with any C compiler, including freeware offerings.
 *
 * Format of a .rsrc section of a PE file:
 * NOTE: A bunch of RSRCDIRENTRY always follow a RSRCDIRTABLEHEAD. The
 * number of RSRCDIRENTRY is RSRCDIRTABLEHEAD.NumNameEntries +
 * RSRCDIRTABLEHEAD.NumIDEntries, and the RSRCDIRENTRY's for the items
 * with name-strings come first (before the items with ID-numbers).
 *
 * master RSRCDIRTABLEHEAD
 *   Named Type 1 (RSRCDIRENTRY)
 *   Named Type 2
 *   ...
 *   Named Type XXX
 *   ID Type 1 (RSRCDIRENTRY)
 *   ID Type 2
 *   ...
 *   ID Type XXX

 * RSRCDIRTABLEHEAD for Named Type 1
 *   Named Item 1 for Named Type 1 (RSRCDIRENTRY)
 *   Named Item 2 for Named Type 1
 *   ...
 *   Named Item XX for Named Type 1
 *   ID'ed Item 1 for Named Type 1 (RSRCDIRENTRY)
 *   ID'ed Item 2 for Named Type 1
 *   ...
 *   ID'ed Item XX for Type 1

 * RSRCDIRTABLEHEAD for Named Item 1 for Named Type 1
 *   Language 1 for Named Item 1 for Named Type 1 (RSRCDIRENTRY)
 *   Language 2 for Named Item 1 for Named Type 1
 *   ...
 *   Language XXX for Named Item 1 for Named Type 1

 *   RSRCDIRTABLEHEAD for ID'ed Item 1 for Named Type 1
 *		Language 1 for ID'ed Item 1 for Named Type 1 (RSRCDIRENTRY)
 *		Language 2 for ID'ed Item 1 for Named Type 1
 *		...
 *		Language XXX for ID'ed Item 1 for Named Type 1

 * RSRCDATAENTRY for Language 1 for Named Item 1 for Named Type 1
 * RSRCDATAENTRY for Language 2 for Named Item 1 for Named Type 1
 * ...
 * RSRCDATAENTRY for Language 1 for ID'ed Item 1 for Named Type 1
 * RSRCDATAENTRY for Language 2 for ID'ed Item 1 for Named Type 1
 * ...
 */

#include <windows.h>
#include "UpdateResource.h"
#include "RsrcStructs.h"




////////////////////////// Global data /////////////////////////

#if defined(_MSC_VER)
#pragma data_seg("Shared")
#endif

const unsigned char RsrcSectionName[] = ".rsrc";
const long DaysInLeapYear[] = {30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
const long DaysInYear[] = {30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

#if defined(_MSC_VER)
#pragma data_seg()
#endif

unsigned long TimezoneOffset = 0;
unsigned char DaylightFlag = 1;





////////////////////////////////////////////////////////////////////////
// Helper functions called only internally, not by some application.

#define _BASE_DOW			4
#define _LEAP_YEAR_ADJUST	17L
#define _BASE_YEAR			70L

/* ********************** set_time_zone() ********************
 * Fetches the values for 'TimezoneOffset' (ie, # of seconds
 * difference between the system's current time zone and GMT)
 * and DaylightFlag (ie, 1 if the system is set to observe
 * USA Daylight Savings Time).
 *
 * NOTES: This must be called once before any of the other
 * time functions are called, such as FTIME, GMTIME, LOCALTIME,
 * etc.
 */ 

static void set_time_zone(void)
{
	TIME_ZONE_INFORMATION tzinfo;

	// Get timezone information
	if (GetTimeZoneInformation(&tzinfo) != 0xffffffff)
	{
		// Calculate a time offset (in seconds) from GMT
		TimezoneOffset = tzinfo.Bias * 60L;

		if (tzinfo.StandardDate.wMonth)
			TimezoneOffset += (tzinfo.StandardBias * 60L);

		// Check to see if daylight savings is not applicable
		if (!tzinfo.DaylightDate.wMonth && !tzinfo.DaylightBias)
			DaylightFlag = 0;
	}
}

/* ************************* is_savings() ***********************
 * Checks if a Date/Time falls within USA Daylight Savings Time.
 *
 * This is the rule for years before 1987:
 * A time is in DST if it is on or after 02:00:00 on the last Sunday
 * in April and before 01:00:00 on the last Sunday in October.
 * This is the rule for years starting with 1987:
 * A time is in DST if it is on or after 02:00:00 on the first Sunday
 * in April and before 01:00:00 on the last Sunday in October.
 *
 * tb =	Pointer to TM struct holding the Date/Time.
 *
 * RETURNS: 1 if in DST, 0 otherwise.
 */

static BOOL is_savings(TM *tb)
{
	int	yr;
	int	mdays;
	int	critsun;

	// If Year < 1967, then it's before DST was instituted. Also, if the
	// month is before April or after October, it cannot be DST.
	if (tb->tm_year < 67 || tb->tm_mon < 3 || tb->tm_mon > 9)
		return(0);

	// If the month is after April and before October, it must be DST.
	if (tb->tm_mon > 3 && tb->tm_mon < 9)
		return(1);

	// Month is April or October, so we need to see if it falls between
	// appropriate Sundays.

	// For years before 1987 (or after 1986), we check if the day is on or
	// after 2:00 am on the last (or first) Sunday in April, or before 1:00
	// am on the last Sunday in October.

	// We can get the Day of Year (0..365) from the tm struct. We need to
	// calculate the Day of Year for the last (or first) Sunday in this month,
	// April or October, and then do the comparison.

	// To calculate the Day of Year of the last Sunday:
	// 1. Get the Day Of Year of the last day of the current month (Apr or Oct)
	// 2. Determine the Day of Week of that day
	// 3. Subtract Day of Week from Day of Year

	// To calculate the Day of Year of the first Sunday:
	// 1. Get the Day of Year of the 7th day of the current month (April)
	// 2. Determine the Day of Week of that day
	// 3. Subtract Day of Week from Day of Year

	// First we get #1. The Day of Year for the first day of each month is
	// stored in _DaysInYear[]. They're all off by -1
	if (((yr = tb->tm_year) > 86) && (tb->tm_mon == 3))
		mdays = 7 + DaysInYear[tb->tm_mon - 1];
	else
		mdays = DaysInYear[tb->tm_mon];

	// If this is a leap-year, add an extra day
	if (!(yr & 3))
		mdays++;

	// Now get #2. We know the Day of Week of January 1, 1970, which is
	// defined as the constant _BASE_DOW. Add to this the number of elapsed
	// days since then, take the result mod 7, and we have our day number.

	// To obtain #3, we just subtract this from mdays.
	critsun = mdays - ((mdays + 365 * (yr - 70) + ((yr - 1) >> 2) - _LEAP_YEAR_ADJUST + _BASE_DOW) % 7);

	return ( (tb->tm_mon == 3) ?
		((tb->tm_yday > critsun) || ((tb->tm_yday == critsun) && (tb->tm_hour >= 2))) :
		((tb->tm_yday < critsun) || ((tb->tm_yday == critsun) && (tb->tm_hour < 1))) );
}

static DWORD get_time(void)
{
	SYSTEMTIME	dt;
	DWORD		totaldays;
	DWORD		totaltime;
	TM			tb;

	GetLocalTime(&dt);

	dt.wYear -= 1900;
	totaldays = dt.wDay + DaysInYear[dt.wMonth - 1];
	if (!(dt.wYear & 3) && (dt.wMonth > 2)) totaldays++;
	totaltime = ((((DWORD)(dt.wYear - _BASE_YEAR) * 365L + ((dt.wYear - 1) >> 2) - _LEAP_YEAR_ADJUST + totaldays) * 24L + dt.wHour) * 60L + dt.wMinute) * 60L + dt.wSecond + TimezoneOffset;

	if (DaylightFlag)
	{
		tb.tm_yday = totaldays;
		tb.tm_year = dt.wYear;
		tb.tm_mon  = dt.wMonth - 1;
		tb.tm_hour = dt.wHour;
		if (is_savings(&tb)) totaltime -= 3600L;
	}

	return(totaltime);
}

/************************ wstrnicmp_a() ************************
 * Compares 2 UNICODE strings, case-insensitive. The second
 * string's nul-termination determines when to stop the
 * compare, and the second string must be shorter or same size
 * as the first.
 *
 * str1 =	Pointer to first string.
 * str2 =	Pointer to the second string.
 *
 * RETURNS: 0 if equal or non-zero if not equal.
 */

static DWORD wstrnicmp_a(const unsigned short *str1, const unsigned short *str2)
{
	unsigned short	chr1, chr2;

	do
	{
		if (!(chr2 = (unsigned short)CharUpperA((LPTSTR)*(str2)++))) return(0);
		chr1 = (unsigned short)CharUpperA((LPTSTR)*(str1)++);
	} while (chr2 == chr1);

	return(chr2 - chr1);
}

/************************ strlen_w() ************************
 * Counts the length of a nul-terminated UNICODE string.
 *
 * RETURNS: Length in WCHARs (ie, shorts).
 */

static DWORD strlen_w(LPCWSTR str)
{
	LPCWSTR strstart;

	strstart = str;
	while (*(str)++);

	return((str - strstart - 1) >> 1);
}

////////////////////////////////////////////////////////////////////////
// This is the BeginUpdateResource() API, and any helper functions.
// (Helper functions are called only by BeginUpdateResource(), not by
// some application).

/************************** getRsrcString() ***************************
 * Puts a nul-terminated copy of the passed WCHAR[] string into our list
 * of strings, and returns a pointer to that copy.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 * pRsrc =		Pointer to the string resource.
 * offset =		Byte offset (from pRsrc) to where the length of
 *				string is stored as a WORD.
 *
 * RETURN: Pointer to nul-terminated WCHAR[] copy of pRsrc's string, or
 * 0 if a memory allocation error.
 */

static LPWSTR getRsrcString(LPRSRCINFO pInfo, char *pRsrc, DWORD offset)
{
	LPSTRINGHEAD	lpStr, lpPrev;
	LPWSTR			orig;
	WORD			size;

	// Get the length of the string
	size = *(WORD *)(pRsrc + offset);

	// Get the start of the WCHAR[] string
	orig = (LPWSTR)(pRsrc + offset + 2);

	// See if we already have a string resource matching this string. If so, return that string
	lpPrev = (LPSTRINGHEAD)&pInfo->vtrStrings;
	if ((lpStr = pInfo->vtrStrings))
	{
		do
		{
			if (lpStr->Size == size && !wstrnicmp_a(orig, &lpStr->String[0])) goto gotit;
			lpPrev = lpStr;
		} while ((lpStr = lpStr->Next));
	}

	// No, we don't already have this string. Create a nul-terminated copy and put it into our list of resources.
	// NOTE: We are copying a WSTR[] so size is 2 bytes per char (plus 2 bytes of nul)
	if ((lpStr = (LPSTRINGHEAD)GlobalAlloc(GMEM_FIXED, (size << 1) + sizeof(STRINGHEAD))))
	{
		lpStr->Size = size;
//		lpStr->Next = pInfo->vtrStrings;
//		pInfo->vtrStrings = lpStr;

		lpPrev->Next = lpStr;
		lpStr->Next = 0;

		memcpy(&lpStr->String[0], orig, size << 1);
		lpStr->String[size] = 0;
gotit:	return(&lpStr->String[0]);
	}

	return(0);
}

/************************** linkIntoNameList() *************************
 * Creates a RESOURCENAME struct for the specified Type and links it
 * into that Type's list of RESOURCENAMEs, and returns a pointer to that
 * RESOURCENAME.
 *
 * pInfo =		Pointer to RESOURCETYPE.
 * dwName =		Name of resource. Could be a DWORD number or a
 *				pointer to a string.
 * info =		Pointer to RESOURCEINFO, or 0 if none.
 *
 * RETURN: Pointer to RESOURCENAME for this Type, or 0 if a memory
 * allocation error.
 */

static LPRESOURCENAME linkIntoNameList(LPRESOURCETYPE type, LPWSTR dwName, LPRESOURCEINFO info)
{
	LPRESOURCENAME	lpRes, lpPrev;

	// See if we already have a RESOURCENAME matching this name. If so, return that RESOURCENAME
	lpPrev = (LPRESOURCENAME)&type->Names;
	if ((lpRes = type->Names))
	{
		do
		{
			if (lpRes->NameString == dwName) goto gotit;
			lpPrev = lpRes;
		} while ((lpRes = lpRes->Next));
	}

	// No, we don't already have this RESOURCENAME. Create a new RESOURCENAME and link it at the tail
	// of our list of named resources for this Type
	if ((lpRes = (LPRESOURCENAME)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCENAME))))
	{
		lpPrev->Next = lpRes;
		lpRes->Next = 0;
		lpRes->Languages = 0;
		lpRes->NameString = dwName;	// Store the name

		memcpy(&lpRes->Characteristics, info, sizeof(RESOURCEINFO));
	}

gotit:
	return(lpRes);
}

/*************************** add_resource() *****************************
 * Called by fillRsrcEntries() to parse a resource (ie, load/store info
 * about each language-specific instance of this Type/Name of resource).
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 *
 * RETURNS: TRUE if successful, or FALSE if an error.
 */

static BOOL add_resource(LPRSRCINFO pInfo, char *pRsrc, RSRCDIRENTRY *pNameEntry, LPWSTR dwName, LPRESOURCETYPE type)
{
	LPRESOURCENAME		name;
	RSRCDIRTABLEHEAD	*pLangHead;
	RSRCDIRENTRY		*pLangEntry;
	DWORD				langIndex;

	// Locate the RSRCDIRTABLEHEAD for this named resource's languages
	pLangHead = (RSRCDIRTABLEHEAD *)(pRsrc + (pNameEntry->SubdirectoryRVA & ~0x80000000));

	// Allocate a RESOURCENAME to store info about this RSRCDIRTABLEHEAD, and link into list
	if (!(name = (LPRESOURCENAME)linkIntoNameList(type, dwName, (LPRESOURCEINFO)pLangHead))) goto bad;

	// Locate the RSRCDIRENTRY for the first language (immediately follows the RSRCDIRTABLEHEAD)
	pLangEntry = (RSRCDIRENTRY *)((char *)pLangHead + sizeof(RSRCDIRTABLEHEAD));

	// Do all of the RSRCDIRENTRY's (ie, language versions of this resource)
	if ((langIndex = pLangHead->NumIDEntries))
	{
		do
		{
			RSRCDATAENTRY	*pData;
			LPRESOURCE		rsc;

			// Can't have a subdir here, so we don't find any RSRCDIRTABLEHEAD struct

			// Allocate a RESOURCE struct to store info
			if (!(rsc = (LPRESOURCE)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCE)))) goto bad;

			// Locate the actual resource data
			pData = (RSRCDATAENTRY *)(pRsrc + (pLangEntry->DataEntryRVA & ~0x80000000));

			// Store info
			rsc->Codepage = pData->Codepage;
			rsc->DataSize = pData->Size;
			rsc->Reserved = pData->Reserved;
			rsc->LanguageID = (WORD)pLangEntry->IntegerID;

			// Allocate a buffer and copy the resource data
			if (!(rsc->Data = (char *)GlobalAlloc(GMEM_FIXED, pData->Size)))
			{
				GlobalFree(rsc);
bad:			return(FALSE);
			}
			memcpy(rsc->Data, pRsrc + pData->DataRVA - pInfo->SectionHeader.VirtualAddress, pData->Size);

			// Link RESOURCE struct into this RESOURCENAME's list of languages
			rsc->Next = name->Languages;
			name->Languages = rsc;

			// Next language
			++pLangEntry;

		} while (--langIndex);
	}

	return(TRUE);
}

/************************** linkIntoTypeList() *************************
 * Creates a RESOURCETYPE struct for the specified type and links it
 * into our list of RESOURCETYPEs, and returns a pointer to that
 * RESOURCETYPE.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 * dwType =		Type of resource. Could be a DWORD Type number or a
 *				pointer to a Type string.
 * info =		Pointer to RESOURCEINFO, or 0 if none.
 *
 * RETURN: Pointer to RESOURCETYPE for this Type, or 0 if a memory
 * allocation error.
 */

static LPRESOURCETYPE linkIntoTypeList(LPRSRCINFO pInfo, LPWSTR dwType, LPRESOURCEINFO info)
{
	LPRESOURCETYPE	lpType, lpPrev;

	// See if we already have a RESOURCETYPE matching this type. If so, return that RESOURCETYPE
	lpPrev = (LPRESOURCETYPE)&pInfo->Types;
	if ((lpType = pInfo->Types))
	{
		do
		{
			if (lpType->TypeString == dwType) goto gotit;
			lpPrev = lpType;
		} while ((lpType = lpType->Next));
	}

	// No, we don't already have this RESOURCETYPE. Create a new RESOURCETYPE and link it at the tail
	// of our list of resources
	if ((lpType = (LPRESOURCETYPE)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCETYPE))))
	{
		lpPrev->Next = lpType;
		lpType->Next = 0;
		lpType->Names = 0;
		lpType->TypeString = dwType;	// Store the type

		memcpy(&lpType->Characteristics, info, sizeof(RESOURCEINFO));
	}

gotit:
	return(lpType);
}

/************************* add_resource_type() **************************
 * Called by fillRsrcEntries() to parse a Type of resource (ie, load/store
 * info about each Named/ID'ed instance of this Type of resource).
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 *
 * RETURNS: TRUE if successful, or FALSE if an error.
 */

static BOOL add_resource_type(LPRSRCINFO pInfo, char *pRsrc, RSRCDIRENTRY *pTypeEntry, LPWSTR dwType)
{
	LPRESOURCETYPE		type;
	RSRCDIRTABLEHEAD	*pNameHead;
	DWORD				nameIndex;
	RSRCDIRENTRY		*pNameEntry;

	// Locate the RSRCDIRTABLEHEAD struct for this resource Type
	pNameHead = (RSRCDIRTABLEHEAD *)(pRsrc + (pTypeEntry->SubdirectoryRVA & ~0x80000000));

	// Allocate a RESOURCETYPE to store info about this RSRCDIRTABLEHEAD, and link into list
	if (!(type = (LPRESOURCETYPE)linkIntoTypeList(pInfo, dwType, (LPRESOURCEINFO)pNameHead))) goto bad;

	// Start with the first RSRCDIRENTRY (immediately after the RSRCDIRTABLEHEAD)
	pNameEntry = (RSRCDIRENTRY *)((char *)pNameHead + sizeof(RSRCDIRTABLEHEAD));

	// Do the items that have a Name string (instead of ID number)
	if ((nameIndex = pNameHead->NumNameEntries))
	{
		do
		{
			// Make sure that there is no Name number. There shouldn't be both a string *and* a number
			if (pNameEntry->DataEntryRVA >> 31)
			{
				LPWSTR		dwName;

				// Put a copy of the name in our list of strings and use that copy
				if (!(dwName = getRsrcString(pInfo, pRsrc, pNameEntry->NameRVA & ~(1<<31))) ||

					// Load/store info about each language-specific instance of this Named item
					!add_resource(pInfo, pRsrc, pNameEntry, dwName, type))
				{
bad:				return(FALSE);
				}
			}

			// Next RSRCDIRENTRY
			++pNameEntry;

		} while (--nameIndex);
	}

	// Do the items that have an ID number
	if ((nameIndex = pNameHead->NumIDEntries))
	{
		do
		{
			// Load/store info about each language-specific instance of this ID'ed item
			if ((pNameEntry->DataEntryRVA >> 31) && !add_resource(pInfo, pRsrc, pNameEntry, (LPWSTR)pNameEntry->IntegerID, type)) goto bad;
			++pNameEntry;
		} while (--nameIndex);
	}

	return(TRUE);
}

/************************** fillRsrcEntries() ***************************
 * Called by BeginUpdateResource() to parse the resources (ie, '.rsrc'
 * section) of a PE file (which we have already loaded into one block of
 * memory) and add those resources to our internal table in RAM. In other
 * words, this implements loading the original, existing resources within
 * the file.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 * pRsrc =		Pointer to memory buffer containing the exe/dll's '.rsrc'
 *				section.
 *
 * RETURNS: 0 if successful, or an appropriate error number as returned by
 * GetLastError().
 */

static DWORD fillRsrcEntries(LPRSRCINFO pInfo, char *pRsrc)
{
	DWORD				numIndex;
	RSRCDIRENTRY		*pTypeEntry;
	LPWSTR				dwType;

	// The master RSRCDIRTABLEHEAD struct starts right at the beginning of the loaded '.rsrc' section.
	// Store it in our RSRCINFO
	pInfo->Characteristics = ((RSRCDIRTABLEHEAD *)pRsrc)->Characteristics;
	pInfo->TimeDateStamp = ((RSRCDIRTABLEHEAD *)pRsrc)->TimeDateStamp;
	pInfo->VersionMajor = ((RSRCDIRTABLEHEAD *)pRsrc)->VersionMajor;
	pInfo->VersionMinor = ((RSRCDIRTABLEHEAD *)pRsrc)->VersionMinor;

	// Start with the first RSRCDIRENTRY (immediately after the master RSRCDIRTABLEHEAD)
	pTypeEntry = (RSRCDIRENTRY *)(pRsrc + sizeof(RSRCDIRTABLEHEAD));

	// Enumerate resources that have a Type string (instead of Type number)
	if ((numIndex = ((RSRCDIRTABLEHEAD *)pRsrc)->NumNameEntries))
	{
		do
		{
			// Make sure that this resource doesn't also have a Type number. That is not legal. (If illegal, just
			// skip it)
			if (pTypeEntry->DataEntryRVA >> 31)	//pTypeEntry->DataEntryRVA & 0x80000000
			{
				// Put a copy of the Type string in our list of strings and use that copy
				if (!(dwType = getRsrcString(pInfo, pRsrc, pTypeEntry->NameRVA & ~(1 << 31))) ||

					// Load/store info about each instance of this Type
					!add_resource_type(pInfo, pRsrc, pTypeEntry, dwType))
				{
bad:				return(ERROR_NOT_ENOUGH_MEMORY);
				}
			}
	
			// Next RSRCDIRENTRY
			++pTypeEntry;

		} while (--numIndex);
	}

	// Enumerate resources that have a Type number
	if ((numIndex = ((RSRCDIRTABLEHEAD *)pRsrc)->NumIDEntries))
	{
		do
		{
			if ((pTypeEntry->DataEntryRVA >> 31) && !add_resource_type(pInfo, pRsrc, pTypeEntry, (LPWSTR)pTypeEntry->IntegerID)) goto bad;

			// Next RSRCDIRENTRY
			++pTypeEntry;

		} while (--numIndex);
	}

	// Success
	return(0);
}

/*********************** _BeginUpdateResourceA() ************************
 * An implementation of the Win32 API BeginUpdateResource(). This is the
 * ansi version.
 *
 * This must be called once before UpdateResource() can be used to change
 * the resources in an EXE/DLL, or before GetRsrcData() can be used to
 * fetch some resource.
 *
 * lpFileName =		Pointer to nul-terminated filename of DLL/EXE.
 * bDelete =		TRUE if deleting the existing resources, or FALSE if
 *					retaining them.
 *
 * RETURNS: Handle to exe/dll resources, or 0 if an error. If an error,
 * then no cleanup need be done by the application, and GetLastError()
 * will return an appropriate error number.
 */

HANDLE WINAPI _BeginUpdateResourceA(LPCSTR lpFileName, BOOL bDelete)
{
	LPRSRCINFO	pInfo;
	DWORD		dw, result, id;

	// Allocate a RSRCINFO struct
	if ((pInfo = (LPRSRCINFO)GlobalAlloc(GMEM_FIXED, sizeof(RSRCINFO))))
	{
		// Clear the lists
		pInfo->Types = 0;
		pInfo->vtrStrings = 0;

		// Open the file for reading
		if ((pInfo->File = CreateFile(lpFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			result = GetLastError();	// Preserve error across call to GlobalFree()
			GlobalFree(pInfo);
			SetLastError(result);
//			SetLastError(ERROR_OPEN_FAILED);
			return(NULL);
		}

		// Seek to the beginning of the PE header
		SetFilePointer(pInfo->File, 0x3c, 0, FILE_BEGIN);
		if (GetLastError() != NO_ERROR)
		{
bad:		result = ERROR_RESOURCE_DATA_NOT_FOUND;
bad2:		_EndUpdateResource(pInfo, 1);
			SetLastError(result);
			return(NULL);
		}

		// Read the long offset to file ID
		if (!ReadFile(pInfo->File, &dw, 4, &result, NULL) || result != 4) goto bad;

		// Seek to that offet
		SetFilePointer(pInfo->File, dw, 0, FILE_BEGIN);
		if (GetLastError() != NO_ERROR ||

			// Read a long ID
			!ReadFile(pInfo->File, &id, 4, &result, NULL) || result != 4 ||

			// Check the ID
			id != IMAGE_NT_SIGNATURE)
		{
			// A non-PE file does not contain resources
			goto bad;
		}

		pInfo->FilePtrToPEHdr = dw + 4;

		// Read in the file header
		if (!ReadFile(pInfo->File, &pInfo->PEHeader, sizeof(IMAGE_FILE_HEADER), &result, NULL) || result != sizeof(IMAGE_FILE_HEADER))
			goto bad;

		// Skip the optional header
		SetFilePointer(pInfo->File, pInfo->PEHeader.SizeOfOptionalHeader, 0, FILE_CURRENT);
		if (GetLastError() != NO_ERROR) goto bad;

		// Locate the '.rsrc' section
		dw = pInfo->PEHeader.NumberOfSections;		
		while (dw--)
		{
			// Read in the next section's header
			if (!ReadFile(pInfo->File, &pInfo->SectionHeader, sizeof(IMAGE_SECTION_HEADER), &result, NULL) || result != sizeof(IMAGE_SECTION_HEADER))
				goto bad;

			// Is this the '.rsrc' section?
			if (!strcasecmp((const unsigned char *)pInfo->SectionHeader.Name, &RsrcSectionName[0])) // IMAGE_SIZEOF_SHORT_NAME?
			{
				// Go to the start of that section's data within the file
				SetFilePointer(pInfo->File, pInfo->SectionHeader.PointerToRawData, 0, FILE_BEGIN);
				if (GetLastError() != NO_ERROR) goto bad;

				// We got the '.rsrc', so we can continue processing it
				break;
			}
		}

		// Did we find a ".rsrc" section?
		if (dw == (DWORD)-1) goto bad;

		// Delete the existing resources? NOTE: Deleting existing resources is implemented by not
		// reading them in right now and adding them to the resource table in memory. So later,
		// when we write out the final resources, the original ones will have been "deleted"
		if (!bDelete)
		{
			char	*pRsrc;

			// Allocate mem to read in the '.rsrc' section
			if (!(pRsrc = GlobalAlloc(GMEM_FIXED, pInfo->SectionHeader.SizeOfRawData)))
			{
				result = ERROR_NOT_ENOUGH_MEMORY;
				goto bad2;
			}

			// Read in the section
			if (!ReadFile(pInfo->File, pRsrc, pInfo->SectionHeader.SizeOfRawData, &result, NULL) || result != pInfo->SectionHeader.SizeOfRawData)
			{
				GlobalFree(pRsrc);
				goto bad;
			}

			// Parse '.rsrc' section into internal structs and link into our RSRCINFO lists
			result = fillRsrcEntries(pInfo, pRsrc);

			// Free the '.rsrc'
			GlobalFree(pRsrc);

			// If an error, delete any allocated resources
			if (result) goto bad2;
		}

		// Get timezone settings for our timestamp
		set_time_zone();

		// Success
		SetLastError(0);
	}

	// Return the pointer to our RSRCINFO struct for this EXE/DLL. The app will pass it
	// to our other functions such as UpdateResource()
	return((HANDLE)pInfo);
}

/*********************** _BeginUpdateResourceW() ************************
 * An implementation of the Win32 API BeginUpdateResource(). This is the
 * unicode version. It merely calls the ansi version after converting
 * filename from unicode to ansi.
 *
 * This must be called once before UpdateResource() can be used to change
 * the resources in an EXE/DLL, or before GetRsrcData() can be used to
 * fetch some resource.
 */

HANDLE WINAPI _BeginUpdateResourceW(LPCWSTR lpFileName, BOOL bDelete)
{
	LPSTR	lpsFileName;
	HANDLE	result;
	DWORD	nLen;

	nLen = WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, NULL, 0, NULL, 0);
	if ((lpsFileName = (LPSTR)GlobalAlloc(GMEM_FIXED, nLen)))
	{
		WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, lpsFileName, nLen, NULL, 0);

		result = _BeginUpdateResourceA(lpsFileName, bDelete);

		nLen = GetLastError();
		GlobalFree(lpsFileName);
		if (nLen) SetLastError(nLen);
		return(result);
	}

	return(0);
}

////////////////////////////////////////////////////////////////////////
// This is the UpdateResource() API, and any helper functions.
// (Helper functions are called only by UpdateResource(), not by
// some application).

/************************** addRsrcString() ***************************
 * Puts a nul-terminated copy of the passed WCHAR[] string into our list
 * of strings, and returns a pointer to that copy.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 * pRsrc =		Pointer to the string resource.
 * offset =		Byte offset (from pRsrc) to where the length of
 *				string is stored as a WORD.
 *
 * RETURN: Pointer to nul-terminated WCHAR[] copy of pRsrc's string, or
 * 0 if a memory allocation error.
 */

static LPCWSTR addRsrcString(LPRSRCINFO pInfo, LPCWSTR orig, LPVOID lpData)
{
	LPSTRINGHEAD	lpStr;
	DWORD			size;

	size = strlen_w(orig);

	// See if we already have a string resource matching this string. If so, use that string
	if ((lpStr = pInfo->vtrStrings))
	{
		do
		{
			if (lpStr->Size == size && !wstrnicmp_a(orig, &lpStr->String[0]))
			{
				// Use the string pointer in our list
				return(&lpStr->String[0]);
			}
		} while ((lpStr = lpStr->Next));
	}

	// No, we don't already have this string. Create a nul-terminated copy and put it into our list of resources.
	// NOTE: We are copying a WSTR[] so size is 2 bytes per char (plus 2 bytes of nul)
	if (lpData && (lpStr = (LPSTRINGHEAD)GlobalAlloc(GMEM_FIXED, (size << 1) + sizeof(STRINGHEAD))))
	{
		lpStr->Size = size;
		lpStr->Next = pInfo->vtrStrings;
		pInfo->vtrStrings = lpStr;
		memcpy(&lpStr->String[0], (PVOID)orig, size << 1);
		lpStr->String[size] = 0;
		return((LPCWSTR)&lpStr->String[0]);
	}

	return(0);
}

/************************** _UpdateResourceA() **************************
 * An implementation of the Win32 API UpdateResource(). This is the ansi
 * version. It merely calls the unicode version after converting any Type
 * and Name strings from ANSI to UNICODE.
 */

BOOL WINAPI _UpdateResourceA(HANDLE hUpdateResource, LPCSTR lpType, LPCSTR lpName, WORD wLanguage, LPVOID lpData, DWORD cbDataSize)
{
	LPCWSTR	lpwType, lpwName;
	BOOL	retCode;

	// Assume ID numbers (rather than string pointers)
	lpwType = (LPCWSTR)lpType;
	lpwName = (LPCWSTR)lpName;

	// Assume failure
	retCode = 0;

	// If Type is a string pointer (rather than an ID number), allocate and convert to Wide character string
	if (HIWORD(lpType))
	{
		DWORD		nLen;

		nLen = MultiByteToWideChar(CP_ACP, 0, lpType, -1, NULL, 0);
		if (!(lpwType = (LPCWSTR)GlobalAlloc(GMEM_FIXED, nLen * sizeof(WCHAR)))) goto bad;
		MultiByteToWideChar(CP_ACP, 0, lpType, -1, (LPWSTR)lpwType, nLen);
	}

	// If Name is a string pointer (rather than an ID number), allocate and convert to Wide character string
	if (HIWORD(lpName))
	{
		DWORD	nLen;

		nLen = MultiByteToWideChar(CP_ACP, 0, lpName, -1, NULL, 0);
		if (!(lpwName = (LPCWSTR)GlobalAlloc(GMEM_FIXED, nLen * sizeof(WCHAR))))
		{
			if (HIWORD(lpType)) GlobalFree((void *)lpwType);
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			goto bad;
		}
		MultiByteToWideChar(CP_ACP, 0, lpName, -1, (LPWSTR)lpwName, nLen);
	}

	retCode = _UpdateResourceW(hUpdateResource, lpwType, lpwName, wLanguage, lpData, cbDataSize);

	// If pointing to strings, free the Type and Name
	if (HIWORD(lpName)) GlobalFree((void *)lpwName);
	if (HIWORD(lpType)) GlobalFree((void *)lpwType);
bad:
	return(retCode);
}

/************************** _UpdateResourceW() **************************
 * An implementation of the Win32 API UpdateResource(). This is the
 * unicode version.
 *
 * Adds the passed resource to the internal list of resources for a DLL/EXE.
 * If another resource with the same lpType, lpName, and wLanguage already
 * exists, it is replaced.
 *
 * hUpdateResource =	Handle returned by BeginUpdateResource().
 * lpType =				DWORD Type number of resource, or pointer to a
 *						WCHAR[] containing the Type as a nul-terminated
 *						string.
 * lpName =				DWORD Name number of resource, or pointer to a
 *						WCHAR[] containing the Name as a nul-terminated
 *						string.
 * wLanguage =			Language ID number.
 * lpData =				Pointer to buffer containing the data. If 0, then
 *						the specified resource is deleted from memory.
 * cbDataSize =			Size (in bytes) of lpData.
 *
 * RETURNS: TRUE if successful, or FALSE if failure. GetLastError()
 * returns an appropriate error number.
 */

BOOL WINAPI _UpdateResourceW(HANDLE hUpdateResource, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage, LPVOID lpData, DWORD cbDataSize)
{
	LPRESOURCETYPE		type, prevtype;
	LPRESOURCENAME		name;
	LPRESOURCE			rsc;

	// Make sure that the handle is valid. This is actually the pointer to the RSRCINFO struct allocated and
	// returned by BeginUpdateResource()
	if (!hUpdateResource)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(FALSE);
	}

	// Is the Type a string pointer (instead of an ID number)?
	if (HIWORD(lpType))
	{
		// It's a string. Let's see if we can find this string in our list of strings. If not,
		// then there's no way that we could have an existing resource with this type
		if (!(lpType = addRsrcString((const LPRSRCINFO)hUpdateResource, lpType, lpData)))
		{
badmem2:	if (!lpData) goto success;
badmem:		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return(FALSE);
		}
	}

	// Is the Name a string pointer (instead of an ID number)?
	if (HIWORD(lpName) &&
		!(lpName = addRsrcString((const LPRSRCINFO)hUpdateResource, lpName, lpData)))
	{
		goto badmem2;
	}

	// If a RESOURCE with the same dwType, dwName, and wLanguage already exists in our list,
	// then set its data.

	// Assume there isn't already an existing resource
	type = 0;
	name = 0;
	rsc = 0;

	// Start with root of the list. This works because the 'Next' field is the first field in the struct
	{
	prevtype = type = (LPRESOURCETYPE)(&((LPRSRCINFO)hUpdateResource)->Types);
	while ((type = type->Next))
	{
		if (type->TypeString == lpType)
		{
			LPRESOURCENAME	prevname;

			// We matched the type, now we need to find a matching name within this type
			prevname = name = (LPRESOURCENAME)&type->Names;
			while ((name = name->Next))
			{
				if (name->NameString == lpName)
				{
					LPRESOURCE	prevrsc;

					// We matched the name, now we need to find a matching language with this name
					prevrsc = rsc = (LPRESOURCE)&name->Languages;
					while ((rsc = rsc->Next))
					{
						if (rsc->LanguageID == wLanguage)
						{
							// Found it!

							// Free any existing data for this RESOURCE
							if (rsc->Data) GlobalFree(rsc->Data);

							// If caller supplied some data, make a copy of it
							if (lpData) goto copydata;

							// Caller did not supply data, so unlink/delete this RESOURCE
							prevrsc->Next = rsc->Next;
							GlobalFree(rsc);

							// If there are no more RESOURCE's in this RESOURCENAME, then we can unlink/delete
							// the RESOURCENAME too
							if (!name->Languages)
							{
								prevname->Next = name->Next;
								GlobalFree(name);

								// If there are no more RESOURCENAME's in this RESOURCETYPE, then we can
								// unlink/delete the RESOURCETYPE too
								if (!type->Names)
								{
									prevtype->Next = type->Next;
									GlobalFree(type);
								}
							}

							goto success;
						}

						// Save this RESOURCE in case we need to unlink from the list
						prevrsc = rsc;
					}

					// We at least found a matching type and name, if not a matching language. We need
					// to allocate a RESOURCE struct now
					break;
				}

				// Save this RESOURCENAME in case we need to unlink from the list
				prevname = name;
			}

			// We at least found a matching type, if not a matching name and language. We need to
			// allocate a RESOURCENAME and RESOURCE struct now
			break;
		}

		// Save this RESOURCETYPE in case we need to unlink from the list
		prevtype = type;
	}
	}

	if (lpData)
	{
		DWORD	timeval;

		// Get the current time
		timeval = get_time();

		// Ok, do we need to make a RESOURCETYPE?
		if (!type)
		{
			// Allocate a RESOURCETYPE
			if (!(type = (LPRESOURCETYPE)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCETYPE)))) goto badmem;
			bzero(type, sizeof(RESOURCETYPE));

			// If a Type ID, put it after any Named Types. If a Named Type, put it at the head of
			// the list. That way, we keep them in order with the Named Types first
			if (HIWORD(lpType))
			{
				type->Next = ((LPRSRCINFO)hUpdateResource)->Types;
				((LPRSRCINFO)hUpdateResource)->Types = type;
			}
			else
			{
				type->Next = prevtype->Next;
				prevtype->Next = type;
			}

			// Update the time on the RESOURCETYPE
			type->TimeDateStamp = timeval;

			// Store the type name/ID
			type->TypeString = (LPWSTR)lpType;
		}

		// Ok, do we need to make a RESOURCENAME?
		if (!name)
		{
			// Allocate a RESOURCETYPE
			if (!(name = (LPRESOURCENAME)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCENAME)))) goto badmem;
			bzero(name, sizeof(RESOURCENAME));

			// If an ID, put it after any Named items. If a Name, put it at the head of
			// the list. That way, we keep them in order with the Named items first
			if (HIWORD(lpName))
			{
				name->Next = type->Names;
				type->Names = name;
			}
			else
			{
				LPRESOURCENAME	prevname;

				prevname = (LPRESOURCENAME)&type->Names;
				while (prevname->Next && HIWORD(prevname->Next->NameString)) prevname = prevname->Next;
				name->Next = prevname->Next;
				prevname->Next = name;
			}

			// Update the time on the RESOURCENAME
			name->TimeDateStamp = timeval;
			
			// Store the name/ID
			name->NameString = (LPWSTR)lpName;
		}

		// Allocate and link in a RESOURCE
		if (!(rsc = (LPRESOURCE)GlobalAlloc(GMEM_FIXED, sizeof(RESOURCE)))) goto badmem;
		bzero(rsc, sizeof(RESOURCE));
		rsc->Next = name->Languages;
		name->Languages = rsc;
		rsc->LanguageID = wLanguage;

copydata:
		if (!(rsc->Data = (char *)GlobalAlloc(GMEM_FIXED, (rsc->DataSize = cbDataSize)))) goto badmem;
		memcpy(rsc->Data, lpData, cbDataSize);
	}

success:
	SetLastError(0);
	return(TRUE);
}

////////////////////////////////////////////////////////////////////////
// This is the GetRsrcData() API. There is no equivalent WIN32 API, but
// this is meant to be used as a sort of substitute to EnumResourceTypes()
// and such APIs to enumerate resources in an EXE/DLL.

LPBYTE WINAPI _GetRsrcDataA(HANDLE hUpdateResource, LPCSTR  lpType, LPCSTR lpName, WORD wLanguage, DWORD *pDataSize)
{
	LPWSTR	lpwType, lpwName;
	LPBYTE	ptr;

	// Assume ID numbers (rather than string pointers)
	lpwType = (LPWSTR)lpType;
	lpwName = (LPWSTR)lpName;

	// Assume failure
	ptr = 0;

	// If Type is a string pointer (rather than an ID number), allocate and convert to Wide character string
	if (HIWORD(lpType))
	{
		DWORD		nLen;

		nLen = MultiByteToWideChar(CP_ACP, 0, lpType, -1, NULL, 0);
		if (!(lpwType = (LPWSTR)GlobalAlloc(GMEM_FIXED, nLen * sizeof(WCHAR)))) goto bad;
		MultiByteToWideChar(CP_ACP, 0, lpType, -1, lpwType, nLen);
	}

	// If Name is a string pointer (rather than an ID number), allocate and convert to Wide character string
	if (HIWORD(lpName))
	{
		DWORD	nLen;

		nLen = MultiByteToWideChar(CP_ACP, 0, lpName, -1, NULL, 0);
		if (!(lpwName = (LPWSTR)GlobalAlloc(GMEM_FIXED, nLen * sizeof(WCHAR))))
		{
			if (HIWORD(lpType)) GlobalFree(lpwType);
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			goto bad;
		}
		MultiByteToWideChar(CP_ACP, 0, lpName, -1, lpwName, nLen);
	}

	ptr = _GetRsrcDataW(hUpdateResource, lpwType, lpwName, wLanguage, pDataSize);

	// If pointing to strings, free the Type and Name
	if (HIWORD(lpName)) GlobalFree(lpwName);
	if (HIWORD(lpType)) GlobalFree(lpwType);
bad:
	return(ptr);
}

LPBYTE WINAPI _GetRsrcDataW(HANDLE hUpdateResource, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage, DWORD *pDataSize)
{
	if (!hUpdateResource)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(0);
	}

	// Is the Type a string pointer (instead of an ID number)?
	if (HIWORD(lpType))
	{
		LPSTRINGHEAD	lpStr;
		DWORD			size;

		// It's a string. Let's see if we can find this string in our list of strings. If not,
		// then there's no way that we could have an existing resource with this type

		size = strlen_w(lpType);

		// See if we already have a string resource matching this string. If so, use that string
		if ((lpStr = ((LPRSRCINFO)hUpdateResource)->vtrStrings))
		{
			do
			{
				if (lpStr->Size == size && !wstrnicmp_a(lpType, &lpStr->String[0])) goto gotit;
			} while ((lpStr = lpStr->Next));
		}

		// No, we don't already have this string
		goto notfound;

		// Use the string pointer in our list
gotit:	lpType = &lpStr->String[0];
	}

	// Is the Name a string pointer (instead of an ID number)?
	if (HIWORD(lpName))
	{
		LPSTRINGHEAD	lpStr;
		DWORD			size;

		size = strlen_w(lpName);

		if ((lpStr = ((LPRSRCINFO)hUpdateResource)->vtrStrings))
		{
			do
			{
				if (lpStr->Size == size && !wstrnicmp_a(lpName, &lpStr->String[0])) goto gotit2;
			} while ((lpStr = lpStr->Next));
		}

		goto notfound;

gotit2:	lpName = &lpStr->String[0];
	}

	// If a RESOURCE with the same dwType, dwName, and wLanguage already exists in our list,
	// then return its data and size

	// Start with root of the list. This works because the 'Next' field is the first field in the struct
	{
	LPRESOURCETYPE		type;
	
	type = (LPRESOURCETYPE)(&((LPRSRCINFO)hUpdateResource)->Types);
	while ((type = type->Next))
	{
		if (type->TypeString == lpType)
		{
			LPRESOURCENAME		name;

			// We matched the type, now we need to find a matching name within this type
			name = (LPRESOURCENAME)&type->Names;
			while ((name = name->Next))
			{
				if (name->NameString == lpName)
				{
					LPRESOURCE		rsc;

					// We matched the name, now we need to find a matching language with this name
					rsc = (LPRESOURCE)&name->Languages;
					while ((rsc = rsc->Next))
					{
						if (rsc->LanguageID == wLanguage)
						{
							// Found it
							if (pDataSize) *pDataSize = rsc->DataSize;
							SetLastError(0);
							return(rsc->Data);
						}
					}
				}
			}
		}
	}
	}

notfound:
	// Can't find the resource
	if (pDataSize) *pDataSize = 0;
	SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND); // was ERROR_RESOURCE_NOT_FOUND ??
	return(0);
}

////////////////////////////////////////////////////////////////////////
// This is the EndUpdateResource() API, and any helper functions.
// (Helper functions are called only by EndUpdateResource(), not by
// some application).

/*********************** calcDirTableSize() ************************
 * Calculates the size (in bytes) of all of the RSRCDIRTABLEHEAD,
 * RSRCDIRENTRY, and RSRCDATAENTRY structs needed to store our
 * resources in the EXE/DLL's '.rsrc' section. Also calculates the
 * size of all raw data.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 *
 * RETURN: Size in bytes.
 */

static DWORD calcDirTableSize(const LPRSRCINFO pInfo, DWORD *raw)
{
	LPRESOURCETYPE	type;
	DWORD			size, rawsize;

	// We need a master RSRCDIRTABLEHEAD
	size = sizeof(RSRCDIRTABLEHEAD);

	rawsize = 0;

	type = (LPRESOURCETYPE)&pInfo->Types;
	while ((type = type->Next))
	{
		LPRESOURCENAME		name;

		// We need a RSRCDIRTABLEHEAD and RSRCDIRENTRY for each Type
		size += sizeof(RSRCDIRENTRY) + sizeof(RSRCDIRTABLEHEAD);

		name = (LPRESOURCENAME)&type->Names;
		while ((name = name->Next))
		{
			LPRESOURCE		rsc;
			
			// We need a RSRCDIRTABLEHEAD and RSRCDIRENTRY for each Name under this Type
			size += sizeof(RSRCDIRENTRY) + sizeof(RSRCDIRTABLEHEAD);

			rsc = (LPRESOURCE)&name->Languages;
			while ((rsc = rsc->Next))
			{
				// We need a RSRCDIRENTRY and RSRCDATAENTRY for each Language under this Type/Name
				size += sizeof(RSRCDIRENTRY) + sizeof(RSRCDATAENTRY);

				rawsize += rsc->DataSize;

				// NOTE: Each resource's individual size must be rounded up to the
				// nearest DWORD
				DWORD_ALIGN(rawsize);
			}
		}
	}

	// Store size of raw data
	*raw = rawsize;

	// Return size of dir structs
	return(size);
}

/*********************** calcStringTableSize() ************************
 * Calculates the size (in bytes) of all of the strings in our list of
 * strings, when they are saved into a StringTable in the EXE/DLL.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 *
 * RETURN: Size in bytes.
 *
 * NOTE: Each string is saved as a WCHAR[] with the first WORD being
 * the string's length. Think of it like a pascal-style string but
 * as a WCHAR[] instead of char[].
 */

static DWORD calcStringTableSize(const LPRSRCINFO pInfo)
{
	LPSTRINGHEAD	lpStr;
	DWORD			size;

	size = 0;

	if ((lpStr = pInfo->vtrStrings))
	{
		do
		{
			// NOTE: +1 for the length WORD. x2 to count 2 bytes for each WCHAR
			size += ((lpStr->Size + 1) << 1);
		} while ((lpStr = lpStr->Next));
	}

	return(size);
}

static void calcNumDirEntries(LPRESOURCENAME list, WORD *numNames, WORD *numIDs)
{
	WORD names;
	WORD ids;

	names = ids = 0;

	if (list)
	{
		do
		{
			if (HIWORD(list->NameString)) names++;
			else ids++;
		} while ((list = list->Next));
	}

	*numNames = names;
	*numIDs = ids;
}

/************************ getStringOffset() *************************
 * Retrieves the offset of the specified string within the .rsrc
 * section. This offset was set by generateStringTable().
 *
 * string =			Pointer to string. This pointer must be to some
 *					STRINGHEAD in our list.
 *
 * RETURNS: The offset in bytes.
 */

static DWORD getStringOffset(LPWSTR string)
{
	LPSTRINGHEAD	lpStr;

	lpStr = (LPSTRINGHEAD)((char *)string - sizeof(STRINGHEAD) + 2);
	return(lpStr->Size  | (1 << 31));
}

/********************** generateStringTable() ***********************
 * Creates an image (in memory) of the string table portion of the
 * '.rsrc' section that will be written to the EXE/DLL.
 *
 * pInfo =			Pointer to RSRCINFO for the current exe/dll.
 * pRsrc =			A block of memory into which the image will be created.
 * offset =			Byte offset to where the string table will start
 *					within the .rsrc.
 */

static void generateStringTable(const LPRSRCINFO pInfo, WORD *pStringTable, DWORD offset)
{
	LPSTRINGHEAD	lpStr;
	WORD			len;

	lpStr = (LPSTRINGHEAD)&pInfo->vtrStrings;
	while ((lpStr = lpStr->Next))
	{
		// Get the length in WORDs
		len = (WORD)lpStr->Size;

		// Change the STRINGHEAD's Size to an offset expressed in bytes
		lpStr->Size = offset;

		// Copy the length
		*(pStringTable)++ = len;

		// Copy the string
		len <<= 1;
		memcpy(pStringTable, &lpStr->String[0], len);

		// Next position in string table
		pStringTable = (WORD *)((char *)pStringTable + len);
		offset += len + 2;
	}
}

/************************* generateCode() **************************
 * Creates an image (in memory) of the '.rsrc' section that will be
 * written to the EXE/DLL. This includes all of the directory
 * structures, and resource data.
 *
 * pInfo =			Pointer to RSRCINFO for the current exe/dll.
 * pRsrc =			A block of memory into which the image will be created.
 * RawDataOffset =	Byte offset to where the resource data will start
 *					within the EXE/DLL. (This is after the string table).
 *
 * NOTE: The string table (for Named Types and Named Items) is
 * formatted by generateStringTable().
 */

static void generateCode(const LPRSRCINFO pInfo, char *pRsrc, DWORD RawDataOffset)
{
	RSRCDIRTABLEHEAD	TypeHead, NameHead, LangHead;
	LPRESOURCETYPE		type;
	LPRESOURCENAME		name;
	LPRESOURCE			rsc;
	DWORD				TableAddress;
	RSRCDIRENTRY		*typeEntry;

	// Create master RSRCDIRTABLEHEAD

	// Copy over the Characteristics, TimeDateStamp, VersionMajor, and VersionMinor
	memcpy(&TypeHead.Characteristics, &pInfo->Characteristics, sizeof(RESOURCEINFO));

	// Set the NumNameEntries and NumIDEntries
	calcNumDirEntries((LPRESOURCENAME)pInfo->Types, &TypeHead.NumNameEntries, &TypeHead.NumIDEntries);

	memcpy(pRsrc, &TypeHead, sizeof(RSRCDIRTABLEHEAD));

	// Calculate offset to where we'll start the RSRCDIRTABLEHEAD for Named Type 1
	TableAddress = sizeof(RSRCDIRTABLEHEAD) + ((TypeHead.NumNameEntries + TypeHead.NumIDEntries) * sizeof(RSRCDIRENTRY));

	// Do all Types (starting with Named Types)
	type = (LPRESOURCETYPE)&pInfo->Types;
	typeEntry = (RSRCDIRENTRY *)(pRsrc + sizeof(RSRCDIRTABLEHEAD));
	while ((type = type->Next))
	{
		RSRCDIRENTRY	*pEntry;
		DWORD			NameHeadAddress;

		// Create the RSRCDIRENTRY for this Type

		// Set the offset to the string within the StringTable if a Named item
		if (HIWORD(type->TypeString))
			typeEntry->NameRVA = getStringOffset(type->TypeString);

		// Set the integer ID if an ID number
		else
			typeEntry->IntegerID = type->TypeID;

		// Set the offset to the item's RSRCDIRTABLEHEAD
		typeEntry->SubdirectoryRVA = TableAddress | (1<<31);

		typeEntry++;

		// Create RSRCDIRTABLEHEAD for this Type

		// Copy over the Characteristics, TimeDateStamp, VersionMajor, and VersionMinor
		memcpy(&NameHead.Characteristics, &type->Characteristics, sizeof(RESOURCEINFO));

		// Set the NumNameEntries and NumIDEntries
		calcNumDirEntries((LPRESOURCENAME)type->Names, &NameHead.NumNameEntries, &NameHead.NumIDEntries);

		memcpy(&pRsrc[TableAddress], &NameHead, sizeof(RSRCDIRTABLEHEAD));

		// Calculate offset to where we'll start the RSRCDIRENTRY for Named Item 1 for Named Type 1
		NameHeadAddress = TableAddress + sizeof(RSRCDIRTABLEHEAD);

		// Calculate offset to where we'll start the RSRCDIRTABLEHEAD for Named Item 1 for Named Type 1
		TableAddress += sizeof(RSRCDIRTABLEHEAD) + (NameHead.NumNameEntries + NameHead.NumIDEntries) * sizeof(RSRCDIRENTRY);

		// Create the RSRCDIRENTRY table for all items for this Type, starting with Named items
		name = (LPRESOURCENAME)&type->Names;
		while (name = name->Next)
		{
			RSRCDATAENTRY	*pDataEntry;
			DWORD			Lang;

			// Create the RSRCDIRTABLEHEAD for this Item

			// Copy over the Characteristics, TimeDateStamp, VersionMajor, and VersionMinor
			memcpy(&LangHead.Characteristics, &name->Characteristics, sizeof(RESOURCEINFO));

			// Set the NumNameEntries and NumIDEntries
			LangHead.NumNameEntries = LangHead.NumIDEntries = 0;
			rsc = (LPRESOURCE)&name->Languages;
			while (rsc = rsc->Next) LangHead.NumIDEntries++;

			memcpy(&pRsrc[TableAddress], &LangHead, sizeof(RSRCDIRTABLEHEAD));

			// Create the RSRCDIRENTRY for this Item
			pEntry = (RSRCDIRENTRY *)(pRsrc + NameHeadAddress);
			NameHeadAddress += sizeof(RSRCDIRENTRY);

			// Set the offset to the string within the StringTable if a Named item
			if (HIWORD(name->NameString))
				pEntry->NameRVA = getStringOffset(name->NameString);

			// Set the integer ID if an ID number
			else
				pEntry->IntegerID = name->NameID;

			// Set the offset to the item's RSRCDIRTABLEHEAD
			pEntry->SubdirectoryRVA = TableAddress | (1<<31);

			// Calculate offset to where we'll start the RSRCDIRENTRY for Language 1 for Named Item 1 for Named Type 1
			Lang = TableAddress + sizeof(RSRCDIRTABLEHEAD);
			pEntry = (RSRCDIRENTRY *)(pRsrc + Lang);

			// Calculate offset to where we'll start the RSRCDATAENTRY for Language 1 for Named Item 1 for Named Type 1
			TableAddress = Lang + ((LangHead.NumNameEntries + LangHead.NumIDEntries) * sizeof(RSRCDIRENTRY));
			pDataEntry = (RSRCDATAENTRY *)(pRsrc + TableAddress);

			// Create the RSRCDATAENTRY and RSRCDIRENTRY tables for all Languages for this Item
			rsc = (LPRESOURCE)&name->Languages;
			while (rsc = rsc->Next)
			{
				// Create next RSRCDATAENTRY for this Language's data block
				pDataEntry->DataRVA = RawDataOffset + pInfo->SectionHeader.VirtualAddress;
				pDataEntry->Size = rsc->DataSize;
				pDataEntry->Codepage = rsc->Codepage;
				pDataEntry->Reserved = rsc->Reserved;
				++pDataEntry;

				// Copy data block
				memcpy(pRsrc + RawDataOffset, rsc->Data, rsc->DataSize);

				// Calculate offset to next data block
				RawDataOffset += rsc->DataSize;
				DWORD_ALIGN(RawDataOffset);

				// Create next RSRCDIRENTRY for this Language
				pEntry->IntegerID = rsc->LanguageID;
				pEntry->DataEntryRVA = TableAddress;
				++pEntry;

				TableAddress += sizeof(RSRCDATAENTRY);
			}
		}
	}
}

/******************** removeUnreferencedStrings() ********************
 * Removes strings (from our list of strings) which are no longer
 * referenced by any resource. This is called in preparation of
 * writing out the StringTable to the EXE/DLL. We don't want to put
 * unneeded strings into that table.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 */

static void removeUnreferencedStrings(LPRSRCINFO pInfo)
{
	LPSTRINGHEAD	str;
	LPSTRINGHEAD	prevstr;

	// Go through the list of strings, and check each one
	prevstr = (LPSTRINGHEAD)&pInfo->vtrStrings;
next:
	while ((str = prevstr->Next))
	{
		LPRESOURCETYPE	type;

		// Go through the list of resources and check to see if any
		// RESOURCE's TypeString or NameString point to this string
		type = (LPRESOURCETYPE)&pInfo->Types;
		while ((type = type->Next))
		{
			LPRESOURCENAME		name;

			if (type->TypeString == &str->String[0]) goto next2;
			name = (LPRESOURCENAME)&type->Names;
			while ((name = name->Next))
			{
				if (name->NameString == &str->String[0])
				{
next2:				prevstr = str;
					goto next;
				}
			}
		}

		// If we didn't find a RESOURCE referencing this string above, then unlink the string
		// from the list and delete it
		prevstr->Next = str->Next;
		GlobalFree(str);
	}
}

/*********************** cleanupRsrcInfo() ************************
 * Frees the RSRCINFO and any allocated resources for it.
 *
 * pInfo =		Pointer to RSRCINFO for the current exe/dll.
 */

static void cleanupRsrcInfo(LPRSRCINFO pInfo)
{
	// Close the EXE/DLL handle
	CloseHandle(pInfo->File);

	// Go through the list of resources and free the data
	{
	LPRESOURCETYPE	type, typenext;

	typenext = pInfo->Types;
	while ((type = typenext))
	{
		LPRESOURCENAME		name, namenext;

		namenext = type->Names;
		while ((name = namenext))
		{
			LPRESOURCE	rsc, rscnext;

			rscnext = name->Languages;
			while ((rsc = rscnext))
			{
				if (rsc->Data) GlobalFree(rsc->Data);
				rscnext = rsc->Next;
				GlobalFree(rsc);
			}

			namenext = name->Next;
			GlobalFree(name);
		}

		typenext = type->Next;
		GlobalFree(type);
	}
	}

	// Go through the list of strings, and free them
	{
	LPSTRINGHEAD	str, strnext;

	strnext = pInfo->vtrStrings;
	while ((str = strnext))
	{
		strnext = str->Next;
		GlobalFree(str);
	}
	}

	// Free the RSRCINFO itself
	GlobalFree(pInfo);
}

/************************** relocateSections() *************************
 * Rewrites the sections of the specified exe/dll file in preparation
 * for writing out the resources.
 *
 * pInfo =			Pointer to RSRCINFO for the current exe/dll.
 * newRsrcSize =	Byte size of the new .rsrc section.
 *
 * RETURN: ERROR_SUCCESS if success, or otherwise if error.
 */

static DWORD relocateSections(LPRSRCINFO pInfo, DWORD newRsrcSize, DWORD alignedRsrcSize, IMAGE_OPTIONAL_HEADER *oh)
{
	IMAGE_SECTION_HEADER	sh;
	DWORD					alignedImageSize, result;
	DWORD					section;

	SetFilePointer(pInfo->File, pInfo->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER) + pInfo->PEHeader.SizeOfOptionalHeader, 0, FILE_BEGIN);

	section = pInfo->PEHeader.NumberOfSections;
	if (newRsrcSize < pInfo->SectionHeader.SizeOfRawData)
	{
		// We don't have to relocate anything - just change the size
		while (section--)
		{
			if (!ReadFile(pInfo->File, &sh, sizeof(IMAGE_SECTION_HEADER), &result, NULL) || result != sizeof(IMAGE_SECTION_HEADER))
			{
bad:			return(GetLastError());
			}

			if (!strcasecmp((char *)sh.Name, &RsrcSectionName[0]))
			{
				sh.SizeOfRawData = alignedRsrcSize;
				sh.Misc.VirtualSize = newRsrcSize;
				SetFilePointer(pInfo->File, - sizeof(IMAGE_SECTION_HEADER), 0, FILE_CURRENT);
				if (!WriteFile(pInfo->File, &sh, sizeof(IMAGE_SECTION_HEADER), &result, NULL) || result != sizeof(IMAGE_SECTION_HEADER)) goto bad;
				alignedImageSize = oh->SizeOfImage - sh.Misc.VirtualSize + newRsrcSize;

				alignedImageSize = (alignedImageSize + oh->SectionAlignment - 1) & ~(oh->SectionAlignment - 1);
//				if (alignedImageSize % oh->SectionAlignment) alignedImageSize += (alignedImageSize % oh->SectionAlignment);

				break;
			}
		}

		SetFilePointer(pInfo->File, pInfo->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER) + offsetof(IMAGE_OPTIONAL_HEADER, DataDirectory[2]) + 4, 0, FILE_BEGIN);
		if (!WriteFile(pInfo->File, &newRsrcSize, 4, &result, NULL) || result != 4) goto bad;

		SetFilePointer(pInfo->File, pInfo->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfImage), 0, FILE_BEGIN);
		if (!WriteFile(pInfo->File, &alignedImageSize, 4, &result, NULL) || result != 4) goto bad;

		return(ERROR_SUCCESS);
	}

	{
	DWORD filePtrToRsrc = 0;

	while (section--)
	{
		if (!ReadFile(pInfo->File, &sh, sizeof(IMAGE_SECTION_HEADER), &result, NULL) || result != sizeof(IMAGE_SECTION_HEADER)) goto bad;

		if (sh.VirtualAddress > pInfo->SectionHeader.VirtualAddress || sh.PointerToRawData > pInfo->SectionHeader.PointerToRawData)
			return(ERROR_CANT_MOVE_SECTION);

		if (!filePtrToRsrc && !strcasecmp((unsigned char *)sh.Name, &RsrcSectionName[0]))
		{
			filePtrToRsrc = SetFilePointer(pInfo->File, 0, 0, FILE_CURRENT) - sizeof(IMAGE_SECTION_HEADER);
			alignedImageSize = oh->SizeOfImage - sh.Misc.VirtualSize + newRsrcSize;
			alignedImageSize = (alignedImageSize + oh->SectionAlignment - 1) & ~(oh->SectionAlignment - 1);
//			if (alignedImageSize % oh->SectionAlignment) alignedImageSize += (alignedImageSize % oh->SectionAlignment);
		}
	}

	SetFilePointer(pInfo->File, filePtrToRsrc, 0, FILE_BEGIN);

	pInfo->SectionHeader.SizeOfRawData = alignedRsrcSize;
	pInfo->SectionHeader.Misc.VirtualSize = newRsrcSize;
	if (!WriteFile(pInfo->File, &pInfo->SectionHeader, sizeof(IMAGE_SECTION_HEADER), &result, NULL) || result != sizeof(IMAGE_SECTION_HEADER)) goto bad;

	SetFilePointer(pInfo->File, pInfo->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER) + offsetof(IMAGE_OPTIONAL_HEADER, DataDirectory[2]) + 4, 0, FILE_BEGIN);
	if (!WriteFile(pInfo->File, &newRsrcSize, 4, &result, NULL) || result != 4) goto bad;

	SetFilePointer(pInfo->File, pInfo->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfImage), 0, FILE_BEGIN);
	if (!WriteFile(pInfo->File, &alignedImageSize, 4, &result, NULL) || result != 4) goto bad;
	}

	return(ERROR_SUCCESS);
}

/************************** _EndUpdateResource() *************************
 * Writes the resources in memory to the specified exe/dll file, and frees
 * the loaded resources.
 *
 * hUpdateResource =	Pointer to RSRCINFO for the current exe/dll.
 * fDiscard =			TRUE if the resources should not be written to
 *						the exe/dll.
 *
 * RETURN: TRUE if success, or FALSE if error.
 */

BOOL WINAPI _EndUpdateResource(HANDLE hUpdateResource, BOOL fDiscard)
{
	if (!hUpdateResource)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(FALSE);
	}

	// Do not discard the changes?
	if (!fDiscard)
	{
		IMAGE_OPTIONAL_HEADER	oh;
		DWORD					dirTableSize, stringTableSize, rawDataSize, totalSize, alignedSize;
		DWORD					error;
		unsigned char			*pRsrc;

		// We need to save our internal table of RESOURCES to the EXE/DLL file, modifying the actual executable

		// Get rid of strings that are no longer referenced by some resource
		removeUnreferencedStrings((LPRSRCINFO)hUpdateResource);

		// Seek to where to PE Header begins (actually, after the IMAGE_FILE_HEADER)
		SetFilePointer(((LPRSRCINFO)hUpdateResource)->File,
						((LPRSRCINFO)hUpdateResource)->FilePtrToPEHdr + sizeof(IMAGE_FILE_HEADER), 0, FILE_BEGIN);

		// Read in the IMAGE_OPTIONAL_HEADER
		if (!ReadFile(((LPRSRCINFO)hUpdateResource)->File, &oh, sizeof(IMAGE_OPTIONAL_HEADER), &error, NULL) || error != sizeof(IMAGE_OPTIONAL_HEADER))
		{
bad2:		error = GetLastError();
			goto bad;
		}

		// First we calculate all sizes and put the '.rsrc' section last in the dll/exe file and Virtual memory
		dirTableSize = calcDirTableSize((LPRSRCINFO)hUpdateResource, &rawDataSize);
		stringTableSize = calcStringTableSize((LPRSRCINFO)hUpdateResource);

		DWORD_ALIGN(dirTableSize);
		DWORD_ALIGN(stringTableSize);

		alignedSize = totalSize = dirTableSize + stringTableSize + rawDataSize;
		alignedSize = (totalSize + oh.FileAlignment - 1) & ~(oh.FileAlignment - 1);

		if ((error = relocateSections((LPRSRCINFO)hUpdateResource, totalSize, alignedSize, &oh)) != ERROR_SUCCESS)
		{
bad:		cleanupRsrcInfo((LPRSRCINFO)hUpdateResource);
			SetLastError(error);
			return(FALSE);
		}

		// Allocate a block of memory to hold all of the directory structures, StringTable, and resource data. We will
		// be forming the '.rsrc' section in memory, and then writing it out to the EXE/DLL file
		if (!(pRsrc = GlobalAlloc(GMEM_FIXED, alignedSize))) goto bad2;
		bzero(pRsrc, alignedSize);

		// Form the stringtable within the '.rsrc' section
		generateStringTable((LPRSRCINFO)hUpdateResource, (WORD *)(pRsrc + dirTableSize), dirTableSize);

		// Form the rest of the '.rsrc' section
		generateCode((LPRSRCINFO)hUpdateResource, pRsrc, dirTableSize + stringTableSize);

		// Write '.rsrc' to file
		SetFilePointer(((LPRSRCINFO)hUpdateResource)->File, 
			((LPRSRCINFO)hUpdateResource)->SectionHeader.PointerToRawData, 0, FILE_BEGIN);
		if (!WriteFile(((LPRSRCINFO)hUpdateResource)->File, pRsrc, alignedSize, &error, NULL) || error != alignedSize)
		{
			GlobalFree(pRsrc);
			goto bad2;
		}

		// Free the image in memory now that it is written out
		GlobalFree(pRsrc);
	}

	// Free up the RSRCINFO and its lists
	cleanupRsrcInfo((LPRSRCINFO)hUpdateResource);

	// Success
	return(TRUE);
}



#if 0

/******************************** DllMain() ********************************
 * Automatically called by Win32 when the DLL is loaded or unloaded.
 ***************************************************************************/

#if defined(_MSC_VER)
#ifndef _DEBUG
BOOL WINAPI _DllMainCRTStartup(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)  /* <--- Doesn't replace startup code */
#endif
#else
BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#endif
{
    switch(fdwReason)
	{
		/* ============================================================== */
		case DLL_PROCESS_ATTACH:
		{
			/*
			 * Here you would do any initialization that you want to do when
			 * the DLL loads. The OS calls this every time another program
			 * runs which uses this DLL. You should put some complementary
			 * cleanup code in the DLL_PROCESS_DETACH case.
			 */
			break;
		}

		/* ============================================================== */
		case DLL_THREAD_ATTACH:
		{
			/* We don't need to do any initialization for each thread of
			 * the program which uses this DLL, so disable thread messages.
			 */
			DisableThreadLibraryCalls(hinstDLL);
			break;
		}

/*
		case DLL_THREAD_DETACH:
		{
			break;
		}
*/
		/* ============================================================== */
		case DLL_PROCESS_DETACH:
		{
		}
	}

	/* Success */
	return(1);
}
#endif
