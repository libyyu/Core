#ifndef __RZLINK_HPP__
#define __RZLINK_HPP__
#pragma once
#include <stdio.h>
#include <string>
#include "../base/RzBuffer.hpp"
#include "../3rd/zlib/RzZlib.hpp"
#include "RzSocket.hpp"
_RzStdBegin
_RzNameSpaceBegin(Net)
class CRzLink
{
protected:
	static const size_t MinBufferSize = 1024;
	static const size_t MaxBufferSize = 1024*1024;
	static const size_t MaxErrCnt     = 2;
	static const size_t TimeOutSec    = 8;
public:
	CRzLink() 
    : _nTimeOut(TimeOutSec)
    , _isConnected(false)
    {}
	virtual ~CRzLink(){}
public:
	inline spRzBufferT GetBuffer();

	inline void SetTimeOut(size_t nTimeOut);
	inline size_t GetTimeOut() const{return _nTimeOut;};

	inline bool IsConnected()const{return _isConnected;};
	inline bool Connect(CRzSockAddr& addr);
	inline bool Close();
	inline bool Send(const void* buf,size_t nlen);
	inline bool Recv(size_t minSize = 0);
protected:
	inline bool FixBuffer(CRzBuffer* pbuffer);
	inline bool _Recv(CRzBuffer* pbuffer,size_t minSize = 0);
protected:
	size_t          _nTimeOut;
	spRzBufferT     _pBuffer;
	bool            _isConnected;
	CRzSocket       _s;
};
inline spRzBufferT CRzLink::GetBuffer()
{
    if(!_pBuffer)
			_pBuffer = spRzBufferT(new CRzBuffer);
	return _pBuffer;
}
void CRzLink::SetTimeOut(size_t nTimeOut)
{
    if(0 == nTimeOut)
        nTimeOut = TimeOutSec;
    _nTimeOut = nTimeOut;
}
bool CRzLink::Connect(CRzSockAddr& addr)
{
    if(IsConnected()) return true;

    if(!_s.IsCreate())
        _s.Create(SOCK_STREAM);

    if(_s.ConnectEx(&addr,_nTimeOut))
        _isConnected = true;
    return IsConnected();
}
bool CRzLink::Close()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    ::shutdown(_s,SD_SEND);
#endif
    _s.Close();
    _isConnected = false;
    return !IsConnected();
}
bool CRzLink::Send(const void* buf,size_t nlen)
{
    if(_s.WaitWriteable(_nTimeOut))
        return _s.Send((char*)buf,nlen) == nlen;
    return false;
}
bool CRzLink::FixBuffer(CRzBuffer* pbuffer)
{
    assert(pbuffer);
    if(!pbuffer) return false;

    if(pbuffer->empty())
        pbuffer->clear();

    int i = MaxErrCnt;
    while(pbuffer->nextwriteblocksize() < MinBufferSize)
    {
        pbuffer->resize();
        --i;
        if(0 >= i)
            break;
    }

    return pbuffer->nextwriteblocksize() >= MinBufferSize;
}
bool CRzLink::Recv(size_t minSize /*= 0*/)
{
    spRzBufferT theBuffer(GetBuffer());
    return _Recv(theBuffer.get(),minSize);
}
bool CRzLink::_Recv(CRzBuffer* pbuffer,size_t minSize/* = 0*/)
{
    assert(IsConnected());
    if(!IsConnected()) return false;
    assert(pbuffer);
    if(!pbuffer) return false;

    size_t nRecvLen = 0;

    do 
    {
        if(!FixBuffer(pbuffer))
        {
            Close();
            assert(false);
            return false;
        }

        if(!_s.WaitReadable(_nTimeOut))
        {
            Close();
            //assert(false);
            return false;
        }

        char* buf   = (char*)((*pbuffer)[pbuffer->wpos()]);
        size_t size = pbuffer->nextwriteblocksize();

        int n = _s.Recv(buf,size);

        if(n > 0) 
        {
            nRecvLen += n;
            pbuffer->wpos(pbuffer->wpos() + n);
        }
        if(0 == n) 
        {
            Close();
            break;
        }
        if(n <0)
        {
            Close();
            //assert(false);
            return false;
        }

    } while (nRecvLen < minSize);

    return (nRecvLen >= minSize)&&(nRecvLen > 0);
}
_RzNameSpaceEnd
_RzStdEnd

