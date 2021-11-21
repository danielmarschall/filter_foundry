/*
    32/64 bit "standalone" ASCII Resource Modification methods for Win9x compatibility
    includes many fixes (marked with "Fix by Daniel Marschall" comment)
    Copyright (C) 2021 Daniel Marschall, ViaThinkSoft
    based on the code of the Wine Project
        Copyright 1993 Robert J. Amstadt
        Copyright 1995, 2003 Alexandre Julliard
        Copyright 2006 Mike McCormack
     and its linked list support
        Copyright (C) 2002 Alexandre Julliard

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

/*

TODO / Things that don't work correctly:
- Resources cannot be deleted correctly: see https://bugs.winehq.org/show_bug.cgi?id=52046
- Errors, e.g. in EndUpdateResource cannot be retrieved by GetLastError
- The field "Size of initialized data" of an OpenWatcom compiled image got changed by these procedures.
  Not sure if this would break something! We should be very careful with that field, since it affects code rather than resources?!

*/

#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#ifndef DUMMYUNIONNAME
#if defined(NONAMELESSUNION) || !defined(_MSC_EXTENSIONS)
#define DUMMYUNIONNAME   u
#define DUMMYUNIONNAME2  u2
#define DUMMYUNIONNAME3  u3
#define DUMMYUNIONNAME4  u4
#define DUMMYUNIONNAME5  u5
#define DUMMYUNIONNAME6  u6
#define DUMMYUNIONNAME7  u7
#define DUMMYUNIONNAME8  u8
#define DUMMYUNIONNAME9  u9
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#define DUMMYUNIONNAME5
#define DUMMYUNIONNAME6
#define DUMMYUNIONNAME7
#define DUMMYUNIONNAME8
#define DUMMYUNIONNAME9
#endif
#endif // DUMMYUNIONNAME

#ifndef DUMMYSTRUCTNAME
#if defined(NONAMELESSUNION) || !defined(_MSC_EXTENSIONS)
#define DUMMYSTRUCTNAME  s
#define DUMMYSTRUCTNAME2 s2
#define DUMMYSTRUCTNAME3 s3
#define DUMMYSTRUCTNAME4 s4
#define DUMMYSTRUCTNAME5 s5
#else
#define DUMMYSTRUCTNAME
#define DUMMYSTRUCTNAME2
#define DUMMYSTRUCTNAME3
#define DUMMYSTRUCTNAME4
#define DUMMYSTRUCTNAME5
#endif
#endif // DUMMYSTRUCTNAME

//#define WIN32_NO_STATUS

#include "compat_win_resource.h"

//#include "ntstatus.h"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((DWORD   )0x00000000L)    
#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// #include "winnt.h" (renamed)
// Required because built-in OpenWatcom structs don't have dummy union names
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct _MyIMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            DWORD NameOffset : 31;
            DWORD NameIsString : 1;
        } DUMMYSTRUCTNAME;
        DWORD   Name;
        WORD    Id;
    } DUMMYUNIONNAME;
    union {
        DWORD   OffsetToData;
        struct {
            DWORD   OffsetToDirectory : 31;
            DWORD   DataIsDirectory : 1;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;
} MyIMAGE_RESOURCE_DIRECTORY_ENTRY, * PMyIMAGE_RESOURCE_DIRECTORY_ENTRY;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// #include "winternl.h"
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef CONST char* PCSZ;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Supplement created by Daniel Marschall
// Attention: These supplements are VERY simple and they ONLY accept ASCII!
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

NTSTATUS WINAPI _RtlCreateUnicodeStringFromAsciiz(PUNICODE_STRING target, LPCSTR src)
{
    // Simple implementation by Daniel Marschall
    size_t i;

    target->Length = (USHORT)(strlen(src) * sizeof(WCHAR));
    target->MaximumLength = target->Length * sizeof(WCHAR) + sizeof(WCHAR)/*NUL*/;
    if (!(target->Buffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, target->MaximumLength)))
        return STATUS_NO_MEMORY;
    memset(target->Buffer, 0, target->MaximumLength);

    for (i = 0; i < strlen(src); i++) {
        // C++ wrong warning: Buffer overflow (C6386)
        #pragma warning(suppress : 6386)
        target->Buffer[i] = (WCHAR)src[i];
    }

    return STATUS_SUCCESS;
}

