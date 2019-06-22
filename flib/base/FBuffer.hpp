#ifndef __FBUFFER_HPP__
#define __FBUFFER_HPP__
#pragma once
#include "FFunc.hpp"
#include "FAlloctor.hpp"
#include "FConvert.hpp"
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <bitset>
#include <functional>
#include <memory>

_FStdBegin

static const uint8 buffer_flag_isuser        = 1<<1;
static const uint8 buffer_flag_iserror       = 1<<2;

class FBuffer
{
public:
    const static size_t DEFAULT_SIZE = 1024*10;

    explicit FBuffer(const uint8 * pBuffer, size_t len)
		:_flag(buffer_flag_isuser),
		_rpos(0),
		_wpos(0),
		_size(len),
		_cnt(len),
		_pdata((uint8 *)pBuffer)
	{
		assert(pBuffer);
		assert(len > 0);
	}

    FBuffer(size_t len = DEFAULT_SIZE)
		:_flag(0),
		_rpos(0),
		_wpos(0),
		_size(len),
		_cnt(0),
		_pdata(NULL)
	{
		if (0 >= _size) _size = DEFAULT_SIZE;

		_pdata = getBuf(_size);
		memset(_pdata, 0, _size);
	}
    virtual ~FBuffer(void)
	{
		if (!isUserBuffer() && _pdata)
		{
			releaseBuf(_pdata);
			_pdata = NULL;
		}
	}
public:
    inline size_t              size() const;
    inline size_t              cnt() const;
    inline bool                empty() const;
    inline bool                full() const;
    inline void                clear();
    inline void                resize(size_t size = 0);

    inline size_t              rpos() const;
    inline size_t              wpos() const;

    inline size_t              rpos(size_t pos,bool advance=true);
    inline size_t              wpos(size_t pos,bool advance=true);

    inline void                SetError(bool error=true);
    inline bool                HasError();

    inline size_t              nextreadblocksize();
    inline size_t              nextwriteblocksize();

    inline int                 find(const uint8* dest,size_t n) const;
    //inline  size_t              rfind(const uint8* dest,size_t n);

    inline void                swap(FBuffer& buffer);


    inline const uint8*        data() const;
    inline const char*         c_str() const;
    inline size_t              Read(uint8 *dest,const size_t cnt);
    inline size_t              Write(const uint8 *src,const size_t cnt);

    template<typename T>
    inline size_t              Read(T &dst);
    template<typename T>
    inline T                   Read();

    template<typename T>
    inline void                Write(const T &src);


    inline uint8*   operator[](size_t pos) const;
    inline bool     operator==(FBuffer& src);

    inline void                posShow() const;
    inline void                textShow() const;
    inline void                hexShow() const;

    //
    template<typename T>
    inline FBuffer& operator<<(T v); // will generate link error
    inline FBuffer& operator<<(int8 v);
    inline FBuffer& operator<<(int16 v);
    inline FBuffer& operator<<(int32 v);
    inline FBuffer& operator<<(int64 v);
    inline FBuffer& operator<<(uint8 v);
    inline FBuffer& operator<<(uint16 v);
    inline FBuffer& operator<<(uint32 v);
    inline FBuffer& operator<<(uint64 v);
#if FLIB_COMPILER_64BITS
    inline FBuffer& operator<<(int v);
    inline FBuffer& operator<<(uint v);
#endif
    inline FBuffer& operator<<(bool v);
    inline FBuffer& operator<<(float v);
    inline FBuffer& operator<<(double v);
    inline FBuffer& operator<<(const char *str);
    inline FBuffer& operator<<(char v[]);
    inline FBuffer& operator<<(FBuffer &v);
    inline FBuffer& operator<< (FBuffer& (*_f)(FBuffer&));

	friend FBuffer& endl(FBuffer& v);

    //
    template<typename T>
    inline FBuffer& operator>>(T &t); // will generate link error
    inline FBuffer& operator>>(int8 &v);
    inline FBuffer& operator>>(int16 &v);
    inline FBuffer& operator>>(int32 &v);
    inline FBuffer& operator>>(int64 &v);
    inline FBuffer& operator>>(uint8 &v);
    inline FBuffer& operator>>(uint16 &v);
    inline FBuffer& operator>>(uint32 &v);
    inline FBuffer& operator>>(uint64 &v);
#if FLIB_COMPILER_64BITS
    inline FBuffer& operator>>(int &v);
    inline FBuffer& operator>>(uint &v);
#endif
    inline FBuffer& operator>>(bool &v);
    inline FBuffer& operator>>(float &v);
    inline FBuffer& operator>>(double &v);
    inline FBuffer& operator>>(const char *dst);
    inline FBuffer& operator>>(char dst[]);
    inline FBuffer& operator>>(FBuffer &v);

