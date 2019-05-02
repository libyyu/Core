#ifndef  __FDEQUEADDLOCK_HPP__
#define  __FDEQUEADDLOCK_HPP__
#pragma once
#include "FDReadAWriteLock.hpp"

_FStdBegin
template <class T>
class F_DLL_API FDequeAddLock
{
public:
    FDequeAddLock():m_pHead(NULL),m_pTail(NULL),m_nCount(0){}
    ~FDequeAddLock(){ Clear(); }
private:
    typedef struct __Node
    {
        struct __Node* pFront;
        struct __Node* pNext;
        T data;
    }Node;
    typedef bool (__stdcall *EnumVaulePro)(const T&,void*);
private:
    FDReadAWriteLock m_Lock;
    unsigned int m_nCount;
    Node* m_pHead;
    Node* m_pTail;	
public:
    FLIB_FORCEINLINE void AddTail(T& data);
    FLIB_FORCEINLINE bool DelHead();
    FLIB_FORCEINLINE void Clear();
    FLIB_FORCEINLINE bool Empty();
    FLIB_FORCEINLINE bool GetHead(T& outResult);
    FLIB_FORCEINLINE unsigned int GetCount();
    FLIB_FORCEINLINE void EnumVaule(EnumVaulePro pEnumFun,void* pParam = NULL);
};

template <class T>
void FDequeAddLock<T>::AddTail(T& data)
{
    Node* pNewNode   = new Node;
    if (!pNewNode)   return;
    pNewNode->data   = data;

    m_Lock.EnterWrite();
    {
        if ( !m_pTail || !m_pHead)
        {
            m_pHead = pNewNode;
            m_pTail = pNewNode;
        }
        else// ������ӵ�β����ͷ��㲻�ı�
        {
            pNewNode->pFront = m_pTail;
            m_pTail->pNext   = pNewNode;
            m_pTail = pNewNode;
        }
        ++m_nCount;
    }
    m_Lock.LeaveWrite();
}

template <class T>
bool FDequeAddLock<T>::GetHead(T& outResult)
{
    bool bResult = false;
    m_Lock.EnterRead();
    if (m_pHead)
    {
        outResult = m_pHead->data;
        bResult = true;
    }
    m_Lock.LeaveRead();
    return bResult;
}

template <class T>
bool FDequeAddLock<T>::DelHead()
{
    bool bResult = false;
    Node* pNode = NULL;
    m_Lock.EnterWrite();
    {
        if (m_pHead)
        {
            pNode = m_pHead;
            m_pHead = m_pHead->pNext;
            delete pNode;
            pNode = NULL;
            bResult = true;
            --m_nCount;
            if (0 == m_nCount)
            {
                m_pHead = NULL;
                m_pTail = NULL;
            }
        }
    }
    m_Lock.LeaveWrite();
    return bResult;
}

template <class T>
void FDequeAddLock<T>::Clear()
{
    Node* pNode = NULL;
    m_Lock.EnterWrite();
    {
        while(m_pHead)
        {
            pNode = m_pHead;
            if (m_pHead == m_pTail)
            {
                m_pHead = NULL;
                m_pTail = NULL;
            }
            else
            {
                m_pHead = m_pHead->pNext;
                m_pHead->pFront = NULL;
            }
            delete pNode;
            pNode = NULL;
            --m_nCount;
        }
    }
    m_Lock.LeaveWrite();
}

template <class T>
bool FDequeAddLock<T>::Empty()
{
    bool bResult = false;
    m_Lock.EnterRead();
    m_pHead ? bResult = false : bResult = true;
    m_Lock.LeaveRead();
    return bResult;
}

template <class T>
unsigned int FDequeAddLock<T>::GetCount()
{
    unsigned int nResult = 0;
    m_Lock.EnterRead();
    nResult = m_nCount;
    m_Lock.LeaveRead();
    return nResult;
}

template <class T>
void FDequeAddLock<T>::EnumVaule(EnumVaulePro pEnumFun,void* pParam /* = NULL */)
{
    m_Lock.EnterRead();
    {
        Node* pNode = NULL;
        pNode = m_pHead;
        while(pNode)
        {
            pEnumFun(pNode->data,pParam);
            pNode = pNode->pNext;
        }
    }
    m_Lock.LeaveRead();
}

_FStdEnd

#endif//__FDEQUEADDLOCK_HPP__