NTSTATUS WINAPI _InplaceRtlUpcaseUnicodeString(PUNICODE_STRING str) {
    int i;

    for (i = 0; i < (int)(str->Length / sizeof(WCHAR)); i++) {
        str->Buffer[i] = toupper(str->Buffer[i]);
    }

    return STATUS_SUCCESS;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Source: https://github.com/reactos/wine/blob/master/dlls/ntdll/rtlstr.c (renamed function)
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

NTSTATUS WINAPI _RtlCharToInteger(
    PCSZ str,      /* [I] '\0' terminated single-byte string containing a number */
    ULONG base,    /* [I] Number base for conversion (allowed 0, 2, 8, 10 or 16) */
    ULONG* value)  /* [O] Destination for the converted value */
{
    CHAR chCurrent;
    int digit;
    ULONG RunningTotal = 0;
    char bMinus = 0;

    while (*str != '\0' && *str <= ' ') {
        str++;
    } /* while */

    if (*str == '+') {
        str++;
    }
    else if (*str == '-') {
        bMinus = 1;
        str++;
    } /* if */

    if (base == 0) {
        base = 10;
        if (str[0] == '0') {
            if (str[1] == 'b') {
                str += 2;
                base = 2;
            }
            else if (str[1] == 'o') {
                str += 2;
                base = 8;
            }
            else if (str[1] == 'x') {
                str += 2;
                base = 16;
            } /* if */
        } /* if */
    }
    else if (base != 2 && base != 8 && base != 10 && base != 16) {
        return STATUS_INVALID_PARAMETER;
    } /* if */

    if (value == NULL) {
        return STATUS_ACCESS_VIOLATION;
    } /* if */

    while (*str != '\0') {
        chCurrent = *str;
        if (chCurrent >= '0' && chCurrent <= '9') {
            digit = chCurrent - '0';
        }
        else if (chCurrent >= 'A' && chCurrent <= 'Z') {
            digit = chCurrent - 'A' + 10;
        }
        else if (chCurrent >= 'a' && chCurrent <= 'z') {
            digit = chCurrent - 'a' + 10;
        }
        else {
            digit = -1;
        } /* if */
        if (digit < 0 || (ULONG)digit >= base) {
            *value = bMinus ? -(int)RunningTotal : RunningTotal;
            return STATUS_SUCCESS;
        } /* if */

        RunningTotal = RunningTotal * base + digit;
        str++;
    } /* while */

    *value = bMinus ? -(int)RunningTotal : RunningTotal;
    return STATUS_SUCCESS;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Source: https://raw.githubusercontent.com/wine-mirror/wine/master/include/wine/list.h
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * Linked lists support
 *
 * Copyright (C) 2002 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __WINE_SERVER_LIST_H
#define __WINE_SERVER_LIST_H

#include <stddef.h>

struct list
{
    struct list* next;
    struct list* prev;
};

/* Define a list like so:
 *
 *   struct gadget
 *   {
 *       struct list  entry;   <-- doesn't have to be the first item in the struct
 *       int          a, b;
 *   };
 *
 *   static struct list global_gadgets = LIST_INIT( global_gadgets );
 *
 * or
 *
 *   struct some_global_thing
 *   {
 *       struct list gadgets;
 *   };
 *
 *   list_init( &some_global_thing->gadgets );
 *
 * Manipulate it like this:
 *
 *   list_add_head( &global_gadgets, &new_gadget->entry );
 *   list_remove( &new_gadget->entry );
 *   list_add_after( &some_random_gadget->entry, &new_gadget->entry );
 *
 * And to iterate over it:
 *
 *   struct gadget *gadget;
 *   LIST_FOR_EACH_ENTRY( gadget, &global_gadgets, struct gadget, entry )
 *   {
 *       ...
 *   }
 *
 */

 /* add an element after the specified one */
static inline void list_add_after(struct list* elem, struct list* to_add)
{
    to_add->next = elem->next;
    to_add->prev = elem;
    elem->next->prev = to_add;
    elem->next = to_add;
}

/* add an element before the specified one */
static inline void list_add_before(struct list* elem, struct list* to_add)
{
    to_add->next = elem;
    to_add->prev = elem->prev;
    elem->prev->next = to_add;
    elem->prev = to_add;
}

/* add element at the head of the list */
static inline void list_add_head(struct list* list, struct list* elem)
{
    list_add_after(list, elem);
}

/* add element at the tail of the list */
static inline void list_add_tail(struct list* list, struct list* elem)
{
    list_add_before(list, elem);
}

/* remove an element from its list */
static inline void list_remove(struct list* elem)
{
    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
}

/* get the next element */
static inline struct list* list_next(const struct list* list, const struct list* elem)
{
    struct list* ret = elem->next;
    if (elem->next == list) ret = NULL;
    return ret;
}

/* get the previous element */
static inline struct list* list_prev(const struct list* list, const struct list* elem)
{
    struct list* ret = elem->prev;
    if (elem->prev == list) ret = NULL;
    return ret;
}

/* get the first element */
static inline struct list* list_head(const struct list* list)
{
    return list_next(list, list);
}

/* get the last element */
static inline struct list* list_tail(const struct list* list)
{
    return list_prev(list, list);
}

/* check if a list is empty */
static inline int list_empty(const struct list* list)
{
    return list->next == list;
}

/* initialize a list */
static inline void list_init(struct list* list)
{
    list->next = list->prev = list;
}

/* count the elements of a list */
static inline unsigned int list_count(const struct list* list)
{
    unsigned count = 0;
    const struct list* ptr;
    for (ptr = list->next; ptr != list; ptr = ptr->next) count++;
    return count;
}

/* move all elements from src to the tail of dst */
static inline void list_move_tail(struct list* dst, struct list* src)
{
    if (list_empty(src)) return;

    dst->prev->next = src->next;
    src->next->prev = dst->prev;
    dst->prev = src->prev;
    src->prev->next = dst;
    list_init(src);
}

/* move all elements from src to the head of dst */
static inline void list_move_head(struct list* dst, struct list* src)
{
    if (list_empty(src)) return;

    dst->next->prev = src->prev;
    src->prev->next = dst->next;
    dst->next = src->next;
    src->next->prev = dst;
    list_init(src);
}

/* iterate through the list */
#define LIST_FOR_EACH(cursor,list) \
    for ((cursor) = (list)->next; (cursor) != (list); (cursor) = (cursor)->next)

/* iterate through the list, with safety against removal */
#define LIST_FOR_EACH_SAFE(cursor, cursor2, list) \
    for ((cursor) = (list)->next, (cursor2) = (cursor)->next; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->next)

/* iterate through the list using a list entry */
#define LIST_FOR_EACH_ENTRY(elem, list, type, field) \
    for ((elem) = LIST_ENTRY((list)->next, type, field); \
         &(elem)->field != (list); \
         (elem) = LIST_ENTRY((elem)->field.next, type, field))

/* iterate through the list using a list entry, with safety against removal */
#define LIST_FOR_EACH_ENTRY_SAFE(cursor, cursor2, list, type, field) \
    for ((cursor) = LIST_ENTRY((list)->next, type, field), \
         (cursor2) = LIST_ENTRY((cursor)->field.next, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = LIST_ENTRY((cursor)->field.next, type, field))

/* iterate through the list in reverse order */
#define LIST_FOR_EACH_REV(cursor,list) \
    for ((cursor) = (list)->prev; (cursor) != (list); (cursor) = (cursor)->prev)

/* iterate through the list in reverse order, with safety against removal */
#define LIST_FOR_EACH_SAFE_REV(cursor, cursor2, list) \
    for ((cursor) = (list)->prev, (cursor2) = (cursor)->prev; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->prev)

/* iterate through the list in reverse order using a list entry */
#define LIST_FOR_EACH_ENTRY_REV(elem, list, type, field) \
    for ((elem) = LIST_ENTRY((list)->prev, type, field); \
         &(elem)->field != (list); \
         (elem) = LIST_ENTRY((elem)->field.prev, type, field))

/* iterate through the list in reverse order using a list entry, with safety against removal */
#define LIST_FOR_EACH_ENTRY_SAFE_REV(cursor, cursor2, list, type, field) \
    for ((cursor) = LIST_ENTRY((list)->prev, type, field), \
         (cursor2) = LIST_ENTRY((cursor)->field.prev, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = LIST_ENTRY((cursor)->field.prev, type, field))

/* macros for statically initialized lists */
#undef LIST_INIT
#define LIST_INIT(list)  { &(list), &(list) }

/* get pointer to object containing list element */
#undef LIST_ENTRY
#define LIST_ENTRY(elem, type, field) \
    ((type *)((char *)(elem) - offsetof(type, field)))

#endif  /* __WINE_SERVER_LIST_H */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Source: https://raw.githubusercontent.com/wine-mirror/wine/master/dlls/kernel32/kernel_private.h
// Modified
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static inline BOOL set_ntstatus(NTSTATUS status) {
    if (status) SetLastError(ERROR_INVALID_HANDLE); // ERROR_INVALID_HANDLE is just a dummy because RtlNtStatusToDosError(status) is not existing on Win9x
    return !status;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Source: https://github.com/wine-mirror/wine/blob/master/dlls/kernel32/resource.c ( Latest commit c5ff8ff on 17 Nov 2020 )
// Modified by Daniel Marschall (16 Nov 2021, "Standalone" code, but only Begin/End/UpdateResources in ASCII. Also some small fixes)
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * Resources
 *
 * Copyright 1993 Robert J. Amstadt
 * Copyright 1995, 2003 Alexandre Julliard
 * Copyright 2006 Mike McCormack
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

 /* retrieve the resource name to pass to the ntdll functions */
static NTSTATUS get_res_nameA(LPCSTR name, UNICODE_STRING* str)
{
    if (IS_INTRESOURCE(name))
    {
        str->Buffer = (PWSTR)ULongToPtr(LOWORD(name));
        return STATUS_SUCCESS;
    }
    else if (name[0] == '#')
    {
        ULONG value;
        if (_RtlCharToInteger(name + 1, 10, &value) != STATUS_SUCCESS || HIWORD(value))
            return STATUS_INVALID_PARAMETER;
        str->Buffer = (PWSTR)ULongToPtr(value);
        return STATUS_SUCCESS;
    }
    else
    {
        _RtlCreateUnicodeStringFromAsciiz(str, name);

        // Fix by Daniel Marschall: Added RtlUpcaseUnicodeString for get_res_nameA, like it was done for get_res_nameW
        // RtlUpcaseUnicodeString(str, str, FALSE);
        // For our implementation, this simple inplace function works:
        _InplaceRtlUpcaseUnicodeString(str);
        
        return STATUS_SUCCESS;
    }
}

/*
HRSRC WINAPI WineFindResourceExA(HMODULE module, LPCSTR type, LPCSTR name, WORD lang)
{
    NTSTATUS status;
    UNICODE_STRING nameW, typeW;
    HRSRC ret = NULL;

    //TRACE("%p %s %s %04x\n", module, debugstr_a(type), debugstr_a(name), lang);

    if (!module) module = GetModuleHandleW(0);
    nameW.Buffer = NULL;
    typeW.Buffer = NULL;

    //__TRY
    //{
    if (!(status = get_res_nameA(name, &nameW)) && !(status = get_res_nameA(type, &typeW)))
        ret = WineFindResourceExW(module, typeW.Buffer, nameW.Buffer, lang);
    else
        SetLastError(1); // RtlNtStatusToDosError(status)
    //}
    //__EXCEPT_PAGE_FAULT
    //{
    //    SetLastError(ERROR_INVALID_PARAMETER);
    //}
    //__ENDTRY

    if (!IS_INTRESOURCE(nameW.Buffer)) HeapFree(GetProcessHeap(), 0, nameW.Buffer);
    if (!IS_INTRESOURCE(typeW.Buffer)) HeapFree(GetProcessHeap(), 0, typeW.Buffer);
    return ret;
}

HRSRC WINAPI WineFindResourceA(HMODULE hModule, LPCSTR name, LPCSTR type)
{
    return WineFindResourceExA(hModule, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
}

BOOL WINAPI WineEnumResourceTypesA(HMODULE hmod, ENUMRESTYPEPROCA lpfun, LONG_PTR lparam)
{
    return WineEnumResourceTypesExA(hmod, lpfun, lparam, 0, 0);
}

BOOL WINAPI WineEnumResourceTypesW(HMODULE hmod, ENUMRESTYPEPROCW lpfun, LONG_PTR lparam)
{
    return WineEnumResourceTypesExW(hmod, lpfun, lparam, 0, 0);
}

BOOL WINAPI WineEnumResourceNamesA(HMODULE hmod, LPCSTR type, ENUMRESNAMEPROCA lpfun, LONG_PTR lparam)
{
    return WineEnumResourceNamesExA(hmod, type, lpfun, lparam, 0, 0);
}

BOOL WINAPI WineEnumResourceLanguagesA(HMODULE hmod, LPCSTR type, LPCSTR name,
    ENUMRESLANGPROCA lpfun, LONG_PTR lparam)
{
    return WineEnumResourceLanguagesExA(hmod, type, name, lpfun, lparam, 0, 0);
}

BOOL WINAPI WineEnumResourceLanguagesW(HMODULE hmod, LPCWSTR type, LPCWSTR name,
    ENUMRESLANGPROCW lpfun, LONG_PTR lparam)
{
    return WineEnumResourceLanguagesExW(hmod, type, name, lpfun, lparam, 0, 0);
}
*/

/*
 *  Data structure for updating resources.
 *  Type/Name/Language is a keyset for accessing resource data.
 *
 *  QUEUEDUPDATES (root) ->
 *    list of struct resource_dir_entry    (Type) ->
 *      list of struct resource_dir_entry  (Name)   ->
 *         list of struct resource_data    Language + Data
 */

typedef struct
{
    void* unknown[6];
    LPSTR pFileName;
    BOOL bDeleteExistingResources;
    struct list root;
} QUEUEDUPDATES;

/* this structure is shared for types and names */
struct resource_dir_entry {
    struct list entry;
    LPWSTR id;
    struct list children;
};

/* this structure is the leaf */
struct resource_data {
    struct list entry;
    LANGID lang;
    DWORD codepage;
    DWORD cbData;
    void* lpData;
};

static int resource_strcmp(LPCWSTR a, LPCWSTR b)
{
    if (a == b)
        return 0;
    if (!IS_INTRESOURCE(a) && !IS_INTRESOURCE(b))
        return wcscmp(a, b);
    /* strings come before ids */
    if (!IS_INTRESOURCE(a) && IS_INTRESOURCE(b))
        return -1;
    if (!IS_INTRESOURCE(b) && IS_INTRESOURCE(a))
        return 1;
    return (a < b) ? -1 : 1;
}

static struct resource_dir_entry* find_resource_dir_entry(struct list* dir, LPCWSTR id)
{
    struct resource_dir_entry* ent;

    /* match either IDs or strings */
    LIST_FOR_EACH_ENTRY(ent, dir, struct resource_dir_entry, entry)
        if (!resource_strcmp(id, ent->id))
            return ent;

    return NULL;
}

static struct resource_data* find_resource_data(struct list* dir, LANGID lang)
{
    struct resource_data* res_data;

    /* match only languages here */
    LIST_FOR_EACH_ENTRY(res_data, dir, struct resource_data, entry)
        if (lang == res_data->lang)
            return res_data;

    return NULL;
}

static void add_resource_dir_entry(struct list* dir, struct resource_dir_entry* resdir)
{
    struct resource_dir_entry* ent;

    LIST_FOR_EACH_ENTRY(ent, dir, struct resource_dir_entry, entry)
    {
        if (0 > resource_strcmp(ent->id, resdir->id))
            continue;

        list_add_before(&ent->entry, &resdir->entry);
        return;
    }
    list_add_tail(dir, &resdir->entry);
}

static void add_resource_data_entry(struct list* dir, struct resource_data* resdata)
{
    struct resource_data* ent;

    LIST_FOR_EACH_ENTRY(ent, dir, struct resource_data, entry)
    {
        if (ent->lang < resdata->lang)
            continue;

        list_add_before(&ent->entry, &resdata->entry);
        return;
    }
    list_add_tail(dir, &resdata->entry);
}

static LPWSTR res_strdupW(LPCWSTR str)
{
    LPWSTR ret;
    UINT len;

    if (IS_INTRESOURCE(str))
        return (LPWSTR)(UINT_PTR)LOWORD(str);
    len = (lstrlenW(str) + 1) * sizeof(WCHAR);
    ret = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, len);
    if (!ret) return NULL; // Added by Daniel Marschall
    memcpy(ret, str, len);
    return ret;
}

static void res_free_str(LPWSTR str)
{
    if (!IS_INTRESOURCE(str))
        HeapFree(GetProcessHeap(), 0, str);
}

static BOOL update_add_resource(QUEUEDUPDATES* updates, LPCWSTR Type, LPCWSTR Name,
    LANGID Lang, struct resource_data* resdata,
    BOOL overwrite_existing)
{
    struct resource_dir_entry* restype, * resname;
    struct resource_data* existing;

    //TRACE("%p %s %s %p %d\n", updates,
    //      debugstr_w(Type), debugstr_w(Name), resdata, overwrite_existing );

    restype = find_resource_dir_entry(&updates->root, Type);
    if (!restype)
    {
        restype = (struct resource_dir_entry*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct resource_dir_entry));
        if (!restype) return FALSE;
        restype->id = res_strdupW(Type);
        list_init(&restype->children);
        add_resource_dir_entry(&updates->root, restype);
    }

    resname = find_resource_dir_entry(&restype->children, Name);
    if (!resname)
    {
        resname = (struct resource_dir_entry*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct resource_dir_entry));
        if (!resname) return FALSE;
        resname->id = res_strdupW(Name);
        list_init(&resname->children);
        add_resource_dir_entry(&restype->children, resname);
    }

    /*
     * If there's an existing resource entry with matching (Type,Name,Language)
     *  it needs to be removed before adding the new data.
     */
    existing = find_resource_data(&resname->children, Lang);
    if (existing)
    {
        if (!overwrite_existing)
            return FALSE;
        list_remove(&existing->entry);
        HeapFree(GetProcessHeap(), 0, existing);
    }

    if (resdata)
        add_resource_data_entry(&resname->children, resdata);

    return TRUE;
}

static struct resource_data* allocate_resource_data(WORD Language, DWORD codepage,
    LPVOID lpData, DWORD cbData, BOOL copy_data)
{
    struct resource_data* resdata;

    if (!lpData || !cbData)
        return NULL;

    resdata = (struct resource_data*)HeapAlloc(GetProcessHeap(), 0, sizeof * resdata + (copy_data ? cbData : 0));
    if (resdata)
    {
        resdata->lang = Language;
        resdata->codepage = codepage;
        resdata->cbData = cbData;
        if (copy_data)
        {
            resdata->lpData = &resdata[1];
            memcpy(resdata->lpData, lpData, cbData);
        }
        else
            resdata->lpData = lpData;
    }

    return resdata;
}

static void free_resource_directory(struct list* head, int level)
{
    struct list* ptr = NULL;

    while ((ptr = list_head(head)))
    {
        list_remove(ptr);
        if (level)
        {
            struct resource_dir_entry* ent;

            ent = LIST_ENTRY(ptr, struct resource_dir_entry, entry);
            res_free_str(ent->id);
            free_resource_directory(&ent->children, level - 1);
            HeapFree(GetProcessHeap(), 0, ent);
        }
        else
        {
            struct resource_data* data;

            data = LIST_ENTRY(ptr, struct resource_data, entry);
            HeapFree(GetProcessHeap(), 0, data);
        }
    }
}

static IMAGE_NT_HEADERS* get_nt_header(void* base, DWORD mapping_size)
{
    IMAGE_NT_HEADERS* nt;
    IMAGE_DOS_HEADER* dos;

    if (mapping_size < sizeof(*dos))
        return NULL;

    dos = (IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return NULL;

    if ((dos->e_lfanew + sizeof(*nt)) > mapping_size)
        return NULL;

    nt = (IMAGE_NT_HEADERS*)((BYTE*)base + dos->e_lfanew);

    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return NULL;

    return nt;
}

static IMAGE_SECTION_HEADER* get_section_header(void* base, DWORD mapping_size, DWORD* num_sections)
{
    IMAGE_NT_HEADERS* nt;
    DWORD section_ofs;

    nt = get_nt_header(base, mapping_size);
    if (!nt)
        return NULL;

    /* check that we don't go over the end of the file accessing the sections */
    section_ofs = FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + nt->FileHeader.SizeOfOptionalHeader;
    if ((nt->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER) + section_ofs) > mapping_size)
        return NULL;

    if (num_sections)
        *num_sections = nt->FileHeader.NumberOfSections;

    /* from here we have a valid PE exe to update */
    return (IMAGE_SECTION_HEADER*)((BYTE*)nt + section_ofs);
}

static BOOL check_pe_exe(HANDLE file, QUEUEDUPDATES* updates)
{
    const IMAGE_NT_HEADERS32* nt;
    //const IMAGE_NT_HEADERS64* nt64;
    const IMAGE_SECTION_HEADER* sec;
    //const IMAGE_DATA_DIRECTORY* dd;
    BOOL ret = FALSE;
    HANDLE mapping;
    DWORD mapping_size, num_sections = 0;
    void* base = NULL;

    mapping_size = GetFileSize(file, NULL);

    mapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mapping)
        goto done;

    base = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, mapping_size);
    if (!base)
        goto done;

    nt = (IMAGE_NT_HEADERS32*)get_nt_header(base, mapping_size);
    if (!nt)
        goto done;

    //Fix by Daniel Marschall: Removed, because the variables are not used!
    //nt64 = (IMAGE_NT_HEADERS64*)nt;
    //dd = &nt->OptionalHeader.DataDirectory[0];
    //if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    //    dd = &nt64->OptionalHeader.DataDirectory[0];
    //TRACE("resources: %08x %08x\n",
    //      dd[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress,
    //      dd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size);

    sec = get_section_header(base, mapping_size, &num_sections);
    if (!sec)
        goto done;

    ret = TRUE;

done:
    if (base)
        UnmapViewOfFile(base);
    if (mapping)
        CloseHandle(mapping);

    return ret;
}