    //


protected:
    FBuffer(const FBuffer& src){} 
    // ...类型转换构造函数
    FBuffer &operator = (const FBuffer &src){return  *this;} 

    inline bool         isUserBuffer();
    inline uint8 *      getBuf(size_t size);
    inline void         releaseBuf(uint8 * p);
protected:
    uint8 *             _pdata;

    size_t              _cnt;
    size_t              _size;
    size_t              _rpos;
    size_t              _wpos;

    uint8               _flag;
};
typedef std::shared_ptr<FBuffer> spFBufferT;

_FStdEnd

_FStdBegin
inline FBuffer& operator<<(FBuffer& buffer,const std::string &v)
{
    size_t len = v.size();
    buffer.Write<uint32>(len);
    buffer.Write((const uint8 *)v.c_str(), len);
    return buffer;
}
inline FBuffer& operator>>(FBuffer& buffer,std::string &v)
{
    v.clear();
    int len;
    buffer.Read(len);
    for(int i=0; i<len; ++i)
    {
        uint8 c = '\0';
        buffer>>c;
        v += c;
    }

    return buffer;
}
template<class T>
inline FBuffer& operator<<(FBuffer& buffer,std::set<T>& TSet)
{
    buffer<<(uint32)TSet.size();
    typename std::set<T>::const_iterator itor;
    for (itor = TSet.begin();itor != TSet.end();++itor)
        buffer<<*itor;

    return buffer;
}
template<class T>
inline FBuffer& operator>>(FBuffer& buffer,std::set<T>& TSet)
{
    uint32 size = 0;
    buffer>>size;
    TSet.clear();

    while(size--)
    {
        T t;
        buffer>>t;
        TSet.insert(t);
    }

    return buffer;
}
template<class T>
inline FBuffer& operator<<(FBuffer& buffer,std::vector<T>& TVector)
{
    buffer<<(uint32)TVector.size();

    typename std::vector<T>::const_iterator itor;
    for(itor = TVector.begin();itor!=TVector.end();++itor)
        buffer<< *itor;

    return buffer;
}
template<class T>
inline FBuffer& operator>>(FBuffer& buffer,std::vector<T>& TVector)
{
    uint32 size = 0;
    buffer>>size;
    TVector.clear();

    while(size--)
    {
        T t;
        buffer>>t;
        TVector.push_back(t);
    }
    return buffer;
}
template<class T>
inline FBuffer& operator<<(FBuffer& buffer,std::list<T>& TList)
{
    buffer<<(uint32)TList.size();

    typename std::list<T>::const_iterator itor;
    for(itor = TList.begin();itor != TList.end();++itor)
        buffer<< *itor;

    return buffer;
}

template<class T>
inline FBuffer& operator>>(FBuffer& buffer,std::list<T>& TList)
{
    uint32 size = 0;
    buffer>>size;
    TList.clear();

    while(size--)
    {
        T t;
        buffer>>t;
        TList.push_back(t);
    }

    return buffer;
}
template<class K,class V>
inline FBuffer& operator<<(FBuffer& buffer,std::map<K,V>& TMap)
{
    buffer<<(uint32)TMap.size();

    typename std::map<K,V>::const_iterator itor;
    for(itor = TMap.begin();itor!=TMap.end();++itor)
        buffer
        <<itor->first
        <<itor->second;

    return buffer;
}

template<class K,class V>
inline FBuffer& operator>>(FBuffer& buffer,std::map<K,V>& TMap)
{
    uint32 size = 0;
    buffer>>size;
    TMap.clear();

    while(size--)
    {
        K k;
        V v;
        buffer>>k>>v;
        TMap.insert(make_pair(k,v));
    }

    return buffer;
}

bool FBuffer::isUserBuffer() {
    return (_flag&buffer_flag_isuser) > 0;
}
uint8 * FBuffer::getBuf(size_t size)
{
    assert(size > 0);
    return (uint8 *)FPoolMalloc(size);
}
void FBuffer::releaseBuf(uint8 * p) 
{
    assert(NULL != p);
    FPoolFree(p);
}

