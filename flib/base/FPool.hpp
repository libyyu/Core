#ifndef __FPOOL_HPP__
#define __FPOOL_HPP__
#pragma once
#include "FLock.hpp"
#include "FMemory.hpp"

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
    _F_Node<T>*         m_pHead;	                 //澶撮敓鏂ゆ嫹閿燂拷
    unsigned long       m_nOutCount;            //閿熼樁绛规嫹閿熸枻鎷烽敓鏂ゆ嫹
    unsigned long       m_nInCount;             //閿熸枻鎷烽敓鍙潻鎷烽敓鏂ゆ嫹	
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
    _F_Node<T>* p_Node = NULL;

    m_lock.lock();
    if (m_pHead)                           //娉ㄩ敓瑙ｄ笉閿熸枻鎷烽敓鏂ゆ嫹澶撮敓鑺傜鎷蜂负閿熺Ц纰夋嫹閿熸枻鎷烽敓锟?
    {                                  //涓洪敓鏂ゆ嫹鏃堕敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鑰勵剨鎷烽敓鏂ゆ嫹鐩撮敓鎺ュ嚖鎷烽敓鏂ゆ嫹NULL
        p_Node = m_pHead;                   //閿熻妭杈炬嫹閿熼叺鍑ゆ嫹鏃剁洿閿熸枻鎷烽敓鏂ゆ嫹鎷ラ敓閰靛嚖鎷疯瘶鎰︼拷,閿熸枻鎷稦reeBuf 閿熸枻鎷烽敓鏂ゆ嫹
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
{                                       //閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹绉侀敓鍙綇鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熺煫锝忔嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鐨嗘杈炬嫹閿熸枻鎷烽敓鐭》鎷烽敓鏂ゆ嫹
    _F_Node<T>* p_Node = NULL;
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
