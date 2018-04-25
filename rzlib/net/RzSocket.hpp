#ifndef __RZSOCKET_HPP__
#define __RZSOCKET_HPP__
#pragma once

#include "../base/RzType.hpp"
#include <ctype.h>
#include <functional>
#include <memory>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#define RZ_SOCKET_STARTUP               \
	WSADATA wsd;                            \
	::WSAStartup(MAKEWORD(2, 2), &wsd);
#define RZ_SOCKET_CLEANUP               \
	::WSACleanup();
#define socklen_t int
#define RZ_ERRNO WSAGetLastError()
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
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define RZ_SOCKET_STARTUP 	signal(SIGPIPE, SIG_IGN);
#define RZ_SOCKET_CLEANUP
#define RZ_ERRNO errno
#endif

_RzStdBegin
_RzNameSpaceBegin(Net)
class CRzSockAddr : public sockaddr_in
{
public:
	CRzSockAddr(): addlen(sizeof(sockaddr_in)){}
	CRzSockAddr(unsigned short uport,const char* ip = NULL):addlen(sizeof(sockaddr_in))
    {
        unsigned long  nIp = htonl(INADDR_ANY);
        if(ip)
        {
            if(isalpha(ip[0]))
            {
 #if PLATFORM_TARGET == PLATFORM_WINDOWS               
                LPHOSTENT     lphostent; 
                lphostent = ::gethostbyname(ip);
                if(lphostent)
                {
                    nIp = inet_addr(inet_ntoa(*((LPIN_ADDR)*lphostent->h_addr_list)));
                }	
#else       
                struct hostent *hptr;
                if ((hptr = gethostbyname(ip)) != NULL) 
                {
                    nIp = inet_addr(inet_ntoa(*((in_addr*)*hptr->h_addr_list)));
                }
#endif
            }
            else
            {
                nIp = inet_addr(ip);
            }
        }   

        assert(uport);
        this->sin_family = AF_INET;   
        this->sin_port = htons(uport);    
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        this->sin_addr.S_un.S_addr = nIp;
#else
        this->sin_addr.s_addr = nIp;
#endif
    }
	~CRzSockAddr(){}

	socklen_t addlen;
};
class CRzSocket 
{
public:
    typedef std::function<bool(void)> ExitWaitPred;
    CRzSocket() : _s(INVALID_SOCKET){}
	virtual ~CRzSocket(){ Close(); }
public:
    inline operator SOCKET() const { return _s;};

    inline bool Ioctl(long cmd,u_long* argp);
    inline void SetKeepAlive(bool on); 
    inline void SetTcpNoDelay(bool on);

    inline bool IsCreate()const {return INVALID_SOCKET != _s;}
	inline bool Create(int ntype = SOCK_STREAM);
	inline bool Close();
	inline bool Bind(const CRzSockAddr* addr);
    inline bool Listen(int nbacklog = 5);
	inline SOCKET Accept(CRzSockAddr* addr = NULL);
    inline bool IsConnect() const;
	inline bool Connect(const CRzSockAddr* pRemoteaddr);
	inline bool ConnectEx(const CRzSockAddr* pRemoteaddr,int nTimeOut = 10,ExitWaitPred isExitWaitPred = NULL);

    inline bool WaitReadable(unsigned int nTimeOut = 10,ExitWaitPred isExitWaitPred = NULL);
	inline bool WaitWriteable(unsigned int nTimeOut = 10,ExitWaitPred isExitWaitPred = NULL);

	inline int Recv(char* buf,size_t cnt);
	inline int Send(char* buf,size_t cnt);

	inline int RecvFrom(char* buf,size_t cnt,CRzSockAddr* pRemoteaddr = NULL);
	inline int SendTo(char* buf,size_t cnt,CRzSockAddr* pRemoteaddr = NULL);
protected:
	SOCKET    _s;
};

typedef std::shared_ptr<CRzSocket> spSocketT;

