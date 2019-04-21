#ifndef __FLOCK_HPP__
#define __FLOCK_HPP__
#pragma once
#include "FType.hpp"
#include <time.h>
#include <sstream>
#include <iostream>
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#endif

_FStdBegin
class i_lock
{
public:
    i_lock() {}
    ~i_lock() {}
    virtual bool lock() = 0;
    virtual bool unlock() = 0;
    virtual int lock_ex() { return 0; }
    virtual int unlock_ex() { return 0; }
};
class lock_wrapper
{
public:
    lock_wrapper(i_lock* p_lock)
    {
        mp_lock = p_lock;
        if (mp_lock != NULL)
            mp_lock->lock();
    }
    ~lock_wrapper() { unlock(); }
    void unlock()
    {
        if (mp_lock != NULL)
        {
            mp_lock->unlock();
            mp_lock = NULL;
        }
    }
private:
    i_lock* mp_lock;
};
_FStdEnd

_FStdBegin
class FLock : public i_lock
{
public:
    FLock()
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        InitializeCriticalSection(&cs);
#else
        pthread_mutexattr_t   mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m,&mta);
#endif
    }
    ~FLock()
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        DeleteCriticalSection(&cs);
#else
        pthread_mutex_destroy(&m);
#endif
    }
    bool lock()
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        EnterCriticalSection(&cs);
#else
        pthread_mutex_lock(&m);
#endif
        return true;
    }
    bool unlock()
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_unlock(&m);
#endif
        return false;
    }
protected:
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t m;
#endif
};
_FStdEnd

#endif//__FLOCK_HPP__