struct resource_size_info {
    DWORD types_ofs;
    DWORD names_ofs;
    DWORD langs_ofs;
    DWORD data_entry_ofs;
    DWORD strings_ofs;
    DWORD data_ofs;
    DWORD total_size;
};

struct mapping_info {
    HANDLE file;
    void* base;
    DWORD size;
    BOOL read_write;
};

static const IMAGE_SECTION_HEADER* section_from_rva(void* base, DWORD mapping_size, DWORD rva)
{
    const IMAGE_SECTION_HEADER* sec;
    DWORD num_sections = 0;
    int i;

    sec = get_section_header(base, mapping_size, &num_sections);
    if (!sec)
        return NULL;

    for (i = num_sections - 1; i >= 0; i--)
    {
        if (sec[i].VirtualAddress <= rva &&
            rva <= (DWORD)sec[i].VirtualAddress + sec[i].SizeOfRawData)
        {
            return &sec[i];
        }
    }

    return NULL;
}

static void* address_from_rva(void* base, DWORD mapping_size, DWORD rva, DWORD len)
{
    const IMAGE_SECTION_HEADER* sec;

    sec = section_from_rva(base, mapping_size, rva);
    if (!sec)
        return NULL;

    if (rva + len <= (DWORD)sec->VirtualAddress + sec->SizeOfRawData)
        return (void*)((LPBYTE)base + (sec->PointerToRawData + rva - sec->VirtualAddress));

    return NULL;
}

