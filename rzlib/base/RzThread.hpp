#ifndef __RZTHREAD_HPP__
#define __RZTHREAD_HPP__
#pragma once
#include <vector>
#include <queue>
#include "RzLock.hpp"
#include "RzSemaphore.hpp"
#include <functional>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	#include <Windows.h>
#else
	#include <pthread.h>
#endif
_RzStdBegin

class CRzThread
{
public:
	typedef std::function<void()> RzThreadCallback;
    enum RzStateT { kInit, kStart, kJoined, kStop };

    explicit CRzThread(const RzThreadCallback &cb)
        : _cb(cb)
        , _state(kInit)
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        , _handle(NULL)
        , _threadId(0)
#endif
    {}

    ~CRzThread()
    {
        join();
        _state = kStop;
    }

    bool start()
    {
        if (kInit != _state) 
        {
            return false;
        }

        bool result = false;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        _handle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProc, (LPVOID)this, 0, &_threadId);
        result = (NULL != _handle);
#else
        int ret = pthread_create(&_thread, NULL, threadProc, (void *)this);
        result = (0 == ret);
#endif
        _state = kStart;
        return result;
    }
    
    bool stop()
    {
        if (kStop == _state || kInit == _state) 
        {
            return true;
        }

        bool result = true;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        if(0 == ::TerminateThread(_handle, 0)) 
        {
            result = false;
        }
#else
        if (0 != pthread_cancel(_thread)) 
        {
            result = false;
        }
#endif
        if (result) 
        {
            _state = kStop;
        }

        return result;
    }

    bool join()
    {
        if (kStart != _state) 
        {
            return false;
        }
        bool result = false;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        if (NULL != _handle) 
        {
            DWORD ret = ::WaitForSingleObject(_handle, INFINITE);
            if (WAIT_OBJECT_0 == ret || WAIT_ABANDONED == ret) 
            {
                result = true;
                _handle = NULL;
            }
        }
#else
        int ret = pthread_join(_thread, NULL);
        if (0 == ret) 
        {
            result = true;
        }
#endif
        _state = kJoined;
        return result;
    }

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    DWORD tid() const { return _threadId; }
    operator HANDLE() { return _handle; }
#else
    pthread_t tid() const { return _thread; }
#endif

private:
    CRzThread(const CRzThread&){}
    void operator=(const CRzThread&){}

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    static DWORD WINAPI threadProc(LPVOID param)
#else
    static void *threadProc(void *param)
#endif
    {
        CRzThread *pThis = reinterpret_cast<CRzThread *>(param);
        pThis->_cb();
        return 0;
    }

    RzThreadCallback _cb;
    RzStateT _state;

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    HANDLE _handle;
    DWORD _threadId;
#else
    pthread_t _thread;
#endif
};

class CRzThreadGroup
{
public:
    CRzThreadGroup()
    {}
    ~CRzThreadGroup()
    {
        joinAll();
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            delete _threads[i];
        }
        
        _threads.clear();
    }
    
    CRzThread *createThread(const CRzThread::RzThreadCallback &threadfunc)
    {
        CRzThread *thread = new CRzThread(threadfunc);
        addThread(thread);
        return thread;
    }
    
    void addThread(CRzThread *thread) 
    {
    	_lock.lock();
        _threads.push_back(thread);
        _lock.unlock();
    }
    
    void startAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]->start();
        }
    }
    
    void joinAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]->join();
        }
    }
    
    void stopAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]->stop();
        }
    }
    
    size_t size() const { return _threads.size(); }  
private:
    CRzThreadGroup(const CRzThreadGroup&){}
    void operator=(const CRzThreadGroup&){}

    std::vector<CRzThread *> _threads;
    CRzLock _lock;
};


class CRzThreadPool
{
public:
	CRzThreadPool()
		: _inited(false)
	{}
	~CRzThreadPool()
	{}
	
	bool init(int threadNum = kDefaultThreadNum)
	{
		if (_inited) 
		{
			return false;
		}
		_inited = true;
		addWorker(threadNum);
		return _inited;
	}
	
	void addTask(const std::function<void()> &task)
	{ 
		_tasks.put(task); 
	}
	
	void join()
	{
		for (_RzThreadVector::const_iterator it = _threads.begin(); it != _threads.end(); ++it) 
		{
			(*it)->join();
		}
			
		_threads.clear();
	}
	
	size_t size() const { return _threads.size(); }
	
	void terminate()
	{
		for (_RzThreadVector::const_iterator it = _threads.begin(); it != _threads.end(); ++it) 
		{
			(*it)->stop();
		}
	}

private:
	CRzThreadPool(const CRzThreadPool&){}
    void operator=(const CRzThreadPool&){}
	typedef std::vector<std::shared_ptr<CRzThread>> _RzThreadVector;
	
	class _RzTaskQueue
	{
	public:
		_RzTaskQueue() {}
		~_RzTaskQueue() {}
		
		void put(const std::function<void()> &task) 
		{
			lock_wrapper lock(&_mutex);
			_tasks.push(task);
			_sem.signal();
		}
		CRzThread::RzThreadCallback get()
		{
			std::function<void()> task;
			_sem.wait(CRzSemaphore::kInfinite);
			lock_wrapper lock(&_mutex);
			task = _tasks.front();
			_tasks.pop();
			return task;
		}
		
	private:
		_RzTaskQueue(const _RzTaskQueue&){}
    	void operator=(const _RzTaskQueue&){}
		typedef std::queue<std::function<void()>> _RzTasks;
		_RzTasks _tasks;
		CRzLock _mutex;
		CRzSemaphore _sem;
	};
	
	// 添加工作线程
	void addWorker(int threadNum)
	{
		_lock.lock();
		for (int i = 0; i < threadNum; ++i) 
		{
			std::shared_ptr<CRzThread> thread(new CRzThread(std::bind(&CRzThreadPool::taskRunner, this)));
			_threads.push_back(thread);					
			thread->start();
		}
		_lock.unlock();
	}
	
	// 运行任务
	void taskRunner()
	{
		while(true) 
		{
			std::function<void()> task = _tasks.get();
			task();
		}
	}
	
	bool _inited;
	_RzThreadVector _threads;
	_RzTaskQueue _tasks;
	CRzLock _lock;

	static const int32_t kDefaultThreadNum = 4;
};


_RzStdEnd


#endif//__RZTHREAD_HPP__