size_t  FBuffer::size() const
{
    return _size;
}
size_t FBuffer::cnt() const
{
    return _cnt;
}
bool   FBuffer::full() const 
{
    return _size == _cnt;
}
bool   FBuffer::empty() const
{
    return 0 == _cnt;
}
size_t  FBuffer::rpos() const
{
    return _rpos;
}
size_t   FBuffer::wpos() const
{
    return _wpos;
}
size_t  FBuffer::rpos(size_t pos,bool advance /*=true*/)
{
    assert(!(_rpos == _wpos && pos == _rpos));
    _rpos = (pos%_size);
    if(_rpos == _wpos) {
        _cnt = advance ? 0 : size();
    }else {
        _cnt = (_rpos > _wpos) ? (size() - (_rpos - _wpos)) : (_wpos - _rpos);
    }
    return _rpos;
}
size_t  FBuffer::wpos(size_t pos,bool advance/*=true*/)
{
    assert(!(_rpos == _wpos && pos == _wpos));
    _wpos = (pos%_size);
    if(_wpos == _rpos) {
        _cnt = advance ? size() : 0;
    }else {
        _cnt = (_rpos > _wpos) ? (size() -(_rpos - _wpos)) : (_wpos - _rpos);
    }
    return _wpos;
}
void   FBuffer::clear() 
{
    _rpos = 0;
    _wpos = 0;
    _cnt  = 0;
}
void   FBuffer::resize(size_t size /*= 0*/) {
    if(size <= cnt()) size = 2*_size;
    if(0 == size) size = DEFAULT_SIZE;
    // if(size == _size) return;

    uint8 * pnew = getBuf(size);
    assert(pnew);
    //memset(pnew, 0, size);

    if(_cnt > 0) memcpyFromRing(pnew, _cnt, _pdata, _rpos, _size);

    if (!isUserBuffer() && _pdata)
        releaseBuf(_pdata);

    _pdata   = pnew;
    _rpos    = 0;
    _wpos    = _cnt;
    _size    = size;

    _flag &= ~buffer_flag_isuser;
}

size_t  FBuffer::nextreadblocksize() {
    if(empty()) return 0;
    return (_wpos > _rpos) ? _cnt : (_size - _rpos);
}
size_t  FBuffer::nextwriteblocksize() {
    if(full()) return 0;
    return (_wpos >= _rpos) ? (_size - _wpos) : (_rpos - _wpos);
}
int FBuffer::find(const uint8* dest,size_t n) const
{
    assert(dest);
    assert(n);
    if(!dest || 0>= n) return -1;
    if(cnt() < n) return -1;


    size_t pos = rpos();
    char* pdst = (char*)dest;
    size_t k = n;
    size_t j = _cnt;
    while(j-- >= k)
    {
        if(*((*this)[pos++]) != *pdst++)
        {
            pdst = (char*)dest;
            k    = n;
        }else
        {
            --k;
            if(0 == k)
                return (int)((pos - n)%_size);
        }
    }
    return -1;
}
void FBuffer::swap(FBuffer& buffer)
{
    std::swap(this->_pdata,buffer._pdata);
    std::swap(this->_cnt,buffer._cnt);
    std::swap(this->_size,buffer._size);
    std::swap(this->_rpos,buffer._rpos);
    std::swap(this->_wpos,buffer._wpos);
    std::swap(this->_flag,buffer._flag);
}
const uint8* FBuffer::data() const
{
    return (*this)[rpos()];
}
const char* FBuffer::c_str() const
{
    return (const char*)data();
}
size_t FBuffer::Read(uint8 *dest,const size_t cnt)
{
    assert(dest);
    assert(cnt >0);
    if(HasError()) return 0;
    if (empty() || NULL == dest || _cnt < cnt) {
        SetError();
        return 0;
    }

    memcpyFromRing(dest, cnt, _pdata, _rpos, _size);
    _rpos = (_rpos + cnt)%_size;


    _cnt -= cnt;
    assert(_cnt >= 0);
    assert(_cnt <= _size);
    return cnt;
}
size_t   FBuffer::Write(const uint8 *src,const size_t cnt) 
{
    assert(src);
    assert(cnt >0);
    if(NULL == src || 0 >= cnt) return 0;

    if (cnt > (_size - _cnt)) {
        const size_t a = 2*(cnt + _cnt);
        const size_t b = 2*_size;
        const size_t newsize = (a > b ? a : b);
        this->resize(newsize);
    }

    memcpyToRing(src, cnt, _pdata, _wpos, _size);
    _wpos = (_wpos + cnt)%_size;

    _cnt += cnt;
    assert(_cnt >= 0);
    assert(_cnt <= _size);
    return cnt;
}
void    FBuffer::SetError(bool error/*=true*/) {
    error ? (_flag |= buffer_flag_iserror) : (_flag &= ~buffer_flag_iserror);
}
bool    FBuffer::HasError() {
    return (_flag&buffer_flag_iserror) > 0;
}
bool FBuffer::operator==(FBuffer& src) 
{
    if(this == &src) 
        return true;
    if(cnt() != src.cnt())
        return false;

    for (size_t i = _rpos; i <_cnt+_rpos; ++i) {
        if(*(*this)[i] != *src[i])
            return false;
    }

    return true;
}
uint8* FBuffer::operator[](size_t pos) const {
    return &_pdata[pos%_size];
}