static LPWSTR resource_dup_string(const IMAGE_RESOURCE_DIRECTORY* root, const MyIMAGE_RESOURCE_DIRECTORY_ENTRY* entry)
{
    const IMAGE_RESOURCE_DIR_STRING_U* string;
    LPWSTR s;

    if (!entry->u.s.NameIsString)
        return (LPWSTR)UIntToPtr(entry->u.Id);

    string = (const IMAGE_RESOURCE_DIR_STRING_U*)(((const char*)root) + entry->u.s.NameOffset);
    s = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, (string->Length + 1) * sizeof(WCHAR));
    if (!s) return NULL;
    memcpy(s, string->NameString, (string->Length + 1) * sizeof(WCHAR));
    s[string->Length] = 0;

    return s;
}

/* this function is based on the code in winedump's pe.c */
static BOOL enumerate_mapped_resources(QUEUEDUPDATES* updates,
    void* base, DWORD mapping_size,
    const IMAGE_RESOURCE_DIRECTORY* root)
{
    const IMAGE_RESOURCE_DIRECTORY* namedir, * langdir;
    const MyIMAGE_RESOURCE_DIRECTORY_ENTRY* e1, * e2, * e3;
    const IMAGE_RESOURCE_DATA_ENTRY* data;
    DWORD i, j, k;

    //TRACE("version (%d.%d) %d named %d id entries\n",
    //      root->MajorVersion, root->MinorVersion, root->NumberOfNamedEntries, root->NumberOfIdEntries);

    for (i = 0; i < (DWORD)root->NumberOfNamedEntries + (DWORD)root->NumberOfIdEntries; i++)
    {
        LPWSTR Type;

        e1 = (const MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)(root + 1) + i;

        Type = resource_dup_string(root, e1);

        namedir = (const IMAGE_RESOURCE_DIRECTORY*)((const char*)root + e1->u2.s2.OffsetToDirectory);
        for (j = 0; j < (DWORD)namedir->NumberOfNamedEntries + (DWORD)namedir->NumberOfIdEntries; j++)
        {
            LPWSTR Name;

            e2 = (const MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)(namedir + 1) + j;

            Name = resource_dup_string(root, e2);

            langdir = (const IMAGE_RESOURCE_DIRECTORY*)((const char*)root + e2->u2.s2.OffsetToDirectory);
            for (k = 0; k < (DWORD)langdir->NumberOfNamedEntries + (DWORD)langdir->NumberOfIdEntries; k++)
            {
                LANGID Lang;
                void* p;
                struct resource_data* resdata;

                e3 = (const MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)(langdir + 1) + k;

                Lang = e3->u.Id;

                data = (const IMAGE_RESOURCE_DATA_ENTRY*)((const char*)root + e3->u2.OffsetToData);

                p = address_from_rva(base, mapping_size, data->OffsetToData, data->Size);

                resdata = allocate_resource_data(Lang, data->CodePage, p, data->Size, FALSE);
                if (resdata)
                {
                    if (!update_add_resource(updates, Type, Name, Lang, resdata, FALSE))
                        HeapFree(GetProcessHeap(), 0, resdata);
                }
            }
            res_free_str(Name);
        }
        res_free_str(Type);
    }

    return TRUE;
}

