#ifndef __FPOOL_HPP__
#define __FPOOL_HPP__
#pragma once
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
    _F_Node<T>*         m_pHead;	                 //头锟斤拷锟�
    unsigned long       m_nOutCount;            //锟阶筹拷锟斤拷锟斤拷
    unsigned long       m_nInCount;             //锟斤拷锟叫革拷锟斤拷	
    FLock               m_lock;
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
    if (m_pHead)                           //注锟解不锟斤拷锟斤拷头锟节碉拷为锟秸碉拷锟斤拷锟�
    {                                  //为锟斤拷时锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷耄拷锟斤拷直锟接凤拷锟斤拷NULL
        p_Node = m_pHead;                   //锟节达拷锟酵凤拷时直锟斤拷锟斤拷拥锟酵凤拷诘愦�,锟斤拷FreeBuf 锟斤拷锟斤拷
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
{                                       //锟斤拷锟斤拷锟斤拷锟斤拷私锟叫ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆此达拷锟斤拷锟矫硷拷锟斤拷
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
