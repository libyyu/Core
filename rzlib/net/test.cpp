//#include "RzSocket.hpp"
#include <iostream>
#include <functional>
#include "../base/RzFunc.hpp"
#include "../base/RzThread.hpp"
using namespace RzStd;
class Async
{
public:
    typedef std::function<void()> AsyncCallback;
    typedef std::function<void(void*)> AsyncCallbackArgs;
    explicit Async(const AsyncCallback& action) : _thread(action)
    {
        _thread.start();
    }
    explicit Async(const AsyncCallbackArgs& action, void* sender) : _thread(action)
    {
        _thread.start(sender);
    }
    ~Async()
    {
        _thread.join();
        delete this;
    }
protected:
    CRzThread _thread;
};

static CRzLock g_lock;

class TestAsync
{
public:
    void DoA(const char* name)
    {
        printf("before %p\n", this);
        new Async([=](void* sender)
        {
            for(int i=0;i<10;++i)
            {
                g_lock.lock();
                std::cout<<i << "TestAsync.DoA run in async thread " << RzGetCurrentThreadId() << std::endl;
                g_lock.unlock();
            }
            printf("inner %p, %s\n", sender, name);
           
            TestAsync *pThis = reinterpret_cast<TestAsync *>(sender);
            pThis->OnADo();
        }, (void*)this);
    }
protected:
    void OnADo()
    {
        g_lock.lock();
        std::cout << "OnADo" << std::endl;
        g_lock.unlock();
    }
};

int main()
{
    TestAsync test;
    test.DoA("hello");
    new RzAsync([]
    {
        for(int i=0;i<10;++i)
        {
            g_lock.lock();
            std::cout<<i << "main: run in async thread " << RzGetCurrentThreadId() << std::endl;
            g_lock.unlock();
        }
    });
    char tmp;
    while(1)
    {
        g_lock.lock();
        std::cout << "main thread test net " << tmp << RzGetCurrentThreadId() << std::endl;
        g_lock.unlock();
        scanf("%c", &tmp);
        if ('q' == tmp)
        {
            break;
        }   
    }
    return 0;
}