#ifndef __FDREADAWRITELOCK_HPP__
#define __FDREADAWRITELOCK_HPP__
#pragma once
#include "FLock.hpp"
#include "FFunc.hpp"

_FStdBegin

class FDReadAWriteLock
{
    typedef struct _F_InsideLock_
    {
        int nReadCount;
        bool bWriteFlag;
        FLock lock;
    }SInsideLock; //�ڲ���
private:
	SInsideLock m_Lock;
public:
    FDReadAWriteLock(void);
    virtual ~FDReadAWriteLock(void);
public:
	void EnterWrite();
    void LeaveWrite();
    int EnterRead();
    int LeaveRead();
    int GetReadCount();
    bool IsWrite();
};

FDReadAWriteLock::FDReadAWriteLock(void)
{
    m_Lock.nReadCount = 0;
	m_Lock.bWriteFlag = false;
}
FDReadAWriteLock::~FDReadAWriteLock(void)
{
    m_Lock.lock.lock();
    m_Lock.lock.unlock();
}

void FDReadAWriteLock::EnterWrite()
{
    while(1)
    {
        m_Lock.lock.lock();
        if ( !m_Lock.bWriteFlag ) //��дʱ��������д���£�д�õ�����Ȩ
        {
            m_Lock.bWriteFlag = true;
            m_Lock.lock.unlock();
            goto _Start_Write;
        }
        m_Lock.lock.unlock();
        FSleep(0/*1*/);
    }
_Start_Write:
    while(GetReadCount())//�ȴ���������Ϊ 0
    {
        FSleep(/*1*/0);
    }
}

void FDReadAWriteLock::LeaveWrite()
{
    m_Lock.lock.lock();
    m_Lock.bWriteFlag = false;
    m_Lock.lock.unlock();
}

int FDReadAWriteLock::EnterRead()
{
    int n = 0;
    while(1)
    {
        m_Lock.lock.lock();
        if (!m_Lock.bWriteFlag) //���߳���д
        {		
            n = ++m_Lock.nReadCount;
            goto _Start_Read;
        }
        m_Lock.lock.unlock();
        FSleep(0/*1*/);
    }
_Start_Read:
    m_Lock.lock.unlock();
    return n;
}

int FDReadAWriteLock::LeaveRead()
{
    int n = 0;
    m_Lock.lock.lock();
    if ( 0 < m_Lock.nReadCount )
    {
        n = --m_Lock.nReadCount;
    }
    m_Lock.lock.unlock();
    return n;
}

int FDReadAWriteLock::GetReadCount()
{
    int n = 0;
    m_Lock.lock.lock();
    n = m_Lock.nReadCount;
    m_Lock.lock.unlock();
    return n;
};
bool FDReadAWriteLock::IsWrite()
{
    bool bVar = false;
    m_Lock.lock.lock();
    bVar = m_Lock.bWriteFlag;
    m_Lock.lock.unlock();
    return bVar;
}

_FStdEnd

#endif//__FDREADAWRITELOCK_HPP__