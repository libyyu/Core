#ifndef __FTHREAD_HPP__
#define __FTHREAD_HPP__
#pragma once
#include "FLock.hpp"
#include "FSemaphore.hpp"
#include <queue>
#include <functional>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	#include <Windows.h>
#else
	#include <pthread.h>
#endif
_FStdBegin

class FThread
{
public:
    typedef std::function<void()> FThreadCallback;
    enum FStateT { kInit, kStart, kJoined, kStop };
    explicit FThread(const FThreadCallback &cb)
        : _cb(cb)
        , _state(kInit)
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        , _handle(NULL)
        , _threadId(0)
#endif
    {}

    ~FThread()
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
    FThread(const FThread&){}
    void operator=(const FThread&){}

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    static DWORD WINAPI threadProc(LPVOID param)
#else
    static void *threadProc(void *param)
#endif
    {
        FThread *pThis = reinterpret_cast<FThread *>(param);
        pThis->_cb();
        return 0;
    }

    FThreadCallback _cb;
    FStateT _state;

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    HANDLE _handle;
    DWORD _threadId;
#else
    pthread_t _thread;
#endif
};

class FThreadGroup
{
    struct _FThreadNode
    {
        FThread* _pThread;
        bool _isHeap;
    };
public:
    FThreadGroup()
    {}
    ~FThreadGroup()
    {
        joinAll();
        dispose();
    }

    void dispose()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            if(_threads[i]._isHeap)
                delete _threads[i]._pThread;
        }     
        _threads.clear();
    }
    
    FThread *createThread(const FThread::FThreadCallback &threadfunc)
    {
        FThread *thread = new FThread(threadfunc);
        addThread(thread, true);
        return thread;
    }
    
    void addThread(FThread *thread, bool isHeap = false) 
    {
        _FThreadNode node;
        node._isHeap = isHeap;
        node._pThread = thread;
    	_lock.lock();
        _threads.push_back(node);
        _lock.unlock();
    }
    
    void startAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]._pThread->start();
        }
    }
    
    void joinAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]._pThread->join();
        }
    }
    
    void stopAll()
    {
        for (size_t i = 0; i < _threads.size(); ++i) 
        {
            _threads[i]._pThread->stop();
        }
    }
    
    size_t size() const { return _threads.size(); }  
private:
    FThreadGroup(const FThreadGroup&){}
    void operator=(const FThreadGroup&){}

    std::vector<_FThreadNode> _threads;
    FLock _lock;
};


class FThreadPool
{
public:
	FThreadPool()
		: _inited(false)
	{}
	~FThreadPool()
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
	
	void addTask(const FThread::FThreadCallback &task)
	{ 
		_tasks.put(task); 
	}
	
	void join()
	{
		for (_FThreadVector::const_iterator it = _threads.begin(); it != _threads.end(); ++it) 
		{
			(*it)->join();
		}
			
		_threads.clear();
	}
	
	size_t size() const { return _threads.size(); }
	
	void terminate()
	{
		for (_FThreadVector::const_iterator it = _threads.begin(); it != _threads.end(); ++it) 
		{
			(*it)->stop();
		}
	}

private:
	FThreadPool(const FThreadPool&){}
    void operator=(const FThreadPool&){}
	typedef std::vector<std::shared_ptr<FThread> > _FThreadVector;
	
	class _FTaskQueue
	{
	public:
		_FTaskQueue() {}
		~_FTaskQueue() {}
		
		void put(const FThread::FThreadCallback &task) 
		{
			lock_wrapper lock(&_mutex);
			_tasks.push(task);
			_sem.signal();
		}
		FThread::FThreadCallback get()
		{
			FThread::FThreadCallback task;
			_sem.wait(FSemaphore::kInfinite);
			lock_wrapper lock(&_mutex);
			task = _tasks.front();
			_tasks.pop();
			return task;
		}
		
	private:
		_FTaskQueue(const _FTaskQueue&){}
    	void operator=(const _FTaskQueue&){}
		typedef std::queue<FThread::FThreadCallback> _FTasks;
		_FTasks _tasks;
		FLock _mutex;
		FSemaphore _sem;
	};
	
	// 添加工作线程
	void addWorker(int threadNum)
	{
		_lock.lock();
		for (int i = 0; i < threadNum; ++i) 
		{
            FThread::FThreadCallback _run = std::bind(&FThreadPool::taskRunner, this);
			std::shared_ptr<FThread> thread(new FThread(_run));
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
			FThread::FThreadCallback task = _tasks.get();
			task();
		}
	}
	
	bool _inited;
	_FThreadVector _threads;
	_FTaskQueue _tasks;
	FLock _lock;

	static const int32_t kDefaultThreadNum = 4;
};

class FAsync
{
public:
    explicit FAsync(const std::function<void()>& action) : _thread(action)
    {
        _thread.start();
    }
    ~FAsync()
    {
        _thread.join();
        delete this;
    }
protected:
    FThread _thread;
};
inline std::function<void()> AsyncCallback(const std::function<void()>& action)
{
    auto func = [action]()
    {
        new FAsync(action);
    };
    return func;
}
_FStdEnd


#endif//__FTHREAD_HPP__