static BOOL read_mapped_resources(QUEUEDUPDATES* updates, void* base, DWORD mapping_size)
{
    const IMAGE_RESOURCE_DIRECTORY* root;
    const IMAGE_NT_HEADERS* nt;
    const IMAGE_SECTION_HEADER* sec;
    DWORD num_sections = 0, i;

    nt = get_nt_header(base, mapping_size);
    if (!nt)
        return FALSE;

    sec = get_section_header(base, mapping_size, &num_sections);
    if (!sec)
        return FALSE;

    for (i = 0; i < num_sections; i++)
        if (!memcmp(sec[i].Name, ".rsrc", 6))
            break;

    if (i == num_sections)
        return TRUE;

    /* check the resource data is inside the mapping */
    if (sec[i].PointerToRawData > mapping_size ||
        (sec[i].PointerToRawData + sec[i].SizeOfRawData) > mapping_size)
        return TRUE;

    //TRACE("found .rsrc at %08x, size %08x\n", sec[i].PointerToRawData, sec[i].SizeOfRawData);

    if (!sec[i].PointerToRawData || sec[i].SizeOfRawData < sizeof(IMAGE_RESOURCE_DIRECTORY))
        return TRUE;

    root = (IMAGE_RESOURCE_DIRECTORY*)((BYTE*)base + sec[i].PointerToRawData);
    enumerate_mapped_resources(updates, base, mapping_size, root);

    return TRUE;
}

static BOOL map_file_into_memory(struct mapping_info* mi)
{
    DWORD page_attr, perm;
    HANDLE mapping;

    if (mi->read_write)
    {
        page_attr = PAGE_READWRITE;
        perm = FILE_MAP_WRITE | FILE_MAP_READ;
    }
    else
    {
        page_attr = PAGE_READONLY;
        perm = FILE_MAP_READ;
    }

    mapping = CreateFileMappingA(mi->file, NULL, page_attr, 0, 0, NULL);
    if (!mapping) return FALSE;

    mi->base = MapViewOfFile(mapping, perm, 0, 0, mi->size);
    CloseHandle(mapping);

    return mi->base != NULL;
}

static BOOL unmap_file_from_memory(struct mapping_info* mi)
{
    if (mi->base)
        UnmapViewOfFile(mi->base);
    mi->base = NULL;
    return TRUE;
}

static void destroy_mapping(struct mapping_info* mi)
{
    if (!mi)
        return;
    unmap_file_from_memory(mi);
    if (mi->file)
        CloseHandle(mi->file);
    HeapFree(GetProcessHeap(), 0, mi);
}

static struct mapping_info* create_mapping(LPCSTR filename, BOOL rw)
{
    struct mapping_info* mi;

    // TODO: Which Windows version supports HEAP_ZERO_MEMORY? Can we safely use it, or is memset() safer?
    mi = (struct mapping_info*)HeapAlloc(GetProcessHeap(), 0/*HEAP_ZERO_MEMORY*/, sizeof * mi);
    if (!mi) {
        return NULL;
    }
    memset(mi, 0, sizeof * mi);

    mi->read_write = rw;

    // Fix by Daniel Marschall: Changed "0" to "FILE_SHARE_READ | (rw ? FILE_SHARE_WRITE : 0)"
    mi->file = CreateFileA(filename, GENERIC_READ | (rw ? GENERIC_WRITE : 0),
        FILE_SHARE_READ | (rw ? FILE_SHARE_WRITE : 0), NULL, OPEN_EXISTING, 0, 0);

    if (mi->file != INVALID_HANDLE_VALUE)
    {
        mi->size = GetFileSize(mi->file, NULL);

        if (map_file_into_memory(mi))
            return mi;
    }
    destroy_mapping(mi);
    return NULL;
}

static BOOL resize_mapping(struct mapping_info* mi, DWORD new_size)
{
    if (!unmap_file_from_memory(mi))
        return FALSE;

    /* change the file size */
    SetFilePointer(mi->file, new_size, NULL, FILE_BEGIN);
    if (!SetEndOfFile(mi->file))
    {
        //ERR("failed to set file size to %08x\n", new_size);
        return FALSE;
    }

    mi->size = new_size;

    return map_file_into_memory(mi);
}

static void get_resource_sizes(QUEUEDUPDATES* updates, struct resource_size_info* si)
{
    struct resource_dir_entry* types, * names;
    struct resource_data* data;
    DWORD num_types = 0, num_names = 0, num_langs = 0, strings_size = 0, data_size = 0;

    memset(si, 0, sizeof * si);

    LIST_FOR_EACH_ENTRY(types, &updates->root, struct resource_dir_entry, entry)
    {
        num_types++;
        if (!IS_INTRESOURCE(types->id))
            strings_size += sizeof(WORD) + lstrlenW(types->id) * sizeof(WCHAR);

        LIST_FOR_EACH_ENTRY(names, &types->children, struct resource_dir_entry, entry)
        {
            num_names++;

            if (!IS_INTRESOURCE(names->id))
                strings_size += sizeof(WORD) + lstrlenW(names->id) * sizeof(WCHAR);

            LIST_FOR_EACH_ENTRY(data, &names->children, struct resource_data, entry)
            {
                num_langs++;
                data_size += (data->cbData + 3) & ~3;
            }
        }
    }

    /* names are at the end of the types */
    si->names_ofs = sizeof(IMAGE_RESOURCE_DIRECTORY) +
        num_types * sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

    /* language directories are at the end of the names */
    si->langs_ofs = si->names_ofs +
        num_types * sizeof(IMAGE_RESOURCE_DIRECTORY) +
        num_names * sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

    si->data_entry_ofs = si->langs_ofs +
        num_names * sizeof(IMAGE_RESOURCE_DIRECTORY) +
        num_langs * sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

    si->strings_ofs = si->data_entry_ofs +
        num_langs * sizeof(IMAGE_RESOURCE_DATA_ENTRY);

    si->data_ofs = si->strings_ofs + ((strings_size + 3) & ~3);

    si->total_size = si->data_ofs + data_size;

    //TRACE("names %08x langs %08x data entries %08x strings %08x data %08x total %08x\n",
    //      si->names_ofs, si->langs_ofs, si->data_entry_ofs,
    //      si->strings_ofs, si->data_ofs, si->total_size);
}

