#ifndef __FEVENTLOOP_HPP__
#define __FEVENTLOOP_HPP__
#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include "../base/FTimer.hpp"
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
#include <windows.h>
#else
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#if FLIB_COMPILER_MACOSX
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif
#endif
_FStdBegin
_FNameSpaceBegin(Net)
class FEventLoop
{
public:
    typedef std::function<void(void)>           USER_PROC;
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    enum class OLV_VALUE
    {
        OVL_NONE = 0,
        OVL_RECV,
        OVL_SEND,
    };

    struct ovl_ext_s
    {
        OVERLAPPED  base;
        const FEventLoop::OLV_VALUE  OP;

        ovl_ext_s(OLV_VALUE op) : OP(op)
        {
            memset(&base, 0, sizeof(base));
        }
    };

    class _F_WakeupChannel
    {
        friend class FEventLoop;
        HANDLE       mIOCP;
        ovl_ext_s    mWakeupOvl;
    public:
        explicit _F_WakeupChannel(HANDLE iocp) : mIOCP(iocp), mWakeupOvl(OLV_VALUE::OVL_RECV)
        {
        }

        inline void    wakeup()
        {
            PostQueuedCompletionStatus(mIOCP, 0, (ULONG_PTR)this, (OVERLAPPED*)&mWakeupOvl);
        }
    private:
        inline void    canSend()
        {

        }
        inline void    canRecv()
        {

        }
        inline void    onClose()
        {

        }
    };
#else
    class _F_WakeupChannel
    {
        friend class FEventLoop;
        int mFd;
    public:
        explicit _F_WakeupChannel(int fd) : mFd(fd)
        {
        }
        ~_F_WakeupChannel()
        {
            close(mFd);
            mFd = -1;
        }
        inline void    wakeup()
        {
            /*uint64_t one = 1;
            if (write(mFd, &one, sizeof one) <= 0)
            {
                std::cerr << "wakeup failed" << std::endl;
            }*/
        }
    private:
        inline void    canSend()
        {

        }
        inline void    canRecv()
        {
            /*char temp[1024 * 10];
            while (true)
            {
                auto n = read(mFd, temp, sizeof(temp));
                if (n == -1 || n < sizeof(temp))
                {
                    break;
                }
            }*/
        }
        inline void    onClose()
        {

        }
    };
#endif
public:
    FEventLoop()
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    : mIOCP(CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1))
#elif FLIB_COMPILER_MACOSX
    : mEpollFd(kqueue())
#else
    : mEpollFd(epoll_create(1))
#endif
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        mPGetQueuedCompletionStatusEx = NULL;
        auto kernel32_module = GetModuleHandleA("kernel32.dll");
        if (kernel32_module != NULL) {
            mPGetQueuedCompletionStatusEx = (sGetQueuedCompletionStatusEx)GetProcAddress(
                kernel32_module,
                "GetQueuedCompletionStatusEx");
            FreeLibrary(kernel32_module);
        }
#elif FLIB_COMPILER_MACOSX
        auto eventfd = kqueue();
        mWakeupChannel.reset(new _F_WakeupChannel(eventfd));
        linkChannel(eventfd, mWakeupChannel.get());
#else
        auto eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        mWakeupChannel.reset(new _F_WakeupChannel(eventfd));
        linkChannel(eventfd, mWakeupChannel.get());
#endif
        mIsAlreadyPostWakeup = false;
        mIsInBlock = true;

        mEventEntries = nullptr;
        mEventEntriesNum = 0;

        reallocEventSize(1024);
        mSelfThreadID = -1;
        mTimer = std::make_shared<FTimerMgr>();
  
    }
    virtual ~FEventLoop()
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        CloseHandle(mIOCP);
        mIOCP = INVALID_HANDLE_VALUE;
#else
        close(mEpollFd);
        mEpollFd = -1;
#endif
        delete[] mEventEntries;
        mEventEntries = nullptr;
        mEventEntriesNum = 0;
    }
