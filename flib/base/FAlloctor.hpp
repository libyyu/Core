#ifndef __FALLOCTOR_HPP__
#define __FALLOCTOR_HPP__
#pragma once
#include "FLock.hpp"
#include "FMemory.hpp"

_FStdBegin
template<size_t T,typename LOCK = FLock,size_t C = 16> 
class FAlloctorPool
{ 
public: 
	FAlloctorPool(): _pBlocks(NULL) , _pFreeNodes(NULL) {}
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
    
	typedef FAlloctorPool<T,LOCK,C> self_type;
	typedef LOCK               		lock_type;
    
public: 
	void*   Alloc()
	{
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
		PNODE pNode = (PNODE)((char*)p - sizeof(PNODE)); 
    
		_lock.lock(); 
		pNode->pPre = _pFreeNodes; 
		_pFreeNodes = pNode; 
		_lock.unlock(); 
	}
    
protected: 
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
};

template<typename LOCK,size_t C> 
class FAlloctorPool<0,LOCK,C>; //MAKE ERROR
_FStdEnd

_FStdBegin
class FAlloctorPoolBase 
{
public:
    FAlloctorPoolBase(){}
    virtual ~FAlloctorPoolBase(){} 
public:
    virtual void* Alloc(size_t size)=0;
    virtual void  Free(void* p)=0;
};

template <typename LOCK = FLock>
class FAlloctorPoolMgr : public FAlloctorPoolBase
{
public:
	explicit FAlloctorPoolMgr() :_pHeader(NULL) {}
	virtual ~FAlloctorPoolMgr()
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
	void* Alloc(size_t size)
	{
		assert(size > 0);  
		if (size <= 0)
			return NULL;
		if ( size <= 16 ) 
			return _pool16.Alloc(); 
		if ( size <= 32 ) 
			return _pool32.Alloc(); 	    
		if ( size <= 64 ) 
			return _pool64.Alloc(); 	    
		if ( size <= 128 ) 
			return _pool128.Alloc(); 	    
		if ( size <= 256 ) 
			return _pool256.Alloc(); 	    
		if ( size <= 512 ) 
			return _pool512.Alloc(); 	    
		if ( size <= 1024 ) 
			return _pool1024.Alloc();     
		if (size <= 2048)
			return _pool2048.Alloc();    
		if (size <= 4096)
			return _pool4096.Alloc();	    
		if (size <= 8192)
			return _pool8192.Alloc();
	    	    
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
	     
	    //  printf("alloc pitem:%p,this:%p\n",pItem,this);
		return (char*)pItem + sizeof(NODE);
	}
	void  Free(void* p)
	{
		assert(NULL != p);
		if (NULL == p ) 
			return ; 
	    
		size_t size = *((size_t*)p - 1); 
		assert(size > 0);
	    
	    if (this == (self_type *)size) 
	    {    
	        PNODE pItem = (PNODE)((char*)p - sizeof(NODE));
	      //   printf("free  pitem:%p,this:%p\n",pItem,this);

	        _lock.lock();
	        if(pItem == _pHeader)
	            _pHeader = _pHeader->pPre;
	        else 
	        {
	            PNODE preNode = pItem->pPre;
	            pItem->pNext->pPre = preNode;
	            if(preNode)
	                preNode->pNext = pItem->pNext;
	        }
	        _lock.unlock();
	        
	        free(pItem);
	        return;
	    }
	    
	    FAlloctorPool<16,LOCK> * pool = (FAlloctorPool<16,LOCK> *)size;  
	    return pool->Free(p);
	}
    
protected: 
	typedef FAlloctorPoolMgr<LOCK> self_type;
	typedef LOCK    lock_type;
	FAlloctorPoolMgr(const self_type &Pool); 
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
    
    FAlloctorPool<16,lock_type>     _pool16; 
	FAlloctorPool<32,lock_type>     _pool32; 
	FAlloctorPool<64,lock_type>     _pool64; 
	FAlloctorPool<128,lock_type>    _pool128; 
	FAlloctorPool<256,lock_type>    _pool256; 
	FAlloctorPool<512,lock_type>    _pool512; 
	FAlloctorPool<1024,lock_type>   _pool1024;
	FAlloctorPool<2048,lock_type>   _pool2048;
	FAlloctorPool<4096,lock_type>   _pool4096;
	FAlloctorPool<8192,lock_type>   _pool8192;
};

class FAlloctorPoolFactory
{
public:
    static FAlloctorPoolMgr<FLock>* get()
    {
        static FAlloctorPoolMgr<FLock> __mempool;
        return &__mempool;
    }
};

inline void* FPoolMalloc(size_t nSize)
{
    return FAlloctorPoolFactory::get()->Alloc(nSize);
}
inline void  FPoolFree(void* p)
{
    return FAlloctorPoolFactory::get()->Free(p);
}

_FStdEnd

_FStdBegin
template <typename T> 
class FSTLAllocator;
template <> 
class FSTLAllocator<void> 
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	// reference to void members are impossible.
	typedef void value_type;
	template <class U>
	struct rebind 
	{
		typedef FSTLAllocator<U> other;
	};
};

namespace internal {
	FLIB_FORCEINLINE void destruct(char*) {}
	FLIB_FORCEINLINE void destruct(wchar_t*) {}
	template <typename T>
	FLIB_FORCEINLINE void destruct(T* t) 
	{
		((void)&t);
		t->~T();
	}
} // namespace internal

template <typename T>
class FSTLAllocator {
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	template <class U>
	struct rebind 
	{
		typedef FSTLAllocator<U> other;
	};

	FSTLAllocator() {}
	pointer address(reference x) 
	{
		return&x;
	}
	const_pointer address(const_reference x) const 
	{
		return &x;
	}
	pointer allocate(size_type size, FSTLAllocator<void>::const_pointer hint = 0) const 
	{
		return static_cast<pointer>(FLIB_ALLOC(sizeof(T) * size));
	}

	// For Dinkumware (VC6SP5):
	char* _Charalloc(size_type n) 
	{
		return static_cast<char*>(FLIB_ALLOC(n));
	}
	// end Dinkumware

	template <class U> FSTLAllocator(const FSTLAllocator<U>&) {}
	FSTLAllocator(const FSTLAllocator<T>&) {}
	void deallocate(pointer p, size_type n) const 
	{
		FLIB_FREE(p);
	}
	void deallocate(void* p, size_type n) const 
	{
		FLIB_FREE(p);
	}
	size_type max_size() const throw() 
	{
		return size_t(-1) / sizeof(value_type);
	}
	void construct(pointer p, const T& val) 
	{
		::new(static_cast<void*>(p)) T(val);
	}
	void destroy(pointer p) 
	{
		internal::destruct(p);
	}
private:
};

template <typename T, typename U>
FLIB_FORCEINLINE bool operator==(const FSTLAllocator<T>&, const FSTLAllocator<U>) 
{
	return true;
}

template <typename T, typename U>
FLIB_FORCEINLINE bool operator!=(const FSTLAllocator<T>&, const FSTLAllocator<U>) 
{
	return false;
}
_FStdEnd


#endif//__FALLOCTOR_HPP__