static void res_write_padding(BYTE* res_base, DWORD size)
{
    static const BYTE pad[] = {
        'P','A','D','D','I','N','G','X','X','P','A','D','D','I','N','G' };
    DWORD i;

    for (i = 0; i < size / sizeof pad; i++)
        memcpy(&res_base[i * sizeof pad], pad, sizeof pad);
    memcpy(&res_base[i * sizeof pad], pad, size % sizeof pad);
}

static BOOL write_resources(QUEUEDUPDATES* updates, LPBYTE base, struct resource_size_info* si, DWORD rva)
{
    struct resource_dir_entry* types, * names;
    struct resource_data* data;
    IMAGE_RESOURCE_DIRECTORY* root;

    //TRACE("%p %p %p %08x\n", updates, base, si, rva );

    memset(base, 0, si->total_size);

    /* the root entry always exists */
    root = (IMAGE_RESOURCE_DIRECTORY*)base;
    memset(root, 0, sizeof * root);
    root->MajorVersion = 4;
    si->types_ofs = sizeof * root;
    LIST_FOR_EACH_ENTRY(types, &updates->root, struct resource_dir_entry, entry)
    {
        MyIMAGE_RESOURCE_DIRECTORY_ENTRY* e1;
        IMAGE_RESOURCE_DIRECTORY* namedir;

        e1 = (MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)&base[si->types_ofs];
        memset(e1, 0, sizeof * e1);
        if (!IS_INTRESOURCE(types->id))
        {
            WCHAR* strings;
            DWORD len;

            root->NumberOfNamedEntries++;
            e1->u.s.NameIsString = 1;
            e1->u.s.NameOffset = si->strings_ofs;

            strings = (WCHAR*)&base[si->strings_ofs];
            len = lstrlenW(types->id);
            strings[0] = (WCHAR)len;
            memcpy(&strings[1], types->id, len * sizeof(WCHAR));
            si->strings_ofs += (len + 1) * sizeof(WCHAR);
        }
        else
        {
            root->NumberOfIdEntries++;
            e1->u.Id = LOWORD(types->id);
        }
        e1->u2.s2.OffsetToDirectory = si->names_ofs;
        e1->u2.s2.DataIsDirectory = TRUE;
        si->types_ofs += sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

        namedir = (IMAGE_RESOURCE_DIRECTORY*)&base[si->names_ofs];
        memset(namedir, 0, sizeof * namedir);
        namedir->MajorVersion = 4;
        si->names_ofs += sizeof(IMAGE_RESOURCE_DIRECTORY);

        LIST_FOR_EACH_ENTRY(names, &types->children, struct resource_dir_entry, entry)
        {
            MyIMAGE_RESOURCE_DIRECTORY_ENTRY* e2;
            IMAGE_RESOURCE_DIRECTORY* langdir;

            e2 = (MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)&base[si->names_ofs];
            memset(e2, 0, sizeof * e2);
            if (!IS_INTRESOURCE(names->id))
            {
                WCHAR* strings;
                DWORD len;

                namedir->NumberOfNamedEntries++;
                e2->u.s.NameIsString = 1;
                e2->u.s.NameOffset = si->strings_ofs;

                strings = (WCHAR*)&base[si->strings_ofs];
                len = lstrlenW(names->id);
                strings[0] = (WCHAR)len;
                memcpy(&strings[1], names->id, len * sizeof(WCHAR));
                si->strings_ofs += (len + 1) * sizeof(WCHAR);
            }
            else
            {
                namedir->NumberOfIdEntries++;
                e2->u.Id = LOWORD(names->id);
            }
            e2->u2.s2.OffsetToDirectory = si->langs_ofs;
            e2->u2.s2.DataIsDirectory = TRUE;
            si->names_ofs += sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

            langdir = (IMAGE_RESOURCE_DIRECTORY*)&base[si->langs_ofs];
            memset(langdir, 0, sizeof * langdir);
            langdir->MajorVersion = 4;
            si->langs_ofs += sizeof(IMAGE_RESOURCE_DIRECTORY);

            LIST_FOR_EACH_ENTRY(data, &names->children, struct resource_data, entry)
            {
                MyIMAGE_RESOURCE_DIRECTORY_ENTRY* e3;
                IMAGE_RESOURCE_DATA_ENTRY* de;
                int pad_size;

                e3 = (MyIMAGE_RESOURCE_DIRECTORY_ENTRY*)&base[si->langs_ofs];
                memset(e3, 0, sizeof * e3);
                langdir->NumberOfIdEntries++;
                e3->u.Id = LOWORD(data->lang);
                e3->u2.OffsetToData = si->data_entry_ofs;

                si->langs_ofs += sizeof(MyIMAGE_RESOURCE_DIRECTORY_ENTRY);

                /* write out all the data entries */
                de = (IMAGE_RESOURCE_DATA_ENTRY*)&base[si->data_entry_ofs];
                memset(de, 0, sizeof * de);
                de->OffsetToData = si->data_ofs + rva;
                de->Size = data->cbData;
                de->CodePage = data->codepage;
                si->data_entry_ofs += sizeof(IMAGE_RESOURCE_DATA_ENTRY);

                /* write out the resource data */
                memcpy(&base[si->data_ofs], data->lpData, data->cbData);
                si->data_ofs += data->cbData;

                pad_size = (-(int)si->data_ofs) & 3;
                res_write_padding(&base[si->data_ofs], pad_size);
                si->data_ofs += pad_size;
            }
        }
    }

    return TRUE;
}

/*
 *  FIXME:
 *  Assumes that the resources are in .rsrc
 *   and .rsrc is the last section in the file.
 *  Not sure whether updating resources will other cases on Windows.
 *  If the resources lie in a section containing other data,
 *   resizing that section could possibly cause trouble.
 *  If the section with the resources isn't last, the remaining
 *   sections need to be moved down in the file, and the section header
 *   would need to be adjusted.
 *  If we needed to add a section, what would we name it?
 *  If we needed to add a section and there wasn't space in the file
 *   header, how would that work?
 *  Seems that at least some of these cases can't be handled properly.
 */
static IMAGE_SECTION_HEADER* get_resource_section(void* base, DWORD mapping_size)
{
    IMAGE_SECTION_HEADER* sec;
    IMAGE_NT_HEADERS* nt;
    DWORD i, num_sections = 0;

    nt = get_nt_header(base, mapping_size);
    if (!nt)
        return NULL;

    sec = get_section_header(base, mapping_size, &num_sections);
    if (!sec)
        return NULL;

    /* find the resources section */
    for (i = 0; i < num_sections; i++)
        if (!memcmp(sec[i].Name, ".rsrc", 6))
            break;

    if (i == num_sections)
        return NULL;

    return &sec[i];
}

static IMAGE_SECTION_HEADER* get_last_section(void* base, DWORD mapping_size)
{
    // Fix by Fix by Daniel Marschall: Added this function which is required by the "SizeOfImage" field calculation

    IMAGE_SECTION_HEADER* sec;
    IMAGE_NT_HEADERS* nt;
    DWORD num_sections = 0;

    nt = get_nt_header(base, mapping_size);
    if (!nt)
        return NULL;

    sec = get_section_header(base, mapping_size, &num_sections);
    if (!sec)
        return NULL;

    /* find the resources section */
    return &sec[num_sections - 1];
}