template<typename T>
inline size_t FBuffer::Read(T &dst)
{
    size_t len = Read((uint8 *)&dst,sizeof(T));
    EndianConvert(dst);
    return len;
}
template<typename T>
inline T FBuffer::Read()
{
    T v;
    Read<T>(v);
    return v;
}
template<typename T>
inline void FBuffer::Write(const T &src)
{
    EndianConvert(src);
    Write((uint8 *)&src,sizeof(T));
}

//////////////////////////////////////////////////////////////////////////
FBuffer& FBuffer::operator<<(int8 v)
{
    Write<int8>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(int16 v)
{
    Write<int16>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(int32 v)
{
    Write<int32>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(int64 v)
{
    Write<int64>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(uint8 v)
{
    Write<uint8>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(uint16 v)
{
    Write<uint16>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(uint32 v)
{
    Write<uint32>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(uint64 v)
{
    Write<uint64>(v);
    return *this;
}
#if FLIB_COMPILER_64BITS
FBuffer& FBuffer::operator<<(int v)
{
    Write<int>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(uint v)
{
    Write<uint>(v);
    return *this;
}
#endif
FBuffer& FBuffer::operator<<(bool v)
{
    Write<bool>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(float v)
{
    Write<float>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(double v)
{
    Write<double>(v);
    return *this;
}
FBuffer& FBuffer::operator<<(const char *str)
{
    assert(str);
    size_t len = strlen(str);
    Write<uint32>(len);
    Write((uint8 const *)str, len);
    return *this;
}
inline FBuffer& FBuffer::operator<<(char v[])
{
    size_t len = strlen(v);
    Write<uint32>(len);
    Write((uint8 const *)v, len);
    return *this;
}

FBuffer& FBuffer::operator<<(FBuffer &v)
{
    if(v.empty()) return *this;

    size_t pos = v.rpos();
    while (!v.empty()) {
        uint8 * src = v[v.rpos()];
        size_t  cnt = v.nextreadblocksize();
        this->Write(src, cnt);
        v.rpos(v.rpos() + cnt,true);
    }
    v.rpos(pos,false);
    return *this;
}
FBuffer& FBuffer::operator<< (FBuffer& (*_f)(FBuffer&))
{
	return _f(*this);
}
inline FBuffer& endl(FBuffer& v)
{
    v << '\n';
	return v;
}

//////////////////////////////////////////////////////////////////////////
FBuffer& FBuffer::operator>>(int8 &v)
{
    Read<int8>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(int16 &v)
{
    Read<int16>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(int32 &v)
{
    Read<int32>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(int64 &v)
{
    Read<int64>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(uint8 &v)
{
    Read<uint8>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(uint16 &v)
{
    Read<uint16>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(uint32 &v)
{
    Read<uint32>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(uint64 &v)
{
    Read<uint64>(v);
    return *this;
}
#if FLIB_COMPILER_64BITS
FBuffer& FBuffer::operator>>(int &v)
{
    Read<int>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(uint &v)
{
    Read<uint>(v);
    return *this;
}
#endif
FBuffer& FBuffer::operator>>(bool &v)
{
    Read<bool>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(float &v)
{
    Read<float>(v);
    return *this;
}
FBuffer& FBuffer::operator>>(double &v)
{
    Read<double>(v);
    return *this;
}

FBuffer& FBuffer::operator>>(const char *dst)
{
    uint32 len;
    Read<uint32>(len);
    uint8* p = (uint8*)dst;
    Read(p, len);
    return *this;
}
FBuffer& FBuffer::operator>>(char dst[])
{
    uint32 len;
    Read<uint32>(len);
    uint8* p = (uint8*)dst;
    Read(p, len);
    return *this;
}
FBuffer& FBuffer::operator>>(FBuffer &v)
{
    v<<*this;
    return *this;
}
void FBuffer::posShow() const
{
    printf("size:%ld,cnt:%ld,rpos:%ld,wpos:%ld\n",
        _size,
        _cnt,
        _rpos,
        _wpos);
}

void FBuffer::textShow() const
{
    posShow();
    size_t pos = _rpos;

    for (size_t i=0; i<_cnt; ++i) {
        char c = *(char*)((*this)[pos++]);
        switch (c)
        {
        case '\r':
            printf("/r\t");
            break;
        case '\n':
            printf("/n\t");
            break;
        case '\t':
            printf("/t\t");
            break;
        case '\0':
            printf("/0\t");
            break;
        default:
            printf("%c\t",c);
        }
        pos %= _size;
    }
    printf("\n");
}
void FBuffer::hexShow() const
{
    posShow();
    size_t pos = _rpos;

    for (size_t i=0; i<_cnt; ++i) {
        printf("%X\t",_pdata[pos++]);
        pos %= _size;
    }
    printf("\n");
}


_FStdEnd

#endif//__FBUFFER_HPP__
