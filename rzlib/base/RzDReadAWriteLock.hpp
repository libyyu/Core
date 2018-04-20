#ifndef __RZDREADAWRITELOCK_HPP__
#define __RZDREADAWRITELOCK_HPP__
#pragma once
#include "RzLock.hpp"
#include "RzFunc.hpp"

_RzStdBegin

class CRzDReadAWriteLock
{
    typedef struct _RZ_InsideLock_
    {
        int nReadCount;
        bool bWriteFlag;
        CRzLock rzLock;
    }SInsideLock; //�ڲ���
private:
	SInsideLock m_Lock;
public:
    CRzDReadAWriteLock(void);
    virtual ~CRzDReadAWriteLock(void);
public:
	void EnterWrite();
    void LeaveWrite();
    int EnterRead();
    int LeaveRead();
    int GetReadCount();
    bool IsWrite();
};

CRzDReadAWriteLock::CRzDReadAWriteLock(void)
{
    m_Lock.nReadCount = 0;
	m_Lock.bWriteFlag = false;
}
CRzDReadAWriteLock::~CRzDReadAWriteLock(void)
{
    m_Lock.rzLock.lock();
    m_Lock.rzLock.unlock();
}

void CRzDReadAWriteLock::EnterWrite()
{
    while(1)
    {
        m_Lock.rzLock.lock();
        if ( !m_Lock.bWriteFlag ) //��дʱ��������д���£�д�õ�����Ȩ
        {
            m_Lock.bWriteFlag = true;
            m_Lock.rzLock.unlock();
            goto _Start_Write;
        }
        m_Lock.rzLock.unlock();
        RzSleep(0/*1*/);
    }
_Start_Write:
    while(GetReadCount())//�ȴ���������Ϊ 0
    {
        RzSleep(/*1*/0);
    }
}

void CRzDReadAWriteLock::LeaveWrite()
{
    m_Lock.rzLock.lock();
    m_Lock.bWriteFlag = false;
    m_Lock.rzLock.unlock();
}

int CRzDReadAWriteLock::EnterRead()
{
    int n = 0;
    while(1)
    {
        m_Lock.rzLock.lock();
        if (!m_Lock.bWriteFlag) //���߳���д
        {		
            n = ++m_Lock.nReadCount;
            goto _Start_Read;
        }
        m_Lock.rzLock.unlock();
        RzSleep(0/*1*/);
    }
_Start_Read:
    m_Lock.rzLock.unlock();
    return n;
}

int CRzDReadAWriteLock::LeaveRead()
{
    int n = 0;
    m_Lock.rzLock.lock();
    if ( 0 < m_Lock.nReadCount )
    {
        n = --m_Lock.nReadCount;
    }
    m_Lock.rzLock.unlock();
    return n;
}

int CRzDReadAWriteLock::GetReadCount()
{
    int n = 0;
    m_Lock.rzLock.lock();
    n = m_Lock.nReadCount;
    m_Lock.rzLock.unlock();
    return n;
};
bool CRzDReadAWriteLock::IsWrite()
{
    bool bVar = false;
    m_Lock.rzLock.lock();
    bVar = m_Lock.bWriteFlag;
    m_Lock.rzLock.unlock();
    return bVar;
}

_RzStdEnd

#endif//__RZDREADAWRITELOCK_HPP__