static DWORD get_init_data_size(void* base, DWORD mapping_size)
{
    DWORD i, sz = 0, num_sections = 0;
    IMAGE_SECTION_HEADER* s;

    s = get_section_header(base, mapping_size, &num_sections);

    for (i = 0; i < num_sections; i++)
        if (s[i].Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
            sz += s[i].SizeOfRawData;

    //TRACE("size = %08x\n", sz);

    return sz;
}

//
// peRoundUpToAlignment() - rounds dwValue up to nearest dwAlign
//
DWORD peRoundUpToAlignment(DWORD dwAlign, DWORD dwVal)
{
    // Fix by Fix by Daniel Marschall: Added this function, based on
    // https://stackoverflow.com/questions/39022853/how-is-sizeofimage-in-the-pe-optional-header-computed
    if (dwAlign)
    {
        //do the rounding with bitwise operations...

        //create bit mask of bits to keep
        //  e.g. if section alignment is 0x1000                        1000000000000
        //       we want the following bitmask      11111111111111111111000000000000
        DWORD dwMask = ~(dwAlign - 1);

        //round up by adding full alignment (dwAlign-1 since if already aligned we don't want anything to change),
        //  then mask off any lower bits
        dwVal = (dwVal + dwAlign - 1) & dwMask;
    }

    return(dwVal);

} //peRoundUpToAlignment()

static BOOL write_raw_resources(QUEUEDUPDATES* updates)
{
    CHAR tempdir[MAX_PATH], tempfile[MAX_PATH];
    DWORD i, section_size;
    BOOL ret = FALSE;
    IMAGE_SECTION_HEADER* sec, *lastsec;
    IMAGE_NT_HEADERS32* nt;
    IMAGE_NT_HEADERS64* nt64;
    struct resource_size_info res_size;
    BYTE* res_base;
    struct mapping_info* read_map = NULL, * write_map = NULL;
    DWORD PeSectionAlignment, PeFileAlignment, PeSizeOfImage;

    /* copy the exe to a temp file then update the temp file... */
    tempdir[0] = 0;
    if (!GetTempPathA(MAX_PATH, tempdir))
        return ret;

    if (!GetTempFileNameA(tempdir, "resu", 0, tempfile))
        return ret;

    if (!CopyFileA(updates->pFileName, tempfile, FALSE))
        goto done;

    //TRACE("tempfile %s\n", debugstr_w(tempfile));

    if (!updates->bDeleteExistingResources)
    {
        read_map = create_mapping(updates->pFileName, FALSE);
        if (!read_map)
            goto done;

        ret = read_mapped_resources(updates, read_map->base, read_map->size);
        if (!ret)
        {
            //ERR("failed to read existing resources\n");
            goto done;
        }
    }

    write_map = create_mapping(tempfile, TRUE);
    if (!write_map)
        goto done;

    nt = (IMAGE_NT_HEADERS32*)get_nt_header(write_map->base, write_map->size);
    if (!nt)
        goto done;

    nt64 = (IMAGE_NT_HEADERS64*)nt;
    if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        PeSectionAlignment = nt64->OptionalHeader.SectionAlignment;
        PeFileAlignment = nt64->OptionalHeader.FileAlignment;
        PeSizeOfImage = nt64->OptionalHeader.SizeOfImage;
    }
    else {
        PeSectionAlignment = nt->OptionalHeader.SectionAlignment;
        PeFileAlignment = nt->OptionalHeader.FileAlignment;
        PeSizeOfImage = nt->OptionalHeader.SizeOfImage;
    }

    if ((LONG)PeSectionAlignment <= 0)
    {
        //ERR("invalid section alignment %08x\n", PeSectionAlignment);
        goto done;
    }

    if ((LONG)PeFileAlignment <= 0)
    {
        //ERR("invalid file alignment %08x\n", PeFileAlignment);
        goto done;
    }

    sec = get_resource_section(write_map->base, write_map->size);
    if (!sec) /* no section, add one */
    {
        DWORD num_sections;

        sec = get_section_header(write_map->base, write_map->size, &num_sections);
        if (!sec)
            goto done;

        sec += num_sections;
        nt->FileHeader.NumberOfSections++;

        memset(sec, 0, sizeof * sec);
        memcpy(sec->Name, ".rsrc", 5);
        sec->Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;
        sec->VirtualAddress = PeSizeOfImage;
    }

    if (!sec->PointerToRawData)  /* empty section */
    {
        sec->PointerToRawData = write_map->size + (-(int)write_map->size) % PeFileAlignment;
        sec->SizeOfRawData = 0;
    }

    //TRACE("before .rsrc at %08x, size %08x\n", sec->PointerToRawData, sec->SizeOfRawData);

    get_resource_sizes(updates, &res_size);

    /* round up the section size */
    section_size = res_size.total_size;
    section_size += (-(int)section_size) % PeFileAlignment;

    //TRACE("requires %08x (%08x) bytes\n", res_size.total_size, section_size );

    /* check if the file size needs to be changed */
    if (section_size != sec->SizeOfRawData)
    {
        DWORD old_size = write_map->size;
        DWORD virtual_section_size = res_size.total_size + (-(int)res_size.total_size) % PeSectionAlignment;
        int delta = section_size - (sec->SizeOfRawData + (-(int)sec->SizeOfRawData) % PeFileAlignment);
        int rva_delta = virtual_section_size -
            (sec->Misc.VirtualSize + (-(int)sec->Misc.VirtualSize) % PeSectionAlignment);
        /* when new section is added it could end past current mapping size */
        BOOL rsrc_is_last = sec->PointerToRawData + sec->SizeOfRawData >= old_size;
        /* align .rsrc size when possible */
        DWORD mapping_size = rsrc_is_last ? sec->PointerToRawData + section_size : old_size + delta;

        /* postpone file truncation if there are some data to be moved down from file end */
        BOOL resize_after = mapping_size < old_size && !rsrc_is_last;

        //TRACE("file size %08x -> %08x\n", old_size, mapping_size);

        if (!resize_after)
        {
            /* unmap the file before changing the file size */
            ret = resize_mapping(write_map, mapping_size);

            /* get the pointers again - they might be different after remapping */
            nt = (IMAGE_NT_HEADERS32*)get_nt_header(write_map->base, mapping_size);
            if (!nt)
            {
                //ERR("couldn't get NT header\n");
                goto done;
            }
            nt64 = (IMAGE_NT_HEADERS64*)nt;

            sec = get_resource_section(write_map->base, mapping_size);
            if (!sec)
                goto done;
        }

        if (!rsrc_is_last) /* not last section, relocate trailing sections */
        {
            IMAGE_SECTION_HEADER* s;
            DWORD tail_start = sec->PointerToRawData + sec->SizeOfRawData;
            DWORD i, num_sections = 0;

            memmove((char*)write_map->base + tail_start + delta, (char*)write_map->base + tail_start, old_size - tail_start);

            s = get_section_header(write_map->base, mapping_size, &num_sections);

            for (i = 0; i < num_sections; i++)
            {
                if (s[i].PointerToRawData > sec->PointerToRawData)
                {
                    s[i].PointerToRawData += delta;
                    s[i].VirtualAddress += rva_delta;
                }
            }
        }

        if (resize_after)
        {
            ret = resize_mapping(write_map, mapping_size);

            nt = (IMAGE_NT_HEADERS32*)get_nt_header(write_map->base, mapping_size);
            if (!nt)
            {
                //ERR("couldn't get NT header\n");
                goto done;
            }
            nt64 = (IMAGE_NT_HEADERS64*)nt;

            sec = get_resource_section(write_map->base, mapping_size);
            if (!sec)
                goto done;
        }

        /* adjust the PE header information */
        sec->SizeOfRawData = section_size;
        sec->Misc.VirtualSize = virtual_section_size;
        if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            DWORD pEndOfLastSection, pEndOfLastSectionMem, uCalcSizeOfFile;

            nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = sec->VirtualAddress;
            nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = res_size.total_size;
            nt64->OptionalHeader.SizeOfInitializedData = get_init_data_size(write_map->base, mapping_size);

            for (i = 0; i < nt64->OptionalHeader.NumberOfRvaAndSizes; i++)
                if (nt64->OptionalHeader.DataDirectory[i].VirtualAddress > sec->VirtualAddress)
                    nt64->OptionalHeader.DataDirectory[i].VirtualAddress += rva_delta;

            //nt64->OptionalHeader.SizeOfImage += rva_delta;
            // Fix by Daniel Marschall: Added this calculation of "SizeOfImage".
            // With the original implementation, Windows won't load some images!
            // https://stackoverflow.com/questions/39022853/how-is-sizeofimage-in-the-pe-optional-header-computed
            lastsec = get_last_section(write_map->base, mapping_size);
            pEndOfLastSection = lastsec->VirtualAddress + lastsec->Misc.VirtualSize + nt64->OptionalHeader.ImageBase;
            //NOTE: we are rounding to memory section alignment, not file
            pEndOfLastSectionMem = peRoundUpToAlignment(nt64->OptionalHeader.SectionAlignment, pEndOfLastSection);
            uCalcSizeOfFile = pEndOfLastSectionMem - nt64->OptionalHeader.ImageBase;
            nt64->OptionalHeader.SizeOfImage = uCalcSizeOfFile;
        }
        else {
            DWORD pEndOfLastSection, pEndOfLastSectionMem, uCalcSizeOfFile;

            nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = sec->VirtualAddress;
            nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = res_size.total_size;
            nt->OptionalHeader.SizeOfInitializedData = get_init_data_size(write_map->base, mapping_size);

            for (i = 0; i < nt->OptionalHeader.NumberOfRvaAndSizes; i++)
                if (nt->OptionalHeader.DataDirectory[i].VirtualAddress > sec->VirtualAddress)
                    nt->OptionalHeader.DataDirectory[i].VirtualAddress += rva_delta;

            //nt->OptionalHeader.SizeOfImage += rva_delta;
            // Fix by Daniel Marschall: Added this calculation of "SizeOfImage".
            // With the original implementation, Windows won't load some images!
            // https://stackoverflow.com/questions/39022853/how-is-sizeofimage-in-the-pe-optional-header-computed
            lastsec = get_last_section(write_map->base, mapping_size);
            pEndOfLastSection = lastsec->VirtualAddress + lastsec->Misc.VirtualSize + nt->OptionalHeader.ImageBase;
            //NOTE: we are rounding to memory section alignment, not file
            pEndOfLastSectionMem = peRoundUpToAlignment(nt->OptionalHeader.SectionAlignment, pEndOfLastSection);
            uCalcSizeOfFile = pEndOfLastSectionMem - nt->OptionalHeader.ImageBase;
            nt->OptionalHeader.SizeOfImage = uCalcSizeOfFile;
        }
    }

    res_base = (LPBYTE)write_map->base + sec->PointerToRawData;

    //TRACE("base = %p offset = %08x\n", write_map->base, sec->PointerToRawData);

    ret = write_resources(updates, res_base, &res_size, sec->VirtualAddress);

    res_write_padding(res_base + res_size.total_size, section_size - res_size.total_size);

    //TRACE("after  .rsrc at %08x, size %08x\n", sec->PointerToRawData, sec->SizeOfRawData);

