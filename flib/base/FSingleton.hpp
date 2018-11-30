#ifndef __FSINGLETON_HPP__
#define __FSINGLETON_HPP__
#pragma once
#include "FLock.hpp"
#include <memory>

_FStdBegin
template <class T>
class FSingleton
{
public:
	static inline T* Instance();

	FSingleton(void) {}
	~FSingleton(void){ /*if( 0 != _instance.get() ) _instance.reset();*/ }
private:
	FSingleton(const FSingleton& ) {}
	FSingleton& operator= (const FSingleton& ) {}
protected:
	static std::auto_ptr<T> _instance;
	static FLock _lock;
};

template <class T>
std::auto_ptr<T> FSingleton<T>::_instance;

template <class T>
FLock FSingleton<T>::_lock;


template <class T>
inline T* FSingleton<T>::Instance()
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
	friend class FSingleton< type >;

_FStdEnd

#endif//__FSINGLETON_HPP__