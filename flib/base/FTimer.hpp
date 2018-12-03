#ifndef _FTIMER_HPP__
#define _FTIMER_HPP__
#pragma once
#include "FType.hpp"
#include <functional>
#include <queue>
#include <memory>
#include <chrono>

_FStdBegin

class FTimerMgr
{
    class FTimer
    {
    public:
        typedef std::shared_ptr<FTimer>          spFTimer;
        typedef std::weak_ptr<FTimer>            wpFTimer;
        typedef std::function<void(void)>       Callback;

        FTimer(std::chrono::steady_clock::time_point startTime, 
            std::chrono::nanoseconds lastTime, 
            Callback f):
            mStartTime(std::move(startTime)),
            mLastTime(std::move(lastTime)),
            mCallback(std::move(f)),
            mActive(true)
        {    
        }

        inline const std::chrono::steady_clock::time_point&    getStartTime() const
        {
            return mStartTime;
        }
        inline const std::chrono::nanoseconds&         getLastTime() const
        {
            return mLastTime;
        }

        inline std::chrono::nanoseconds                getLeftTime() const
        {
            return getLastTime() - (std::chrono::steady_clock::now() - getStartTime());
        }
        inline void                                    cancel()
        {
            mActive = false;
        }

    private:
        inline void operator()                         ()
        {
            if (mActive)
            {
                mCallback();
            }
        }

    private:
        bool                                    mActive;
        Callback                                mCallback;
        const std::chrono::steady_clock::time_point mStartTime;
        std::chrono::nanoseconds                mLastTime;

        friend class FTimerMgr;
    };
public:
    template<typename F, typename ...TArgs>
    inline FTimer::wpFTimer  addTimer(std::chrono::nanoseconds timeout, 
            F callback, 
            TArgs&& ...args)
    {
        auto timer = std::make_shared<FTimer>(std::chrono::steady_clock::now(),
                                            std::chrono::nanoseconds(timeout),
                                            std::bind(std::move(callback), std::forward<TArgs>(args)...));
        mTimers.push(timer);

        return timer;
    }

    inline void                                    schedule()
    {
        while (!mTimers.empty())
        {
            auto tmp = mTimers.top();
            if (tmp->getLeftTime() > std::chrono::nanoseconds::zero())
            {
                break;
            }

            mTimers.pop();
            (*tmp)();
        }
    }
    inline bool                                    isEmpty() const
    {
        return mTimers.empty();
    }
    // if timer empty, return zero
    inline std::chrono::nanoseconds                nearLeftTime() const
    {
        if (mTimers.empty())
        {
            return std::chrono::nanoseconds::zero();
        }

        auto result = mTimers.top()->getLeftTime();
        if (result < std::chrono::nanoseconds::zero())
        {
            return std::chrono::nanoseconds::zero();
        }

        return result;
    }
    inline void                                    clear()
    {
        while (!mTimers.empty())
        {
            mTimers.pop();
        }
    }

private:
    class _F_CompareTimer
    {
    public:
        inline bool operator() (const FTimer::spFTimer& left, const FTimer::spFTimer& right) const
        {
            auto startDiff = left->getStartTime() - right->getStartTime();
            auto lastDiff = left->getLastTime() - right->getLastTime();
            auto diff = startDiff.count() + lastDiff.count();
            return diff > 0;
        }
    };

    std::priority_queue<FTimer::spFTimer, std::vector<FTimer::spFTimer>, _F_CompareTimer>  mTimers;
};
_FStdEnd

#endif//_FTIMER_HPP__