bool CRzSocket::Ioctl(long cmd,u_long* argp)
{
	assert(IsCreate());
	if(!IsCreate()) return false;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	return SOCKET_ERROR != ::ioctlsocket(_s,cmd,argp);
#else
    return SOCKET_ERROR != ioctl(_s, cmd, argp);
#endif
}
void CRzSocket::SetKeepAlive(bool on)
{
    assert(IsCreate());
    if(!IsCreate()) return;
    char optval = on ? 1 : 0;
	::setsockopt(_s, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}
void CRzSocket::SetTcpNoDelay(bool on)
{
    assert(IsCreate());
    if(!IsCreate()) return;
    char optval = on ? 1 : 0;
    ::setsockopt(_s, SOL_SOCKET, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}
bool CRzSocket::Create(int nType)
{
	if(IsCreate()) return true;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    _s = ::WSASocket(AF_INET, nType, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
    _s = ::socket(AF_INET, nType, 0);
#endif
    if (INVALID_SOCKET == _s) 
    {
        return false;
    }

	assert(IsCreate());
	return IsCreate();
}
bool CRzSocket::Close()
{
	if(!IsCreate()) return true;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    ::shutdown(_s, SD_BOTH);
    if (SOCKET_ERROR == ::closesocket(_s))
        return false;
#else
    ::shutdown(_s, SHUT_RDWR);
    if (-1 == ::close(_s))
        return false;
#endif
	_s = INVALID_SOCKET;
	return !IsCreate();
}
bool CRzSocket::Bind(const CRzSockAddr* addr)
{
	assert(IsCreate());
	if(!IsCreate()) return false;
	assert(addr);
	return SOCKET_ERROR != ::bind(_s,(sockaddr*)addr, addr->addlen);
}
bool CRzSocket::Listen(int nbacklog /* = 5 */)
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	return SOCKET_ERROR != ::listen(_s, nbacklog);
}
SOCKET CRzSocket::Accept(CRzSockAddr* addr/* = NULL*/)
{
	assert(IsCreate());
	if(!IsCreate()) return false;
	return ::accept(_s,(sockaddr*)addr, &(addr->addlen));
}
bool CRzSocket::IsConnect() const
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	socklen_t optval, optlen = sizeof(int);
	if(SOCKET_ERROR != getsockopt(_s, SOL_SOCKET, SO_ERROR,(char*) &optval, &optlen))
	{
		switch(optval)
		{
		case 0: return true; 
#if PLATFORM_TARGET == PLATFORM_WINDOWS   
		case ECONNREFUSED:break;
#endif
		}
	}
	
	return false;
}
bool CRzSocket::Connect(const CRzSockAddr* pRemoteaddr)
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	assert(pRemoteaddr);
	int ret = ::connect(_s,(sockaddr*)pRemoteaddr,(pRemoteaddr ? pRemoteaddr->addlen : NULL));
    if(SOCKET_ERROR == ret
#if PLATFORM_TARGET == PLATFORM_WINDOWS  
        && WSAEWOULDBLOCK != ::WSAGetLastError()
#endif
    )
    {
        return false;
    }
    return true;
}
bool CRzSocket::ConnectEx(const CRzSockAddr* pRemoteaddr,int nTimeOut /* = 10 */,CRzSocket::ExitWaitPred isExitWaitPred/* = NULL*/)
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	assert(pRemoteaddr);
	unsigned long ul = 1; 
	this->Ioctl(FIONBIO, &ul);//设置阻塞模式<0为阻塞，1非阻塞>  
	int nRet = ::connect(_s,(sockaddr*)pRemoteaddr,(pRemoteaddr ? pRemoteaddr->addlen : NULL)); 
	if(SOCKET_ERROR == nRet)
	{
#if PLATFORM_TARGET == PLATFORM_WINDOWS  
        int nError = ::WSAGetLastError();
#else
        int nError = errno;
#endif
#if PLATFORM_TARGET == PLATFORM_WINDOWS  
		if(WSAEWOULDBLOCK != nError && 
			WSAEALREADY != nError &&
			WSAEISCONN != nError)
		{ 
			return false;
		}
		if(WSAEISCONN != nError)
		{
			if(!this->WaitWriteable(nTimeOut,isExitWaitPred))
				return false;
		}
#else
        if(EWOULDBLOCK != nError && 
            EALREADY != nError &&
            EISCONN != nError)
        {
            return false;
        }
        if(EISCONN != nError)
        {
            if(!this->WaitWriteable(nTimeOut,isExitWaitPred))
				return false;
        }
#endif
	}
	ul = 0;
	this->Ioctl(FIONBIO, &ul);
	return true;
}
bool CRzSocket::WaitReadable(unsigned int nTimeOut /* = 10 */,CRzSocket::ExitWaitPred isExitWaitPred /* = NULL */)
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	if(0 >= nTimeOut)
		nTimeOut = 1;
	while(nTimeOut-- && !(isExitWaitPred ? isExitWaitPred() : false))
	{
		fd_set      fdRead;
		timeval     tv;
		FD_ZERO(&fdRead);
		FD_SET(_s,&fdRead);
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		int nRet = ::select(_s,&fdRead,NULL,NULL,&tv);
		if(0 < nRet)
			return true;
		if(0 > nRet)
			break;
	}
	return false;
}
bool CRzSocket::WaitWriteable(unsigned int nTimeOut /* = 10 */,CRzSocket::ExitWaitPred isExitWaitPred /* = NULL */)
{
	assert(IsCreate());
	if(!IsCreate()) return false;

	if(0 >= nTimeOut)
		nTimeOut = 1;
	while(nTimeOut-- && !(isExitWaitPred ? isExitWaitPred() : false))
	{
		fd_set      fdWrite;
		timeval     tv;
		FD_ZERO(&fdWrite);
		FD_SET(_s,&fdWrite);
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		int nRet = ::select(_s,NULL,&fdWrite,NULL,&tv);
		if(0 < nRet)
			return true;
		if(0 > nRet)
			break;
	}

	return false;
}
int CRzSocket::Recv(char* buf,size_t cnt)
{
	assert(IsCreate());
	return ::recv(_s,buf,cnt,0);
}
int CRzSocket::Send(char* buf,size_t cnt)
{
	assert(IsCreate());

	size_t nSendLen      = 0;
	while(nSendLen != cnt)
	{
		int nRet = ::send(_s,(char*)buf+nSendLen,cnt-nSendLen,0);
		if(nRet <= 0)
			break;

		nSendLen += nRet;
	}

	return nSendLen;
}
int CRzSocket::RecvFrom(char* buf,size_t cnt,CRzSockAddr* pRemoteaddr)
{
	assert(IsCreate());
	assert(pRemoteaddr);   
	return ::recvfrom(_s,buf,cnt,0,(sockaddr*)pRemoteaddr,(pRemoteaddr ? &pRemoteaddr->addlen : NULL));
}
int CRzSocket::SendTo(char* buf,size_t cnt,CRzSockAddr* pRemoteaddr)
{
	assert(IsCreate());
	assert(pRemoteaddr);

	return ::sendto(_s,buf,cnt,0,(sockaddr*)pRemoteaddr,(pRemoteaddr ? pRemoteaddr->addlen : NULL));
}
_RzNameSpaceEnd
_RzStdEnd


#endif//__RZSOCKET_HPP__