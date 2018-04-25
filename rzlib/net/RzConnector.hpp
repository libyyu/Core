#ifndef _RZCONNECTOR_HPP__
#define _RZCONNECTOR_HPP__
#pragma once
#include <algorithm>  
#include <iostream>
#include <map>
#include <set>
#include <future>
#include <thread>
#include "RzSocket.hpp"
#include "RzFDSet.hpp"
#include "RzEventLoop.hpp"
_RzStdBegin
_RzNameSpaceBegin(Net)
class CRzConnector
{   
    typedef std::function<void(spSocketT)> COMPLETED_CALLBACK;
    typedef std::function<void()> FAILED_CALLBACK;
    class Rz_ConnectAddr
    {
    public:
        explicit Rz_ConnectAddr(const CRzSockAddr* pAddr, 
            std::chrono::nanoseconds timeout, 
            const CRzConnector::COMPLETED_CALLBACK& successCB,
            const CRzConnector::FAILED_CALLBACK& failedCB)
            :mAddr(pAddr)
            ,mTimeout(timeout)
            ,mSuccessCB(successCB)
            ,mFailedCB(failedCB)
        { }
        inline const CRzSockAddr* getAddr() const
        {
            return mAddr;
        }
        inline const CRzConnector::COMPLETED_CALLBACK&   getSuccessCB() const
        {
            return mSuccessCB;
        }

        inline const CRzConnector::FAILED_CALLBACK&  getFailedCB() const
        {
            return mFailedCB;
        }

        inline std::chrono::nanoseconds                getTimeout() const
        {
            return mTimeout;
        }
    private:
        const CRzSockAddr* mAddr;
        std::chrono::nanoseconds            mTimeout;
        CRzConnector::COMPLETED_CALLBACK  mSuccessCB;
        CRzConnector::FAILED_CALLBACK     mFailedCB;
    }; 
    class Rz_ConnectorWorkInfo
    {
        struct Rz_ConnectingInfo
        {
            Rz_ConnectingInfo()
            {
                timeout = std::chrono::nanoseconds::zero();
            }

            std::chrono::steady_clock::time_point   startConnectTime;
            std::chrono::nanoseconds                timeout;
            CRzConnector::COMPLETED_CALLBACK        successCB;
            CRzConnector::FAILED_CALLBACK           failedCB;
        };
        struct FDSetDeleter
        {
            inline void operator()(struct fdset_s* ptr) const
            {
                ox_fdset_delete(ptr);
            }
        };

        std::unique_ptr<struct fdset_s, FDSetDeleter> mFDSet;
        std::map<spSocketT, Rz_ConnectingInfo>  mConnectingInfos;
        std::set<spSocketT>                     mConnectingFds;
    public:
        inline void                checkConnectStatus(int millsecond);
        inline bool                isConnectSuccess(spSocketT pClientfd) const;
        inline void                checkTimeout();
        inline void                processConnect(const CRzConnector::Rz_ConnectAddr&);
        inline void                causeAllFailed();
        Rz_ConnectorWorkInfo()
        {
            mFDSet.reset(ox_fdset_new());
        }
    };
    typedef std::shared_ptr<Rz_ConnectorWorkInfo> spConnectorWorkInfo;
    typedef std::shared_ptr<std::thread> spThread;
    typedef std::shared_ptr<bool> spBool;

    std::shared_ptr<CRzEventLoop>      mEventLoop;
    spConnectorWorkInfo mWorkInfo;
    spThread    mThread;
    std::mutex  mThreadGuard;
    spBool   mIsRun;

    friend void runOnceCheckConnect(const std::shared_ptr<CRzEventLoop>& eventLoop,
        const std::shared_ptr<CRzConnector::Rz_ConnectorWorkInfo>& workerInfo);
public:
    CRzConnector()
    {
        mIsRun = std::make_shared<bool>(false);
    }

