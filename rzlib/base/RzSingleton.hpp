#ifndef __RZSINGLETON_HPP__
#define __RZSINGLETON_HPP__
#pragma once
#include "RzLock.hpp"

_RzStdBegin
template <class T>
class CRzSingleton
{
public:
	static inline T* Instance();

	CRzSingleton(void) {}
	~CRzSingleton(void){ /*if( 0 != _instance.get() ) _instance.reset();*/ }
private:
	CRzSingleton(const CRzSingleton& ) {}
	CRzSingleton& operator= (const CRzSingleton& ) {}
protected:
	static auto_ptr<T> _instance;
	static CRzLock _lock;
};

template <class T>
auto_ptr<T> CRzSingleton<T>::_instance;

template <class T>
CRzLock CRzSingleton<T>::_lock;


template <class T>
inline T* CRzSingleton<T>::Instance()
{
	if( 0 == _instance.get() )
	{
		lock_wrapper gd(&_lock);
		if( 0 == _instance.get())
		{
			_instance.reset ( new T);
		}
	}
	return _instance.get();
}

#define DECLARE_SINGLETON_CLASS( type ) \
	friend class auto_ptr< type >;\
	friend class CRzSingleton< type >;

_RzStdEnd

#endif//__RZSINGLETON_HPP__