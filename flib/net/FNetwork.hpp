#ifndef __FNETWORK_HPP__
#define __FNETWORK_HPP__
#pragma once
#include <stdio.h>
#include <string>
#include "FSocket.hpp"
_FStdBegin
_FNameSpaceBegin(Net)
class IFLinker
{
public:
	virtual void    onConnected(IFLinker* h) = 0;
	virtual void    onDisconnected(IFLinker* h,const int ec) = 0;
	virtual void    onSendCompleted(IFLinker* h,const void* pbuffer,const int ec,size_t bytes) = 0;
	virtual size_t  onRecvCompleted(IFLinker* h,const void* pbuffer,size_t bytes) = 0;
	virtual void    onDestroy(IFLinker* h) = 0;
};

class IFListener
{
public:
	virtual void    onConnected(IFListener*,IFLinker*) = 0;
	virtual void    onDisconnected(IFListener*,IFLinker*,const int ec) = 0;
	virtual void    onSendCompleted(IFListener*,IFLinker*,const void* pbuffer,const int ec,size_t bytes) = 0;
	virtual size_t  onRecvCompleted(IFListener*,IFLinker*,const void* pbuffer,size_t bytes) = 0;
	virtual void    onDestroy(IFListener*,IFLinker*) = 0;
};
_FNameSpaceEnd
_FStdEnd

#endif//__FNETWORK_HPP__