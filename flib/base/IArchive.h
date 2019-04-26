#ifndef _IARCHIVE_H_
#define _IARCHIVE_H_
#pragma once
#include "FType.hpp"
////////////TODO: Will Imp
_FStdBegin
class IArchive
{
public:
    template<typename T>
    inline IArchive& operator<<(T v); // will generate link error
    inline IArchive& operator<<(int8 v);
    inline IArchive& operator<<(int16 v);
    inline IArchive& operator<<(int32 v);
    inline IArchive& operator<<(int64 v);
    inline IArchive& operator<<(uint8 v);
    inline IArchive& operator<<(uint16 v);
    inline IArchive& operator<<(uint32 v);
    inline IArchive& operator<<(uint64 v);
#if FLIB_COMPILER_64BITS
    inline IArchive& operator<<(int v);
    inline IArchive& operator<<(uint v);
#endif
    inline IArchive& operator<<(bool v);
    inline IArchive& operator<<(float v);
    inline IArchive& operator<<(double v);
    inline IArchive& operator<<(const char *str);
    inline IArchive& operator<<(char v[]);
public:
    virtual IArchive& Serialize();
};
_FStdEnd

#endif//_IARCHIVE_H_