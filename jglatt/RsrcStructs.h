#ifndef __RSRCSTRUCTS_H__
#define __RSRCSTRUCTS_H__

// Structures found embedded in an EXE/DLL's resources

#define DWORD_ALIGN(i) (i = (i + 3) & ~3)

typedef struct {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD  VersionMajor;
	WORD  VersionMinor;
	WORD  NumNameEntries;
	WORD  NumIDEntries;
} RSRCDIRTABLEHEAD;

typedef struct {
	union {
		DWORD NameRVA;
		DWORD IntegerID;
	};
	union {
		DWORD DataEntryRVA;
		DWORD SubdirectoryRVA;
	};
} RSRCDIRENTRY;

typedef struct {
	DWORD DataRVA;
	DWORD Size;
	DWORD Codepage;
	DWORD Reserved;
} RSRCDATAENTRY;





// Internal structures we use to maintain information about an EXE/DLL's loaded resources

typedef struct {
	DWORD	Characteristics;
	DWORD	TimeDateStamp;
	WORD	VersionMajor;
	WORD	VersionMinor;
} RESOURCEINFO, *LPRESOURCEINFO;

#pragma pack(1)
typedef struct StringHead {

	// To link the next StringHead in a linked list. Must be the first field!
	struct StringHead	*Next;

	// Size of the string
	DWORD				Size;

	// The actual string is embedded here, so this struct's size can vary
	WCHAR				String[1];

} STRINGHEAD, *LPSTRINGHEAD;
#pragma pack()

typedef struct Resource {

	// To link the next Resource in a linked list
	struct Resource		*Next;

	// Information associated with this resource
	LPBYTE				Data;
	DWORD				DataSize;
	DWORD				Codepage;
	DWORD				Reserved;
	WORD				LanguageID;
} RESOURCE, *LPRESOURCE;

// Holds info about each Named instance of a given Type of resource in the EXE/DLL
typedef struct ResourceName {

	// To link the next RESOURCENAME in a linked list. Must be the first field!
	struct ResourceName	*Next;

	// The resource Name string or number. Must be the second field!
	union {
	DWORD				NameID;
	LPWSTR				NameString;
	};

	// A linked list of the Language Resources for this Type/Name
	LPRESOURCE			Languages;

	// Information associated with this resource Type
	DWORD				Characteristics;
	DWORD				TimeDateStamp;
	WORD				VersionMajor;
	WORD				VersionMinor;

} RESOURCENAME, *LPRESOURCENAME;

// Holds info about each Type of resource in the EXE/DLL
typedef struct ResourceType {

	// To link the next RESOURCETYPE in a linked list. Must be the first field!
	struct ResourceType	*Next;

	// The resource Type string or number. Must be the second field!
	union {
	DWORD				TypeID;
	LPWSTR				TypeString;
	};

	// A linked list of the Named/ID'ed resources for this Type
	LPRESOURCENAME		Names;

	// Information associated with this resource Type
	DWORD				Characteristics;
	DWORD				TimeDateStamp;
	WORD				VersionMajor;
	WORD				VersionMinor;

} RESOURCETYPE, *LPRESOURCETYPE;

// An internal structure we use to maintain information about an EXE/DLL's loaded resources
typedef struct RsrcInfo {
	// Handle to open file
	HANDLE				*File;

	// Byte offset within file to the resources
	DWORD				FilePtrToPEHdr;

	// To load the Section Header
	IMAGE_SECTION_HEADER SectionHeader;

	// To load the PE Header
	IMAGE_FILE_HEADER    PEHeader;

	// A linked list of the Named/ID'ed Types in the EXE/DLL
	LPRESOURCETYPE		Types;

	// Information from the master RsrcDirTableHead in the EXE/DLL
	DWORD				Characteristics;
	DWORD				TimeDateStamp;
	WORD				VersionMajor;
	WORD				VersionMinor;

	// A linked list of strings
	LPSTRINGHEAD		vtrStrings;

} RSRCINFO, *LPRSRCINFO;

// Used for get_time()
typedef struct {
	int tm_sec;     // Seconds [0 to 59]
	int tm_min;     // Minutes [0 to 59]
	int tm_hour;    // Hour [0 to 23, where 0 = Midnight]
	int tm_mday;    // Day of the month [1 to 31]
	int tm_mon;     // Months [0 to 11, where 0 = Jan]
	int tm_year;    // Current Year - 1900
	int tm_wday;    // Day of week [0 to 6 where 0 = Sunday]
	int tm_yday;    // Day of the Year [0 to 365 where 0 = Jan 1]
	int tm_isdst;   // Daylight Savings Time flag [0=none, 1=yes]
} TM;

#define offsetof(s,m)   (size_t)&(((s *)0)->m)

#endif //__RSRCSTRUCTS_H__
