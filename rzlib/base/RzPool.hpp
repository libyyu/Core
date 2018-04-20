#ifndef __RZPOOL_HPP__
#define __RZPOOL_HPP__
#pragma once
#include <string>
#include "RzLock.hpp"

_RzStdBegin
template <class T>
struct _RZ_Node
{
    _RZ_Node<T> *pNext;
    T data;	
};
template <class T, unsigned int nInitCount = 10>
class CRzPool
{
private:
    _RZ_Node<T>*           m_pHead;	                 //ͷ���
    unsigned long       m_nOutCount;            //�׳�����
    unsigned long       m_nInCount;             //���и���	
    CRzLock             m_lock;
public:
    CRzPool();
    ~CRzPool();
public:
    T* GetBuf();
    void FreeBuf(T* p,bool bDelete = false);
    void PrintInfo();
private:
    void DeleteAll_Node();
};
_RzStdEnd

_RzStdBegin
template <class T,unsigned int nInitCount>
CRzPool<T, nInitCount>::CRzPool():m_pHead(NULL),m_nOutCount(0),m_nInCount(0)
{	
    for (unsigned int i=0;i<nInitCount;++i)
    {
        _RZ_Node<T>* p_Node = new _RZ_Node<T>;
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
CRzPool<T,nInitCount>::~CRzPool()
{
    m_lock.lock();
    if (m_pHead)
    {
        DeleteAll_Node();
    }
    m_lock.unlock();
}

template <class T,unsigned int nInitCount>
T* CRzPool<T,nInitCount>::GetBuf()
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
        p_Node = new _RZ_Node<T>;
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
void CRzPool<T,nInitCount>::FreeBuf(T* p,bool bDelete /* = false */)
{	
    m_lock.lock();
    {
        _RZ_Node<T>* p_Node = (_RZ_Node<T>*)((char*)p - sizeof(_RZ_Node<T>*));
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
void CRzPool<T,nInitCount>::DeleteAll_Node()
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
_RzStdEnd

#endif//__RZPOOL_HPP__