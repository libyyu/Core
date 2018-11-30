#ifndef  __FCOUNTER_HPP__
#define  __FCOUNTER_HPP__
#pragma once
#include "FLock.hpp"
_FStdBegin
class FCounter
{
public:
    explicit FCounter():m_i(0){};
    ~FCounter(){};
private:
    FLock m_Lock;
    unsigned int m_i;
public:
    unsigned int  Set(const unsigned int& n)
    {
        unsigned int nResult = n;
        m_Lock.lock();
        m_i = n;
        m_Lock.unlock();
        return nResult;
    };
    unsigned int  Add()
    {
        unsigned int nResult = 0;
        m_Lock.lock();
        nResult = ++m_i;
        m_Lock.unlock();
        return nResult;
    };
    unsigned int  Dec()
    {
        unsigned int nResult = 0;
        m_Lock.lock();
        nResult = --m_i;
        m_Lock.unlock();
        return nResult;
    };
    unsigned int  GetCount()
    {
        unsigned int nResult = 0;
        m_Lock.lock();
        nResult = m_i;
        m_Lock.unlock();
        return nResult;
    };
};
_FStdEnd
#endif//__FCOUNTER_HPP__

