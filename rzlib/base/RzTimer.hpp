#ifndef _RZTIMER_HPP__
#define _RZTIMER_HPP__
#pragma once
#include "RzType.hpp"
#include <functional>
#include <queue>
#include <memory>
#include <vector>
#include <chrono>

_RzStdBegin

class CRzTimerMgr
{
    class RzTimer
    {
    public:
        typedef std::shared_ptr<RzTimer>          spRzTimer;
        typedef std::weak_ptr<RzTimer>            wpRzTimer;
        typedef std::function<void(void)>       Callback;

        RzTimer(std::chrono::steady_clock::time_point startTime, 
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

        friend class CRzTimerMgr;
    };
public:
    template<typename F, typename ...TArgs>
    inline RzTimer::wpRzTimer  addTimer(std::chrono::nanoseconds timeout, 
            F callback, 
            TArgs&& ...args)
    {
        auto timer = std::make_shared<RzTimer>(std::chrono::steady_clock::now(),
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
    class _Rz_CompareTimer
    {
    public:
        inline bool operator() (const RzTimer::spRzTimer& left, const RzTimer::spRzTimer& right) const
        {
            auto startDiff = left->getStartTime() - right->getStartTime();
            auto lastDiff = left->getLastTime() - right->getLastTime();
            auto diff = startDiff.count() + lastDiff.count();
            return diff > 0;
        }
    };

    std::priority_queue<RzTimer::spRzTimer, std::vector<RzTimer::spRzTimer>, _Rz_CompareTimer>  mTimers;
};
_RzStdEnd

#endif//_RZTIMER_HPP__