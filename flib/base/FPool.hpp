#ifndef __FPOOL_HPP__
#define __FPOOL_HPP__
#pragma once
#include <string>
#include "FLock.hpp"

_FStdBegin
template <class T>
struct _F_Node
{
    _F_Node<T> *pNext;
    T data;	
};
template <class T, unsigned int nInitCount = 10>
class FPool
{
private:
    _F_Node<T>*           m_pHead;	                 //ͷ���
    unsigned long       m_nOutCount;            //�׳�����
    unsigned long       m_nInCount;             //���и���	
    FLock             m_lock;
public:
    FPool();
    ~FPool();
public:
    T* GetBuf();
    void FreeBuf(T* p,bool bDelete = false);
    void PrintInfo();
private:
    void DeleteAll_Node();
};
_FStdEnd

_FStdBegin
template <class T,unsigned int nInitCount>
FPool<T, nInitCount>::FPool():m_pHead(NULL),m_nOutCount(0),m_nInCount(0)
{	
    for (unsigned int i=0;i<nInitCount;++i)
    {
        _F_Node<T>* p_Node = new _F_Node<T>;
        if (p_Node)
        {
            p_Node->pNext = NULL;
            if (m_pHead==NULL)
            {
                m_pHead = p_Node;
            }
            else
            {
                p_Node->pNext = m_pHead;
                m_pHead = p_Node;
            }
            ++m_nInCount;
        }
    }
}

template <class T,unsigned int nInitCount>
FPool<T,nInitCount>::~FPool()
{
    m_lock.lock();
    if (m_pHead)
    {
        DeleteAll_Node();
    }
    m_lock.unlock();
}

template <class T,unsigned int nInitCount>
T* FPool<T,nInitCount>::GetBuf()
{
    T* pResult = NULL;
    _Node<T>* p_Node = NULL;

    m_lock.lock();
    if (m_pHead)                           //ע�ⲻ����ͷ�ڵ�Ϊ�յ����
    {                                  //Ϊ��ʱ����������������������룬���ֱ�ӷ���NULL
        p_Node = m_pHead;                   //�ڴ��ͷ�ʱֱ����ӵ�ͷ�ڵ㴦,��FreeBuf ����
        m_pHead = m_pHead->pNext;			
        pResult = &(p_Node->data);
        --m_nInCount;
        ++m_nOutCount;
    }
    else 
    {	
        p_Node = new _F_Node<T>;
        if (p_Node)
        {	
            p_Node->pNext = NULL;
            pResult = &(p_Node->data);	
            ++m_nOutCount;
        }
    }
    m_lock.unlock();

    return pResult;
}

template <class T,unsigned int nInitCount>
void FPool<T,nInitCount>::FreeBuf(T* p,bool bDelete /* = false */)
{	
    m_lock.lock();
    {
        _F_Node<T>* p_Node = (_F_Node<T>*)((char*)p - sizeof(_F_Node<T>*));
        if (bDelete)
        {
            delete p_Node;
            p_Node = NULL;
        }
        else
        {	
            p_Node->pNext = m_pHead;
            m_pHead = p_Node;
            ++m_nInCount;
        }
        --m_nOutCount;
    }	
    m_lock.unlock();
}

template <class T,unsigned int nInitCount>
void FPool<T,nInitCount>::DeleteAll_Node()
{                                       //��������˽�У����������������ã������������Դ˴����ü���
    _Node<T>* p_Node = NULL;
    while(1)
    {
        if (!m_pHead) break;
        p_Node = m_pHead->pNext;
        delete m_pHead;
        m_pHead = p_Node;
        --m_nInCount;
    }
}
_FStdEnd

#endif//__FPOOL_HPP__