    ~CRzConnector()
    {
        stopWorkerThread();
        printf("Delete ...\n");
    }
    inline void                startWorkerThread();
    inline void                stopWorkerThread();
    inline void                asyncConnect(const CRzSockAddr* pAddr, 
                                        std::chrono::nanoseconds timeout,
                                        COMPLETED_CALLBACK, 
                                        FAILED_CALLBACK);
};
typedef std::shared_ptr<CRzConnector> spConnectorT;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void runOnceCheckConnect(const std::shared_ptr<CRzEventLoop>& eventLoop,
    const std::shared_ptr<CRzConnector::Rz_ConnectorWorkInfo>& workerInfo)
{
    eventLoop->loop(std::chrono::milliseconds(10).count());
    workerInfo->checkConnectStatus(0);
    workerInfo->checkTimeout();
}
inline void CRzConnector::startWorkerThread()
{
    std::lock_guard<std::mutex> lck(mThreadGuard);
    if (mThread != nullptr)
    {
        return;
    }
    mIsRun = std::make_shared<bool>(true);
    mWorkInfo = std::make_shared<Rz_ConnectorWorkInfo>();
    mEventLoop = std::make_shared<CRzEventLoop>();

    auto eventLoop = mEventLoop;
    auto workerInfo = mWorkInfo;
    auto isRun = mIsRun;

    mThread = std::make_shared<std::thread>([eventLoop, workerInfo, isRun](){
        while (*isRun)
        {
            runOnceCheckConnect(eventLoop, workerInfo);
        }

        workerInfo->causeAllFailed();
    });
}
inline void CRzConnector::stopWorkerThread()
{
    std::lock_guard<std::mutex> lck(mThreadGuard);
    if (mThread == nullptr)
    {
        return;
    }

    mEventLoop->pushAsyncProc([this]() {
        *mIsRun = false;
    });
    try
    {
        if (mThread->joinable())
        {
            mThread->join();
        }
    }
    catch(...)
    { }

    mEventLoop = nullptr;
    mWorkInfo = nullptr;
    mIsRun = nullptr;
    mThread = nullptr;
}
inline void CRzConnector::asyncConnect(const CRzSockAddr* pAddr, 
                                        std::chrono::nanoseconds timeout,
                                        CRzConnector::COMPLETED_CALLBACK successCB, 
                                        CRzConnector::FAILED_CALLBACK failedCB)
{
    std::lock_guard<std::mutex> lck(mThreadGuard);
    if (successCB == nullptr || failedCB == nullptr)
    {
        throw std::runtime_error("all callback is nullptr");
    }

    if (!(*mIsRun))
    {
        throw std::runtime_error("work thread already stop");
    }
    auto workInfo = mWorkInfo;
    auto address = Rz_ConnectAddr(pAddr,
        timeout,
        successCB,
        failedCB);

    mEventLoop->pushAsyncProc([workInfo, address]() {
        workInfo->processConnect(address);
    });
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool CRzConnector::Rz_ConnectorWorkInfo::isConnectSuccess(spSocketT pClientfd) const
{
    int fd = (*pClientfd);
    if (!ox_fdset_check(mFDSet.get(), fd, WriteCheck))
    {
        return false;
    }

    int error;
    int len = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t*)&len) == -1)
    {
        return false;
    }

    return error == 0;
}
inline void CRzConnector::Rz_ConnectorWorkInfo::checkConnectStatus(int millsecond)
{
    if (ox_fdset_poll(mFDSet.get(), millsecond) <= 0)
    {
        return;
    }

    std::set<spSocketT>  total_fds;
    std::set<spSocketT>  success_fds;

    for (auto& v : mConnectingFds)
    {
        int fd = (*v);
        if (ox_fdset_check(mFDSet.get(), fd, WriteCheck))
        {
            total_fds.insert(v);
            if (isConnectSuccess(v))
            {
                success_fds.insert(v);
            }
        }
    }

    for (auto v : total_fds)
    {
        int fd = (*v);
        ox_fdset_del(mFDSet.get(), fd, WriteCheck);
        mConnectingFds.erase(v);

        auto it = mConnectingInfos.find(v);
        if (it == mConnectingInfos.end())
        {
            continue;
        }

        if (success_fds.find(v) != success_fds.end())
        {
            if (it->second.successCB != nullptr)
            {
                it->second.successCB(std::move(v));
            }
        }
        else
        {
            if (it->second.failedCB != nullptr)
            {
                it->second.failedCB();
            }
        }
        
        mConnectingInfos.erase(it);
    }
}
inline void CRzConnector::Rz_ConnectorWorkInfo::checkTimeout()
{
    for (auto it = mConnectingInfos.begin(); it != mConnectingInfos.end();)
    {
        auto now = std::chrono::steady_clock::now();
        if ((now - it->second.startConnectTime) < it->second.timeout)
        {
            ++it;
            continue;
        }
        
        auto socket = it->first;
        auto cb = it->second.failedCB;

        int fd = *socket;
        ox_fdset_del(mFDSet.get(), fd, WriteCheck);

        mConnectingFds.erase(socket);
        mConnectingInfos.erase(it++);

        socket->Close();
        if (cb != nullptr)
        {
            cb();
        }
    }
}
inline void CRzConnector::Rz_ConnectorWorkInfo::causeAllFailed()
{
    for (auto it = mConnectingInfos.begin(); it != mConnectingInfos.end();)
    {
        auto socket = it->first;
        auto cb = it->second.failedCB;

        int fd = *socket;
        ox_fdset_del(mFDSet.get(), fd, WriteCheck);

        mConnectingFds.erase(socket);
        mConnectingInfos.erase(it++);

        socket->Close();
        if (cb != nullptr)
        {
            cb();
        }
    }
}