public:
    inline void                            loop(int64_t milliseconds)
    {
        tryInitThreadID();

#ifndef NDEBUG
        assert(isInLoopThread());
#endif
        if (!isInLoopThread())
        {
            return;
        }

        if (!mAfterLoopProcs.empty())
        {
            milliseconds = 0;
        }
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        ULONG numComplete = 0;
        if (mPGetQueuedCompletionStatusEx != nullptr)
        {
            if (!mPGetQueuedCompletionStatusEx(mIOCP, 
                                            mEventEntries, 
                                            static_cast<ULONG>(mEventEntriesNum), 
                                            &numComplete, 
                                            static_cast<DWORD>(milliseconds), 
                                            false))
            {
                numComplete = 0;
            }
        }
        else
        {
            do
            {
                /* don't check the return value of GQCS */
                GetQueuedCompletionStatus(mIOCP,
                    &mEventEntries[numComplete].dwNumberOfBytesTransferred,
                    &mEventEntries[numComplete].lpCompletionKey,
                    &mEventEntries[numComplete].lpOverlapped,
                    (numComplete == 0) ? static_cast<DWORD>(milliseconds) : 0);
            } while (mEventEntries[numComplete].lpOverlapped != nullptr && ++numComplete < mEventEntriesNum);
        }

        mIsInBlock = false;

        for (ULONG i = 0; i < numComplete; ++i)
        {
            auto channel = (_F_WakeupChannel*)mEventEntries[i].lpCompletionKey;
            const auto ovl = (ovl_ext_s*)mEventEntries[i].lpOverlapped;
            if (ovl->OP == OLV_VALUE::OVL_RECV)
            {
                channel->canRecv();
            }
            else if (ovl->OP == OLV_VALUE::OVL_SEND)
            {
                channel->canSend();
            }
            else
            {
                assert(false);
            }
        }
#elif FLIB_COMPILER_MACOSX
        struct timespec timeout;
        timeout.tv_sec = milliseconds / 1000;
        timeout.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
        int numComplete = kevent(mEpollFd, NULL, 0, (struct kevent *)mEventEntries, mEventEntriesNum, &timeout);
        for(int i=0; i< numComplete; ++i)
        {
            auto    channel = (_F_WakeupChannel*)(mEventEntries[i].udata);
            int events = mEventEntries[i].filter;
            if(mEventEntries[i].flags & EV_ERROR)
            {
                channel->canRecv();
                channel->onClose();
                continue;
            }
            if(events == EVFILT_READ)
            {
                channel->canRecv();
            }

            else if (events == EVFILT_WRITE)
            {
                channel->canSend();
            }
            else
            {
                assert(false);
            }
        }
#else
        int numComplete = epoll_wait(mEpollFd, mEventEntries, mEventEntriesNum, milliseconds);

        mIsInBlock = false;

        for (int i = 0; i < numComplete; ++i)
        {
            auto    channel = (_F_WakeupChannel*)(mEventEntries[i].data.ptr);
            auto    event_data = mEventEntries[i].events;

            if (event_data & EPOLLRDHUP)
            {
                channel->canRecv();
                channel->onClose();
                continue;
            }

            if (event_data & EPOLLIN)
            {
                channel->canRecv();
            }

            if (event_data & EPOLLOUT)
            {
                channel->canSend();
            }
        }
#endif
        mIsAlreadyPostWakeup = false;
        mIsInBlock = true;

        processAsyncProcs();
        processAfterLoopProcs();

        if (numComplete == mEventEntriesNum)
        {
            reallocEventSize(mEventEntriesNum + 128);
        }

        mTimer->schedule();
    }
    inline bool                            wakeup()
    {
        if (!isInLoopThread() && mIsInBlock && !mIsAlreadyPostWakeup.exchange(true))
        {
            mWakeupChannel->wakeup();
            return true;
        }

        return false;
    }
    inline void                            pushAsyncProc(USER_PROC f)
    {
        if (isInLoopThread())
        {
            f();
        }
        else
        {
            {
                std::lock_guard<std::mutex> lck(mAsyncProcsMutex);
                mAsyncProcs.emplace_back(std::move(f));
            }
            wakeup();
        }
    }
    inline void                            pushAfterLoopProc(USER_PROC f)
    {
        assert(isInLoopThread());
        if (isInLoopThread())
        {
            mAfterLoopProcs.emplace_back(std::move(f));
        }
    }
    inline std::shared_ptr<FTimerMgr>    getTimerMgr()
    {
        tryInitThreadID();
        assert(isInLoopThread());
        return isInLoopThread() ? mTimer : nullptr;
    }

    inline bool                     isInLoopThread() const
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        return mSelfThreadID == GetCurrentThreadId();
#else
        uint32 id = static_cast<uint32>(reinterpret_cast<uintptr_t>(pthread_self()));
        return mSelfThreadID == id;
