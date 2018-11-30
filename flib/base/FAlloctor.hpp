#ifndef __FALLOCTOR_HPP__
#define __FALLOCTOR_HPP__

#pragma once
#include <stdlib.h>
#include "FLock.hpp"
_FStdBegin
template<size_t T,size_t MAXSIZE,typename LOCK = FLock,size_t C = 16> 
class FAlloctorPool
{ 
public: 
	FAlloctorPool():_pBlocks(NULL),_pFreeNodes(NULL){} 
	virtual ~FAlloctorPool()
	{
		_lock.lock(); 
		while (NULL != _pBlocks) 
		{ 
			PBLOCK pItem = _pBlocks; 
			_pBlocks = _pBlocks->pPre; 
			//free(pItem);
			delete pItem;
		} 
		_lock.unlock(); 
	}

public: 
	void*   Alloc(size_t size)
	{
		assert(size);
		if(0 == size)
			return NULL;
		if(size > T)
			return this->_next.Alloc(size);

		_lock.lock(); 
		if (_pFreeNodes == NULL) 
		{ 
			// PBLOCK pBlock = (PBLOCK)malloc(sizeof(BLOCK)); 
			PBLOCK pBlock = new BLOCK;
			assert(pBlock);
			if (NULL == pBlock ) 
			{ 
				_lock.unlock(); 
				return NULL; 
			} 

			pBlock->pPre = _pBlocks; 
			pBlock->pNodes[0].pPre = NULL; 
			for (size_t i=1;i<C;++i)  
			{ 
				pBlock->pNodes[i].pPre = &pBlock->pNodes[i - 1]; 
			} 
			_pBlocks = pBlock; 
			_pFreeNodes = &_pBlocks->pNodes[C - 1]; 
		}

		PNODE pNode = _pFreeNodes; 
		_pFreeNodes = _pFreeNodes->pPre; 

		_lock.unlock(); 

		//pNode->pPre = (PNODE)T; //指向一个常指针区域
		pNode->pPre = (PNODE)this;

		return pNode->szBuf; 
	}
	void    Free(void *p)
	{
		size_t size = *((size_t*)p - 1); 
		assert(size);
		if(this != (self_type *)size)
			return this->_next.Free(p);

		PNODE pNode = (PNODE)((char*)p - sizeof(PNODE)); 

		_lock.lock(); 
		pNode->pPre = _pFreeNodes; 
		_pFreeNodes = pNode; 
		_lock.unlock(); 
	}

protected: 
	typedef FAlloctorPool<T,MAXSIZE,LOCK,C>   self_type;
	typedef LOCK                         lock_type;
	typedef FAlloctorPool<2*T,MAXSIZE,LOCK,C> next_type;
	FAlloctorPool(const self_type &Pool); 
	self_type operator =(const self_type &Pool);    
protected: 
	typedef struct tagNODE 
	{ 
		tagNODE *       pPre; 
		char            szBuf[T]; 
	}NODE, *PNODE; 

	typedef struct tagBLOCK 
	{
		tagBLOCK *      pPre;
		NODE            pNodes[C];
	}BLOCK, *PBLOCK; 

protected: 
	PBLOCK       _pBlocks; 
	PNODE        _pFreeNodes; 
	lock_type    _lock;
	next_type    _next;
}; 
_FStdEnd

_FStdBegin
template<size_t T,typename LOCK,size_t C> 
class FAlloctorPool<T, T, LOCK, C>
{ 
public: 
	FAlloctorPool():_pHeader(NULL){}
	virtual ~FAlloctorPool()
	{
		_lock.lock();
		while(NULL != _pHeader)
		{
			PNODE pItem = _pHeader;
			_pHeader    = _pHeader->pPre;
			free(pItem);
		}
		_lock.unlock();
	}

public: 
	inline void* Alloc(size_t size)
	{
		assert(size);

		size_t newsize = size + sizeof(NODE);
		newsize =  (((newsize)+7) &~ 7);// 按8字节对齐

		PNODE pItem = (PNODE)malloc(newsize);

		if(NULL == pItem)
			return NULL;

		_lock.lock();
		pItem->pPre       = _pHeader;
		if(NULL != _pHeader)  (_pHeader->pNext = pItem);
		_pHeader          = pItem;
		_lock.unlock();

		pItem->size        = (size_t)this;

		return (char*)pItem + sizeof(NODE);
	}
	inline void Free(void *p)
	{
		assert(p);
		size_t size = *((size_t*)p - 1); 
		assert(size > 0);

		assert(this == (self_type *)size);
		if (this == (self_type *)size) {

			PNODE pItem = (PNODE)((char*)p - sizeof(NODE));

			_lock.lock();
			if(pItem == _pHeader)
				_pHeader = _pHeader->pPre;
			else {
				PNODE preNode = pItem->pPre;
				pItem->pNext->pPre = preNode;
				if(preNode)
					preNode->pNext = pItem->pNext;
			}
			_lock.unlock();

			free(pItem);
		}
	}

protected: 
	typedef FAlloctorPool<T,T,LOCK,C>         self_type;
	typedef LOCK                         lock_type;

	FAlloctorPool(const self_type &Pool); 
	self_type operator =(const self_type &Pool);    
protected: 
	typedef struct tagNODE {
		tagNODE *  pPre;
		tagNODE *  pNext;
		size_t     size;
	}NODE,*PNODE;

protected: 
	PNODE            _pHeader;
	lock_type        _lock;
}; 
_FStdEnd

_FStdBegin
template<typename LOCK,size_t C> 
class FAlloctorPool<0, 0, LOCK, C>; //MAKE ERROR

_FStdEnd


#endif//__FALLOCTOR_HPP__