inline void CRzConnector::Rz_ConnectorWorkInfo::processConnect(const CRzConnector::Rz_ConnectAddr& addr)
{
    spSocketT pSock = spSocketT(new CRzSocket());
    Rz_ConnectorWorkInfo::Rz_ConnectingInfo ci;

#if PLATFORM_TARGET == PLATFORM_WINDOWS
    int check_error = WSAEWOULDBLOCK;
#else
    int check_error = EINPROGRESS;
#endif
    bool flag = false;
    unsigned long ul = 1; 
    RZ_SOCKET_STARTUP

    if (!pSock->Create())
    {
        goto FAILED;
    }
    
	pSock->Ioctl(FIONBIO, &ul);//设置阻塞模式<0为阻塞，1非阻塞>  

    flag = pSock->Connect(addr.getAddr());
    if (flag)
    {
        goto SUCCESS;
    }

    if (check_error != RZ_ERRNO)
    {
        pSock->Close();
        goto FAILED;
    }

    ci.startConnectTime = std::chrono::steady_clock::now();
    ci.successCB = addr.getSuccessCB();
    ci.failedCB = addr.getFailedCB();
    ci.timeout = addr.getTimeout();

    mConnectingInfos[pSock] = ci;
    mConnectingFds.insert(pSock);
    ox_fdset_add(mFDSet.get(), (*pSock), WriteCheck);
    return;

SUCCESS:
    if (addr.getSuccessCB() != nullptr)
    {
        addr.getSuccessCB()(pSock);
    }
    return;

FAILED:
    if (addr.getFailedCB() != nullptr)
    {
        addr.getFailedCB()();
    }
}

_RzStdEnd
_RzNameSpaceEnd

_RzStdBegin
_RzNameSpaceBegin(Net)
inline spSocketT SyncConnect(const char* ip,
            unsigned short port,
            unsigned int timeout,
            spConnectorT asyncConnector = nullptr)
{
    if (asyncConnector == nullptr)
    {
        asyncConnector = std::make_shared<CRzConnector>();
        asyncConnector->startWorkerThread();
    }

    CRzSockAddr addr(port, ip);

    auto pSocket = std::make_shared<std::promise<spSocketT>>();
    asyncConnector->asyncConnect(&addr,
        std::chrono::nanoseconds(timeout),
        [pSocket](spSocketT sock) {
            printf("OnConnect Success.\n");
            pSocket->set_value(std::move(sock));
        }, 
        [pSocket]() {
            printf("OnCloseed\n");
            pSocket->set_value(nullptr);
        }
    );

    return pSocket->get_future().get();
}
_RzNameSpaceEnd
_RzStdEnd


#endif//_RZCONNECTOR_HPP__