done:
    destroy_mapping(read_map);
    destroy_mapping(write_map);

    if (ret)
        ret = CopyFileA(tempfile, updates->pFileName, FALSE);

    DeleteFileA(tempfile);

    return ret;
}

// ------------------------------------------------------------------------------------------------------------------------

HANDLE WINAPI WineBeginUpdateResourceA(LPCSTR pFileName, BOOL bDeleteExistingResources)
{
    QUEUEDUPDATES* updates = NULL;
    HANDLE hUpdate, file, ret = NULL;

    //TRACE("%s, %d\n", debugstr_w(pFileName), bDeleteExistingResources);

    hUpdate = GlobalAlloc(GHND, sizeof(QUEUEDUPDATES));
    if (!hUpdate)
        return ret;

    updates = (QUEUEDUPDATES*)GlobalLock(hUpdate);
    if (updates)
    {
        list_init(&updates->root);
        updates->bDeleteExistingResources = bDeleteExistingResources;
        updates->pFileName = (LPSTR)HeapAlloc(GetProcessHeap(), 0, (lstrlenA(pFileName) + 1) * sizeof(CHAR));
        if (updates->pFileName)
        {
            lstrcpyA(updates->pFileName, pFileName);

            // Fix by Daniel Marschall: Changed "GENERIC_READ | GENERIC_WRITE, 0" to "GENERIC_READ, FILE_SHARE_READ"
            file = CreateFileA(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);

            /* if resources are deleted, only the file's presence is checked */
            if (file != INVALID_HANDLE_VALUE &&
                (bDeleteExistingResources || check_pe_exe(file, updates)))
                ret = hUpdate;
            else
                HeapFree(GetProcessHeap(), 0, updates->pFileName);

            CloseHandle(file);
        }
        GlobalUnlock(hUpdate);
    }

    if (!ret)
        GlobalFree(hUpdate);

    return ret;
}

BOOL WINAPI WineEndUpdateResourceA(HANDLE hUpdate, BOOL fDiscard)
{
    QUEUEDUPDATES* updates;
    BOOL ret;

    // TRACE("%p %d\n", hUpdate, fDiscard);

    updates = (QUEUEDUPDATES*)GlobalLock(hUpdate);
    if (!updates)
        return FALSE;

    ret = fDiscard || write_raw_resources(updates);

    free_resource_directory(&updates->root, 2);

    HeapFree(GetProcessHeap(), 0, updates->pFileName);
    GlobalUnlock(hUpdate);
    GlobalFree(hUpdate);

    return ret;
}

BOOL WINAPI WineUpdateResourceA(HANDLE hUpdate, LPCSTR lpType, LPCSTR lpName,
    WORD wLanguage, LPVOID lpData, DWORD cbData)
{
    QUEUEDUPDATES* updates;
    UNICODE_STRING nameW, typeW;
    BOOL ret = FALSE;

    //TRACE("%p %s %s %08x %p %d\n", hUpdate,
    //      debugstr_w(lpType), debugstr_w(lpName), wLanguage, lpData, cbData);

    nameW.Buffer = typeW.Buffer = NULL;
    updates = (QUEUEDUPDATES*)GlobalLock(hUpdate);
    if (updates)
    {
        if (!set_ntstatus(get_res_nameA(lpName, &nameW))) goto done;
        if (!set_ntstatus(get_res_nameA(lpType, &typeW))) goto done;

        if (lpData == NULL && cbData == 0)  /* remove resource */
        {
            ret = update_add_resource(updates, typeW.Buffer, nameW.Buffer, wLanguage, NULL, TRUE);
        }
        else
        {
            struct resource_data* data;
            data = allocate_resource_data(wLanguage, 0, lpData, cbData, TRUE);
            if (data)
                ret = update_add_resource(updates, typeW.Buffer, nameW.Buffer, wLanguage, data, TRUE);
        }

    done:
        GlobalUnlock(hUpdate);
    }

    if (!IS_INTRESOURCE(nameW.Buffer)) HeapFree(GetProcessHeap(), 0, nameW.Buffer);
    if (!IS_INTRESOURCE(typeW.Buffer)) HeapFree(GetProcessHeap(), 0, typeW.Buffer);
    return ret;
}
