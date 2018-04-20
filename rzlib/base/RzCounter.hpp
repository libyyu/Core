#ifndef  __RZCOUNTER_HPP__
#define  __RZCOUNTER_HPP__
#pragma once
#include "RzLock.hpp"
_RzStdBegin
class CRzCounter
{
public:
    explicit CRzCounter():m_i(0){};
    ~CRzCounter(){};
private:
    CRzLock m_Lock;
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
_RzStdEnd
#endif//__RZCOUNTER_HPP__

