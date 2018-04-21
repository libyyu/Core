#ifndef __RZPLATFORM_HPP__
#define __RZPLATFORM_HPP__
#pragma once

/********************************************************************************/
/*PLATFORM*/
/********************************************************************************/
#define PLATFORM_UNKNOW        0
#define PLATFORM_WINDOWS       1
#define PLATFORM_ANDROID       2
#define PLATFORM_IOS           3
#define PLATFORM_MACOSX        4

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#define PLATFORM_TARGET        PLATFORM_WINDOWS
#elif defined(ANDROID) || defined(_ANDROID) || defined(__ANDROID__)
#define PLATFORM_TARGET        PLATFORM_ANDROID
#elif defined(IOS) || defined(_IOS) || defined(__IOS__)
#define PLATFORM_TARGET        PLATFORM_IOS
#elif defined(MACOSX) || defined(_MACOSX) || defined(__MACOSX__) || (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_TARGET        PLATFORM_MACOSX
#else
#define PLATFORM_TARGET        PLATFORM_UNKNOW
#endif
/********************************************************************************/
/*END PLATFORM*/
/********************************************************************************/


#endif//__RZPLATFORM_HPP__
