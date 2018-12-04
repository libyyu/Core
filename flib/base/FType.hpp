#ifndef __FTYPE_HPP__
#define __FTYPE_HPP__
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <vector>
#include <stddef.h>
#include "FPlatform.hpp"
#if MINGW32
#include <cstring>
#else
#include <string.h>
#endif
#include "FMemTrack.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <tchar.h>
#if defined( UNICODE ) || defined( _UNICODE )
	#undef _T
    #define _T(type)           L##type
    #define _W(fun)            w##fun
    #define Fchar              wchar_t
    #define Fstring 	       std::wstring
    #define Fstringstream      std::wstringstream
    #define Fstrftime          wcsftime
    #define Fstrcat            wcscat      

    #define Ffprintf           fwprintf
	#define Fstrstr			   wcsstr
	#define Fcsstr			   wcschr
    #define Fstrrchr           wcsrchr
    #define Fstrncpy           wcsncpy
	#define Fstrncasecmp	   _wcsnicmp
    #define Fstrlen            wcslen
    #define Fstrcmp            wcscmp
	#define Fstrcpy            wcscpy
	#define Fcscspn			   wcscspn
    #define Fvfprintf          vfwprintf
    #define Fvsnprintf		   _vsnwprintf_s 
	#define Fsprintf	       swprintf
#else
    #undef _T
    #define _T(type)           type
    #define _W(fun)            fun
    #define Fchar              char
    #define Fstring			   std::string
    #define Fstringstream      std::stringstream
    #define Fstrftime          strftime
    #define Fstrcat            strcat

    #define Ffprintf           fprintf
	#define Fstrstr			   strstr
    #define Fstrrchr           strrchr
	#define Fcsstr			   strchr
    #define Fstrncpy           strncpy
	#define Fstrncasecmp	   _strnicmp
    #define Fstrlen            strlen
    #define Fstrcmp            strcmp
    #define Fstrcpy            strcpy
	#define Fcscspn			   strcspn
    #define Fvfprintf          vfprintf
	#define Fvsnprintf		   vsnprintf 
	#define Fsprintf	       sprintf
#endif
#else
#include <wchar.h>
#if defined( UNICODE ) || defined( _UNICODE )
    #undef _T
    #define _T(type)           L##type
    #define _W(fun)            w##fun
    #define Fchar              wchar_t
    #define Fstring 	       std::wstring
    #define Fstringstream      std::wstringstream
    #define Fstrftime          wcsftime
    #define Fstrcat            wcscat      

    #define Ffprintf           fwprintf
	#define Fstrstr			   wcsstr
	#define Fcsstr			   wcschr
    #define Fstrrchr           wcsrchr
    #define Fstrncpy           wcsncpy
	#define Fstrncasecmp	   wcsncasecmp
    #define Fstrlen            wcslen
    #define Fstrcmp            wcscmp
	#define Fstrcpy            wcscpy
	#define Fcscspn			   wcscspn
    #define Fvfprintf          vfwprintf
    #define Fvsnprintf		   vswprintf 
	#define Fsprintf	       swprintf
#else
    #undef _T
    #define _T(type)           type
    #define _W(fun)            fun
    #define Fchar              char
    #define Fstring			   std::string
    #define Fstringstream      std::stringstream
    #define Fstrftime          strftime
    #define Fstrcat            strcat

    #define Ffprintf           fprintf
	#define Fstrstr			   strstr
    #define Fstrrchr           strrchr
	#define Fcsstr			   strchr
    #define Fstrncpy           strncpy
	#define Fstrncasecmp	   strncasecmp
    #define Fstrlen            strlen
    #define Fstrcmp            strcmp
    #define Fstrcpy            strcpy
	#define Fcscspn			   strcspn
    #define Fvfprintf          vfprintf
	#define Fvsnprintf		   vsnprintf 
	#define Fsprintf	       sprintf
#endif
#endif

#ifdef  _DEBUG
     #define  FDeBugOut(type)   cout(type)
#else
     #define  FDeBugOut(type)   //cout(type)
#endif

#if defined(__cplusplus)
    #define _FDeclsBegin   extern "C" {
    #define _FDeclsEnd     }
#else
    #define _FDeclsBegin
    #define _FDeclsEnd
#endif

#if defined(__cplusplus)
    #define  _FCFun   extern "C"
#else
    #define  _FCFun   extern
#endif //__cplusplus

#define  _FNameSpaceBegin(name)   namespace name {
#define  _FNameSpaceEnd           }
#define  _FUsing(name)            using namespace name;

#define _FStdBegin  _FNameSpaceBegin(FStd)
#define _FStdEnd    _FNameSpaceEnd

#if defined(_F_DLL_)
	#if defined(_MSC_VER)
		#define F_DLL_API __declspec(dllexport)
	#else
		#define F_DLL_API
	#endif
#else
	#if defined(_MSC_VER)
		#define F_DLL_API __declspec(dllimport)
	#else
		#define F_DLL_API
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


#if PLATFORM_TARGET == PLATFORM_WINDOWS //&& !MINGW32
typedef __int64           				int64;
typedef unsigned __int64  				uint64;
#else
typedef __int64_t         				int64;
typedef __uint64_t        				uint64;
#endif

#ifndef _MSC_VER
typedef unsigned char byte;
#endif

typedef std::vector<uchar>              ByteArray;


#undef F_DISALLOW_CONSTRUCTORS
#define F_DISALLOW_CONSTRUCTORS(TypeName)    \
    TypeName(const TypeName&);            \
    void operator= (const TypeName&);


#define lengthof(x)   (sizeof(x)/sizeof(*x))

#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#define MAX(a,b) ((a)>(b)) ? (a) : (b)

#define JOIN( X, Y ) DO_JOIN( X, Y )
#define DO_JOIN( X, Y ) DO_JOIN2(X,Y)
#define DO_JOIN2( X, Y ) X##Y

#define PROPERTY(varType, varName, funName) \
    protected: varType varName; \
    public: virtual varType get##funName(void) const { return varName; } \
    public: virtual void set##funName(const varType& var) { varName = var; }

// #if MINGW32
// //_CRTIMP FILE* __cdecl __MINGW_NOTHROW   _fdopen (int, const char*);
// _CRTIMP FILE* __cdecl __MINGW_NOTHROW   fdopen (int, const char*);
// _CRTIMP int __cdecl __MINGW_NOTHROW _strnicmp (const char*, const char*, size_t);
// #define _fdopen fdopen
// #endif

#endif//__FTYPE_HPP__