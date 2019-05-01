#ifndef _FLIB_STRING_H
#define _FLIB_STRING_H
#pragma once
#include "../FAlloctor.hpp"
_FStdBegin
typedef std::basic_string<char, std::char_traits<char>, FSTLAllocator<char> > string;
typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, FSTLAllocator<wchar_t> > wstring;
_FStdEnd
#endif//_FLIB_STRING_H