#ifndef __FPLATFORM_HPP__
#define __FPLATFORM_HPP__
#pragma once

/********************************************************************************/
/*PLATFORM*/
/********************************************************************************/
#if _MSC_VER

#if !defined(_CRT_SECURE_NO_WARNINGS)
#   define _CRT_SECURE_NO_WARNINGS
#endif

#define FLIB_COMPILER_MSVC       1

#if _MSC_VER >= 1900
#   define FLIB_COMPILER_MSVC2015   1
#   define FLIB_COMPILER_NAME       "vs2015"
#elif _MSC_VER >= 1800
#   define FLIB_COMPILER_MSVC2013   1
#   define FLIB_COMPILER_NAME       "vs2013"
#elif _MSC_VER >= 1600
#   define FLIB_COMPILER_MSVC2010   1
#   define FLIB_COMPILER_NAME       "vs2010"
#elif _MSC_VER >= 1500
#   define FLIB_COMPILER_MSVC2008   1
#   define FLIB_COMPILER_NAME       "vs2008"
#   define nullptr 0
#elif _MSC_VER >= 1400
#   define FLIB_COMPILER_MSVC2005   1
#   define FLIB_COMPILER_NAME       "vs2005"
#   define nullptr 0
#elif _MSC_VER >= 1310
#   define FLIB_COMPILER_MSVC2003   1
#   define FLIB_COMPILER_NAME       "vs2003"
#   define nullptr 0
#else
#   error Requires Visual C++ 2003 or above
#endif//_MSC_VER >= 1900

#if defined(_WIN64)
#   define FLIB_COMPILER_64BITS      1
#endif//_WIN64

#elif defined(ANDROID) || defined(_ANDROID) || defined(__ANDROID__)
#   define FLIB_COMPILER_ANDROID     1
#   define FLIB_COMPILER_NAME        "android"
#if defined(__LP64__)
#   define FLIB_COMPILER_64BITS      1
#endif//__LP64__

#elif defined(IOS) || defined(_IOS) || defined(__IOS__)
#   define FLIB_COMPILER_IOS         1
#   define FLIB_COMPILER_NAME        "ios"
#if defined(__LP64__)
#   define FLIB_COMPILER_64BITS      1
#endif

#elif defined(__APPLE_CC__) || defined(MACOSX) || defined(_MACOSX) || defined(__MACOSX__)
#   define FLIB_COMPILER_MACOSX      1
#   define FLIB_COMPILER_NAME        "macosx"
#if defined(__LP64__)
#   define FLIB_COMPILER_64BITS      1
#endif

#elif __CYGWIN__ || __MINGW32__ || __MINGW64__
#   define FLIB_COMPILER_CYGWIN      1
#   define FLIB_COMPILER_NAME        "cygwin"
#if __GNUC__ < 4 || __GNUC_MINOR < 6
#   define nullptr 0
#endif
#if defined(__LP64__) || defined(__MINGW64__)
#   define FLIB_COMPILER_64BITS      1
#endif

#elif defined(__linux__)
#   define FLIB_COMPILER_LINUX       1
#   define FLIB_COMPILER_NAME        "linux"
#if defined(__LP64__)
#   define FLIB_COMPILER_64BITS      1
#endif
#else
#   error Unsupported C++ compiler
#endif//_MSC_VER
/********************************************************************************/
/*END PLATFORM*/
/********************************************************************************/

#if FLIB_COMPILER_MSVC
#   define FLIB_FORCEINLINE                            __inline
#   define FLIB_FORCENOINLINE                          _declspec(noinline)
#   define FLIB_UNUSED
#   define FLIB_ALIAS
#   if _MSC_VER >= 1400
#       define FLIB_RESTRICT                           __restrict
#   else
#       define FLIB_RESTRICT
#   endif//#if _MSC_VER >= 1400
#   define FLIB_ALIGN_PREFIX(alignment)				   __declspec(align(alignment))
#   define FLIB_ALIGN_SUFFIX(alignment)
#   ifdef FLIB_DLL
#       define FLIB_DLL_IMPORT						   __declspec(dllimport)
#       define FLIB_DLL_EXPORT						   __declspec(dllexport)
#   else
#       define FLIB_DLL_IMPORT
#       define FLIB_DLL_EXPORT
#   endif//FLIB_DLL
#else
#   define FLIB_FORCEINLINE                            inline
#   define FLIB_RESTRICT                               __restrict__
#   define FLIB_UNUSED                                 __attribute__((unused))
#   define FLIB_ALIAS                                  __attribute__((__may_alias__))
#   define FLIB_ALIGN_PREFIX(alignment)
#   define FLIB_ALIGN_SUFFIX(alignment)				   __attribute__((aligned(n)))
#   ifdef FLIB_DLL
#       define FLIB_DLL_IMPORT
#       define FLIB_DLL_EXPORT						   __attribute__((visibility("default")))
#   else
#       define FLIB_DLL_IMPORT
#       define FLIB_DLL_EXPORT
#   endif//FLIB_DLL
#endif//FLIB_COMPILER_MSVC

#ifdef FLIB_DLL_EXPORTS
#	define FLIB_DLL_API FLIB_DLL_EXPORT
#else
#	define FLIB_DLL_API FLIB_DLL_IMPORT
#endif//FLIB_DLL_EXPORTS
#define F_DLL_API FLIB_DLL_API

#if FLIB_COMPILER_MSVC
//warning C4275: non dll-interface class 'stdext::exception' used as base for dll-interface class 'std::bad_cast'
#pragma warning(disable : 4275)
//warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable : 4530)
//warning C4251: 'behaviac::VariableRegistry::m_proxyHolders' : class 'behaviac::vector<T>' needs to have dll-interface to be used
//by clients of class 'behaviac::VariableRegistry'
#pragma warning(disable : 4251)
//unreferenced formal parameter
#pragma warning(disable : 4100)
#pragma warning(disable : 4127) // conditional expression is constant
//warning C4702: unreachable code
#pragma warning(disable : 4702)
//'strcpy': This function or variable may be unsafe.
#pragma warning(disable : 4996)
#endif//FLIB_COMPILER_MSVC

#endif//__FPLATFORM_HPP__
