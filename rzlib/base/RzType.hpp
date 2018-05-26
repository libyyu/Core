#ifndef __RZTYPE_HPP__
#define __RZTYPE_HPP__
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <stddef.h>
#include "RzPlatform.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <tchar.h>
#if defined( UNICODE ) || defined( _UNICODE )
    #define _T(type)            L##type
    #define _W(fun)             w##fun
    #define Rzchar              wchar_t
    #define Rzstring 	        std::wstring
    #define Rzstringstream      std::wstringstream
    #define Rzstrftime          wcsftime
    #define Rzstrcat            wcscat      

    #define Rzfprintf           fwprintf
	#define Rzstrstr			wcsstr
	#define Rzcsstr				wcschr
    #define Rzstrrchr           wcsrchr
    #define Rzstrncpy           wcsncpy
	#define Rzcsicmp			wcsicmp
    #define Rzstrlen            wcslen
    #define Rzstrcmp            wcscmp
	#define Rzstrcpy            wcscpy
	#define Rzcscspn			wcscspn
    #define Rzvfprintf          vfwprintf
    #define Rzvsnprintf		    _vsnwprintf_s 
	#define Rzsprintf	        swprintf
#else
    #define _T(type)            type
    #define _W(fun)             fun
    #define Rzchar              char
    #define Rzstring			std::string
    #define Rzstringstream      std::stringstream
    #define Rzstrftime          strftime
    #define Rzstrcat            strcat

    #define Rzfprintf           fprintf
	#define Rzstrstr			strstr
    #define Rzstrrchr           strrchr
	#define Rzcsstr				strchr
    #define Rzstrncpy           strncpy
	#define Rzcsicmp			stricmp
    #define Rzstrlen            strlen
    #define Rzstrcmp            strcmp
    #define Rzstrcpy            strcpy
	#define Rzcscspn			strcspn
    #define Rzvfprintf          vfprintf
	#define Rzvsnprintf			vsnprintf 
	#define Rzsprintf	        sprintf
#endif
#else
#include <wchar.h>
#if defined( UNICODE ) || defined( _UNICODE )
    #define _T(type)            L##type
    #define _W(fun)             w##fun
    #define Rzchar              wchar_t
    #define Rzstring 	        std::wstring
    #define Rzstringstream      std::wstringstream
    #define Rzstrftime          wcsftime
    #define Rzstrcat            wcscat      

    #define Rzfprintf           fwprintf
	#define Rzstrstr			wcsstr
	#define Rzcsstr				wcschr
    #define Rzstrrchr           wcsrchr
    #define Rzstrncpy           wcsncpy
	#define Rzcsicmp			wcscasecmp
    #define Rzstrlen            wcslen
    #define Rzstrcmp            wcscmp
	#define Rzstrcpy            wcscpy
	#define Rzcscspn			wcscspn
    #define Rzvfprintf          vfwprintf
    #define Rzvsnprintf		    vswprintf 
	#define Rzsprintf	        swprintf
#else
    #define _T(type)            type
    #define _W(fun)             fun
    #define Rzchar              char
    #define Rzstring			std::string
    #define Rzstringstream      std::stringstream
    #define Rzstrftime          strftime
    #define Rzstrcat            strcat

    #define Rzfprintf           fprintf
	#define Rzstrstr			strstr
    #define Rzstrrchr           strrchr
	#define Rzcsstr				strchr
    #define Rzstrncpy           strncpy
	#define Rzcsicmp			strcasecmp
    #define Rzstrlen            strlen
    #define Rzstrcmp            strcmp
    #define Rzstrcpy            strcpy
	#define Rzcscspn			strcspn
    #define Rzvfprintf          vfprintf
	#define Rzvsnprintf			vsnprintf 
	#define Rzsprintf	        sprintf
#endif
#endif

#ifdef  _DEBUG
     #define  RzDeBugOut(type)   cout(type)
#else
     #define  RzDeBugOut(type)   //cout(type)
#endif

#if defined(__cplusplus)
    #define _RzDeclsBegin   extern "C" {
    #define _RzDeclsEnd     }
#else
    #define _RzDeclsBegin
    #define _RzDeclsEnd
#endif

#if defined(__cplusplus)
    #define  _RzCFun   extern "C"
#else
    #define  _RzCFun   extern
#endif //__cplusplus

#define  _RzNameSpaceBegin(name)   namespace name {
#define  _RzNameSpaceEnd           }
#define  _RzUsing(name)            using namespace name;

#define _RzStdBegin  _RzNameSpaceBegin(RzStd)
#define _RzStdEnd    _RzNameSpaceEnd

#if defined(_RZ_DLL_)
	#if defined(_MSC_VER)
		#define RZ_DLL_API __declspec(dllexport)
	#else
		#define RZ_DLL_API
	#endif
#else
	#if defined(_MSC_VER)
		#define RZ_DLL_API __declspec(dllimport)
	#else
		#define RZ_DLL_API
	#endif
#endif

/********************************************************************************
 
 Base integer types for all target OS's and CPU's
 
 UInt8            8-bit unsigned integer 
 SInt8            8-bit signed integer
 UInt16          16-bit unsigned integer 
 SInt16          16-bit signed integer           
 UInt32          32-bit unsigned integer 
 SInt32          32-bit signed integer   
 UInt64          64-bit unsigned integer 
 SInt64          64-bit signed integer   
 
 *********************************************************************************/
typedef unsigned char                   uchar;
typedef unsigned short                  ushort;
typedef unsigned int                    uint;
typedef unsigned long                   ulong;

typedef char							int8;
typedef uchar                           uint8;
typedef short							int16;
typedef ushort                          uint16;


#ifdef __LP64__
typedef int								int32;
typedef uint                            uint32;
#else
typedef long							int32;
typedef unsigned long                   uint32;
#endif


#ifdef _MSC_VER
typedef __int64           				int64;
typedef unsigned __int64  				uint64;
#else
typedef __int64_t         				int64;
typedef __uint64_t        				uint64;
#endif

#ifndef _MSC_VER
typedef unsigned char byte;
#endif

#undef RZ_DISALLOW_CONSTRUCTORS
#define RZ_DISALLOW_CONSTRUCTORS(TypeName)    \
    TypeName(const TypeName&);            \
    void operator= (const TypeName&)


#define lengthof(x)   (sizeof(x)/sizeof(*x))

#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#define MAX(a,b) ((a)>(b)) ? (a) : (b)

#endif//__RZTYPE_HPP__