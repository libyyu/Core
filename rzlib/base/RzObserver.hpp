#ifndef __RZOBSERVER_HPP__
#define __RZOBSERVER_HPP__
#pragma once
#include "RzType.hpp"
#include <map>
#include <vector>

_RzStdBegin
template <typename ReturnT, typename ParamT>
class CRzReceiverImplBase;

template <typename ReturnT, typename ParamT>
class CRzObserverImplBase
{
public:
	virtual void AddReceiver(CRzReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
	virtual void RemoveReceiver(CRzReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
	virtual ReturnT Broadcast(ParamT param) = 0;
	virtual ReturnT RBroadcast(ParamT param) = 0;
	virtual ReturnT Notify(ParamT param) = 0;
};

template <typename ReturnT, typename ParamT>
class CRzReceiverImplBase
{
public:
	virtual void AddObserver(CRzObserverImplBase<ReturnT, ParamT>* observer) = 0;
	virtual void RemoveObserver() = 0;
	virtual ReturnT Receive(ParamT param) = 0;
	virtual ReturnT Respond(ParamT param, CRzObserverImplBase<ReturnT, ParamT>* observer) = 0;
};

template <typename ReturnT, typename ParamT>
class CRzReceiverImpl;

template <typename ReturnT, typename ParamT>
class CRzObserverImpl : public CRzObserverImplBase<ReturnT, ParamT>
{
	template <typename ReturnT_, typename ParamT_>
	friend class Iterator;
public:
	CRzObserverImpl()
	{}

	virtual ~CRzObserverImpl()	{}

	virtual void AddReceiver(CRzReceiverImplBase<ReturnT, ParamT>* receiver)
	{
		if (receiver == NULL)
			return;

		receivers_.push_back(receiver);
		receiver->AddObserver(this);
	}

	virtual void RemoveReceiver(CRzReceiverImplBase<ReturnT, ParamT>* receiver)
	{
		if (receiver == NULL)
			return;

		typename ReceiversVector::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			if (*it == receiver)
			{
				receivers_.erase(it);
				break;
			}
		}
	}

	virtual ReturnT Broadcast(ParamT param)
	{
		typename ReceiversVector::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			(*it)->Receive(param);
		}

		return ReturnT();
	}

	virtual ReturnT RBroadcast(ParamT param)
	{
		typename ReceiversVector::reverse_iterator it = receivers_.rbegin();
		for (; it != receivers_.rend(); ++it)
		{
			(*it)->Receive(param);
		}

		return ReturnT();
	}

	virtual ReturnT Notify(ParamT param)
	{
		typename ReceiversVector::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			(*it)->Respond(param, this);
		}

		return ReturnT();
	}

	template <typename ReturnT_, typename ParamT_>
	class Iterator
	{
		CRzObserverImpl<ReturnT_, ParamT_> & _tbl;
		unsigned long index;
		CRzReceiverImplBase<ReturnT_, ParamT_>* ptr;
	public:
		Iterator( CRzObserverImpl & table )
			: _tbl( table ), index(0), ptr(NULL)
		{}

		Iterator( const Iterator & v )
			: _tbl( v._tbl ), index(v.index), ptr(v.ptr)
		{}

		CRzReceiverImplBase<ReturnT_, ParamT_>* next()
		{
			if ( index >= _tbl.receivers_.size() )
				return NULL;

			for ( ; index < _tbl.receivers_.size(); )
			{
				ptr = _tbl.receivers_[ index++ ];
				if ( ptr )
					return ptr;
			}
			return NULL;
		}
	};

protected:
	typedef std::vector<CRzReceiverImplBase<ReturnT, ParamT>*> ReceiversVector;
	ReceiversVector receivers_;
};
/*
template <typename ReturnT, typename ParamT>
class CRzReceiverImpl : public CRzReceiverImplBase<ReturnT, ParamT>
{
public:
	CRzReceiverImpl()
	{}

	virtual ~CRzReceiverImpl()	{}

	virtual void AddObserver(CRzObserverImplBase<ReturnT, ParamT>* observer)
	{
		observers_.push_back(observer);
	}

	virtual void RemoveObserver()
	{
		ObserversVector::iterator it = observers_.begin();
		for (; it != observers_.end(); ++it)
		{
			(*it)->RemoveReceiver(this);
		}
	}

	virtual ReturnT Receive(ParamT param)
	{
		return ReturnT();
	}

	virtual ReturnT Respond(ParamT param, CRzObserverImplBase<ReturnT, ParamT>* observer)
	{
		return ReturnT();
	}

protected:
	typedef std::vector<CRzObserverImplBase<ReturnT, ParamT>*> ObserversVector;
	ObserversVector observers_;
};*/
_RzStdEnd

#endif // __RZOBSERVER_HPP__