#endif
    }
private:
    inline void                            reallocEventSize(size_t size)
    {
        if (mEventEntries != nullptr)
        {
            delete[] mEventEntries;
            mEventEntries = nullptr;
        }

#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        mEventEntries = new OVERLAPPED_ENTRY[size];
#elif FLIB_COMPILER_MACOSX
        mEventEntries = new struct kevent[size];
#else
        mEventEntries = new epoll_event[size];
#endif

        mEventEntriesNum = size;
    }
    inline void                            processAfterLoopProcs()
    {
        mCopyAfterLoopProcs.swap(mAfterLoopProcs);
        for (const auto& x : mCopyAfterLoopProcs)
        {
            x();
        }
        mCopyAfterLoopProcs.clear();
    }
    inline void                            processAsyncProcs()
    {
        {
            std::lock_guard<std::mutex> lck(mAsyncProcsMutex);
            mCopyAsyncProcs.swap(mAsyncProcs);
        }

        for (const auto& x : mCopyAsyncProcs)
        {
            x();
        }
        mCopyAsyncProcs.clear();
    }
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    inline int                             getEpollHandle() const
    {
        return mEpollFd;
    }
#endif
    inline bool                            linkChannel(int fd, _F_WakeupChannel* ptr)
    {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        return CreateIoCompletionPort((HANDLE)fd, mIOCP, (ULONG_PTR)ptr, 0) != nullptr;
#elif FLIB_COMPILER_MACOSX
        const int kReadEvent = 1;
        const int kWriteEvent = 2;
        struct kevent ev[2];
        int n = 0;
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)ptr);
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)ptr);
        return 0 == kevent(mEpollFd, ev, n, NULL, 0, NULL);
#else
        struct epoll_event ev = { 0, { 0 } };
        ev.events = EPOLLET | EPOLLIN | EPOLLRDHUP;
        ev.data.ptr = ptr;
        return epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
#endif
    }
    inline void                            tryInitThreadID()
    {
        std::call_once(mOnceInitThreadID, [this]() 
        {
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
            mSelfThreadID = GetCurrentThreadId();
#else
            mSelfThreadID = static_cast<uint32>(reinterpret_cast<uintptr_t>(pthread_self()));
#endif
        });
    }
private:
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    OVERLAPPED_ENTRY*               mEventEntries;
    typedef BOOL(WINAPI *sGetQueuedCompletionStatusEx) (HANDLE, LPOVERLAPPED_ENTRY, ULONG, PULONG, DWORD, BOOL);
    sGetQueuedCompletionStatusEx    mPGetQueuedCompletionStatusEx;
    HANDLE                          mIOCP;
#elif FLIB_COMPILER_MACOSX
    struct kevent*                  mEventEntries;
    int                             mEpollFd;
#else
    epoll_event*                    mEventEntries;
    int                             mEpollFd;
#endif
    size_t                          mEventEntriesNum;
    std::unique_ptr<_F_WakeupChannel>  mWakeupChannel;

    std::atomic_bool                mIsInBlock;
    std::atomic_bool                mIsAlreadyPostWakeup;

    std::mutex                      mAsyncProcsMutex;
    std::vector<USER_PROC>          mAsyncProcs;                
    std::vector<USER_PROC>          mCopyAsyncProcs;

    std::vector<USER_PROC>          mAfterLoopProcs;
    std::vector<USER_PROC>          mCopyAfterLoopProcs;

    std::once_flag                  mOnceInitThreadID;
    uint32                             mSelfThreadID;
    std::shared_ptr<FTimerMgr>    mTimer;
};
_FNameSpaceEnd
_FStdEnd

#endif//__FEVENTLOOP_HPP__
