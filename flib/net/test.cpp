#include "FSocket.hpp"
#include <iostream>
#include <functional>
#include "../base/FFunc.hpp"
#include "../base/FThread.hpp"
#include "FConnector.hpp"
using namespace FStd;

static FLock g_lock;

class TestAsync
{
public: 
    int value;
    static const int32_t kDefaultThreadNum = 4;
    TestAsync():value(5){}
    void DoA(const char* name)
    {
        printf("before %p\n", this);
        new FAsync([=]
        {
            for(int i=0;i<10;++i)
            {
                g_lock.lock();
                std::cout<<i << "TestAsync.DoA run in async thread " << FGetCurrentThreadId() << std::endl;
                g_lock.unlock();
            }
            printf("inner %s\n", name);
        });

        AsyncCallback(std::bind(&TestAsync::DoB, this))();
        auto func = std::bind(&TestAsync::DoC, this, (void*)this);
        AsyncCallback(func)();
    }
    void DoB()
    {
        for(int i=0;i<10;++i)
        {
            g_lock.lock();
            std::cout<<i << "TestAsync.DoB run in async thread " << FGetCurrentThreadId() << std::endl;
            g_lock.unlock();
        }
    }
    void DoC(void* arg)
    {
        TestAsync *pThis = reinterpret_cast<TestAsync *>(arg);
        g_lock.lock();
        printf("inner DoC %p, %d\n", pThis, pThis->value);
        std::cout << ">>>>>>>>>>>>>>>>DoC " << arg << "," << pThis->kDefaultThreadNum << std::endl;
        g_lock.unlock();
        for(int i=0;i<10;++i)
        {
            g_lock.lock();
            std::cout<<i << "TestAsync.DoC run in async thread " << FGetCurrentThreadId() << std::endl;
            g_lock.unlock();
        }
    }
protected:
    void OnADo()
    {
        g_lock.lock();
        std::cout << "OnADo" << std::endl;
        g_lock.unlock();
    }
};

void TestSock()
{
    // FSockAddr addr(8002);
    // FSocket client;
    // client.Create();
    // bool b = client.Connect(&addr);
    // printf("Connect b = %d\n", b);
    int timeout = 10;
    auto socket = Net::SyncConnect(NULL, 8002, timeout);
}

int main()
{
    /*TestAsync test;
    test.DoA("hello");
    new FAsync([]
    {
        for(int i=0;i<10;++i)
        {
            g_lock.lock();
            std::cout<<i << "main: run in async thread " << FGetCurrentThreadId() << std::endl;
            g_lock.unlock();
        }
    });
    AsyncCallback(&TestSock)();
    
    
    char tmp;
    while(1)
    {
        g_lock.lock();
        std::cout << "main thread test net " << tmp << FGetCurrentThreadId() << std::endl;
        g_lock.unlock();
        scanf("%c", &tmp);
        if ('q' == tmp)
        {
            break;
        }   
    }*/
    TestSock();
    return 0;
}