_RzStdBegin
_RzNameSpaceBegin(Net)
class CRzHttpLink : public CRzLink
{
public:
	CRzHttpLink(){}
	virtual ~CRzHttpLink(){}

public:
	inline bool SendHttpRequest(const std::string& s);
protected:
	inline bool GetAllResponse();
	inline bool ParseResponseHeader(bool &bGZip,bool &bChunked,int &nContentLen,int& nError);
	inline bool RecvByContentLen(const int nContenLen);
	inline bool RecvByChunked();
};
inline bool CRzHttpLink::SendHttpRequest(const std::string& s)
{
    assert(!s.empty());
    if(s.empty()) return false;

    assert(IsConnected());
    if(!IsConnected()) return false;

    if(!Send(s.c_str(),s.length())) return false;

    return GetAllResponse();
}
inline bool CRzHttpLink::ParseResponseHeader(bool &bGZip,bool &bChunked,int &nContentLen,int& nError)
{
    const int pos = _pBuffer->find((uint8*)("\r\n\r\n"),4);
    if(-1 == pos) return false;

    static const char HTTP_OK_FLAG[]   ="200 OK";
    static const char GZIP[]           ="gzip";
    static const char CONTENT_ENCOD[]  ="Content-Encoding:";
    static const char CHUNKED[]        ="chunked";
    static const char TRANSFER_ENCOD[] ="Transfer-Encoding:";
    static const char CONTENT_LENGTH[] ="Content-Length: ";

    if(-1 == _pBuffer->find((uint8*)HTTP_OK_FLAG,strlen(HTTP_OK_FLAG)))
    {
        assert(false);
        return false;
    }

    bGZip = 
        (-1 != _pBuffer->find((uint8*)GZIP,strlen(GZIP)) &&
        -1 != _pBuffer->find((uint8*)CONTENT_ENCOD,strlen(CONTENT_ENCOD)));

    bChunked =
        (-1 != _pBuffer->find((uint8*)CHUNKED,strlen(CHUNKED)) &&
        -1 != _pBuffer->find((uint8*)TRANSFER_ENCOD,strlen(TRANSFER_ENCOD)));


    nContentLen = -1;
    int i = _pBuffer->find((uint8*)CONTENT_LENGTH,strlen(CONTENT_LENGTH));
    if(-1 != i)
    {
        *((*_pBuffer)[pos]) = '\0';
        sscanf((char*)(*_pBuffer)[i+strlen(CONTENT_LENGTH)],"%d\r\n",&nContentLen);
    }

//	assert(0 != nContentLen);
    _pBuffer->rpos(pos+4);
    return true;
}
inline bool CRzHttpLink::GetAllResponse()
{
    if(_pBuffer)
        _pBuffer->clear();

    bool bGZip         =  false;
    bool bChunked      =  false;
    int  nContentLen   = -1;
    bool bHeaderParsed = false;
    int  nError        = 200;

    do 
    {
        if(!Recv()) return false;
        bHeaderParsed = ParseResponseHeader(bGZip,bChunked,nContentLen,nError);
    } while (!bHeaderParsed);

    assert(200 == nError);
    if(200 != nError)
        return false;

    bool bOK = (bChunked ? RecvByChunked() : RecvByContentLen(nContentLen));
    //assert(bOK);
    if(!bOK) return false;

    if(bGZip)
    {
        char* p_buf   = NULL;
        char* p_inbuf = (char*)((*_pBuffer)[_pBuffer->rpos()]);
        int nResult = ungzip(p_inbuf, _pBuffer->cnt(), &p_buf, 1);
        if(Z_OK != nResult)
        {
            assert(false);
            return false;
        }

        //gzip有10个字节的头部和8个字节的尾部
        std::string sBuf(p_buf);
        sBuf.erase(sBuf.rfind('}') + 1);
        _pBuffer->clear();
        //_pBuffer->Write((uint8*)p_buf,strlen(p_buf)-18); 
        (*_pBuffer)<<sBuf;
        free(p_buf);
    }

    return bOK;
}
inline bool CRzHttpLink::RecvByContentLen(const int nContenLen)
{
    if(nContenLen > 0)
    {
        if(_pBuffer->cnt() >= (size_t)nContenLen) return true;
    }

    const int oldLen = _pBuffer->cnt();

    do 
    {
        int needRecvSize = (nContenLen > 0 ? (nContenLen - _pBuffer->cnt()) : 0);
        if(!Recv(needRecvSize))
        {
            return (nContenLen > 0 ? false : true);
        }

        if(nContenLen > 0 && _pBuffer->cnt() >= (size_t)nContenLen)
            return true;
    } while (true);
    assert(false);
    return false;
}
bool CRzHttpLink::RecvByChunked()
{
    static const char CHUNKED[]        ="\r\n";
    static const size_t MinChunkedSize = strlen(CHUNKED);

    CRzBuffer chunkedBuffer;
    chunkedBuffer<<(*_pBuffer);
    _pBuffer->clear();

    size_t chunkCnt = 0;
    long datasize = -1;
    do 
    {
        do 
        {
            if(-1 != datasize)
            {
                if(chunkedBuffer.cnt() < (size_t)datasize)
                    break;
                else
                {
                    size_t bytes = datasize - MinChunkedSize;
                    while(bytes != 0)
                    {
                        size_t blocksize = std::min<size_t>(chunkedBuffer.nextreadblocksize(),bytes);
                        _pBuffer->Write(chunkedBuffer[chunkedBuffer.rpos()],blocksize);
                        chunkedBuffer.rpos(chunkedBuffer.rpos()+blocksize);
                        bytes -= blocksize;
                    }
                    chunkedBuffer.rpos(chunkedBuffer.rpos()+MinChunkedSize);
                    if(MinChunkedSize == datasize)
                        return true;
                    datasize = -1;
                    ++chunkCnt;
                }
            }

            if(chunkedBuffer.cnt() <= MinChunkedSize) 
                break;
            const int pos = chunkedBuffer.find((const uint8*)CHUNKED,MinChunkedSize);
            if(-1 == pos) break;
            assert(pos != chunkedBuffer.rpos());
            if(pos == chunkedBuffer.rpos())
                return false;
            if((size_t)pos > chunkedBuffer.rpos())
            {
                const char* pStart = (const char*)chunkedBuffer[chunkedBuffer.rpos()];
                char* pEnd = NULL;
                datasize = strtol(pStart,&pEnd,16);

                if(0 == datasize && pEnd == pStart)
                    return false;
                if(0 > datasize)
                    return false;
                if(datasize > MaxBufferSize)
                    return false;
                if(0 == datasize)
                {
                    assert(chunkCnt > 0);
                    if(0 >= chunkCnt) return false;
                }

                chunkedBuffer.rpos(pos+MinChunkedSize);
                datasize += MinChunkedSize;
            }else
            {
                std::string str;
                while(chunkedBuffer.rpos() != pos)
                {
                    char* pc = (char*)chunkedBuffer[chunkedBuffer.rpos()];
                    str += *pc;
                    chunkedBuffer.rpos(chunkedBuffer.rpos()+1);
                }

                const char* pStart = (const char*)chunkedBuffer[chunkedBuffer.rpos()];
                char* pEnd = NULL;
                datasize   = strtol(pStart,&pEnd,16);
                if(0 == datasize && pEnd == pStart)
                    return false;
                if(0 > datasize)
                    return false;
                if(datasize > MaxBufferSize)
                    return false;
                if(0 == datasize)
                {
                    assert(chunkCnt > 0);
                    if(0 >= chunkCnt) return false;
                }

                chunkedBuffer.rpos(MinChunkedSize);
                datasize += MinChunkedSize;
            }
        } while (true);

        if(!_Recv(&chunkedBuffer))
        {
            return false;
        }
    } while (1);
    return false;
}
_RzNameSpaceEnd
_RzStdEnd


#endif//__RZLINK_HPP__
