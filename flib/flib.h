#ifndef _FLIB_H_
#define _FLIB_H_
#pragma once
#include "base/FFunc.hpp"
#include "base/FBuffer.hpp"
#include "base/FString.hpp"
#include "base/FWString.hpp"
#include "base/FConvert.hpp"
#include "base/FCounter.hpp"
#include "base/FEncoding.hpp"
#include "base/FFile.hpp"
#include "base/FIni.hpp"
#include "base/FLock.hpp"
#include "base/FMD5.hpp"
#include "base/FMemory.hpp"
#include "base/FMemPool.hpp"
#include "base/FAlloctor.hpp"
#include "base/FPool.hpp"
#include "base/FSemaphore.hpp"
#include "base/FThread.hpp"
#include "base/FTimer.hpp"
#include "base/FSingleton.hpp"
#include "base/FPlugin.hpp"
#include "base/FProcess.hpp"
#include "base/FBase64.hpp"
#include "base/FDataTable.hpp"
#include "base/FLogFile.hpp"
#include "base/FConsole.hpp"
#include "base/FValue.hpp"
////////////////////////////////////
////
_FStdBegin
#if defined( UNICODE ) || defined( _UNICODE )
typedef  FWString  String;
#else
typedef  FString   String;
#endif

typedef std::basic_string<char, std::char_traits<char>, FSTLAllocator<char> > string;
typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, FSTLAllocator<wchar_t> > wstring;

_FStdEnd


#endif//_FLIB_H_
