#ifndef __FLogInterface_HPP__
#define __FLogInterface_HPP__
#pragma once
#include <iostream>
#include <string>
#include <sstream> 
#include "FType.hpp"
#include "FFunc.hpp"

_FStdBegin
enum F_LOGLEVEL 
{
	F_LOGLEVEL_TRACE,
	F_LOGLEVEL_DEBUG,
	F_LOGLEVEL_INFO,
	F_LOGLEVEL_WARN,
	F_LOGLEVEL_ERROR,
	F_LOGLEVEL_FATAL,
    F_LOGLEVEL_NUM_LOG_LEVELS
};

const static char* FLogLevelName[F_LOGLEVEL_NUM_LOG_LEVELS] =
{
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

class FLogFinisher;
class FLograceFunction;
class FLogInterface
{
public:
    FLogInterface(F_LOGLEVEL level):_level(level) 
    {
        char buff[200] = {0};
        tm* aTm = FGetNowTime();
        sprintf(buff, "%-4d-%02d-%02d %02d:%02d:%02d",
            aTm->tm_year + 1900,
            aTm->tm_mon + 1,
            aTm->tm_mday,
            aTm->tm_hour,
            aTm->tm_min,
            aTm->tm_sec);
        *this << "[" 
		<< FGetCurrentThreadId() << "|"
		<< FLogLevelName[_level] << "|" 
        << buff << "|"
        << "]";
    }
    FLogInterface(F_LOGLEVEL level, const char* filename, int line = -1)
    : _level(level)
    {
        char buff[200] = {0};
        tm* aTm = FGetNowTime();
        sprintf(buff, "%-4d-%02d-%02d %02d:%02d:%02d",
            aTm->tm_year + 1900,
            aTm->tm_mon + 1,
            aTm->tm_mday,
            aTm->tm_hour,
            aTm->tm_min,
            aTm->tm_sec);
        *this << "[" 
		<< FGetCurrentThreadId() << "|"
		<< FLogLevelName[_level] << "|" 
        << buff << "|"
		<< (filename ? filename : "<unknow source>") << ":" 
        << line
        << "]";
    }
    virtual ~FLogInterface()
    {
        _message.clear();
    }
    template<typename T>
    inline FLogInterface& operator<<(T v); // will generate link error
#define TRMPLATE_DECLARE(T) \
    inline FLogInterface& operator<<(T v);

    TRMPLATE_DECLARE(int8)
    TRMPLATE_DECLARE(int16)
    TRMPLATE_DECLARE(int32)
    TRMPLATE_DECLARE(int64)
    TRMPLATE_DECLARE(uint8)
    TRMPLATE_DECLARE(uint16)
    TRMPLATE_DECLARE(uint32)
    TRMPLATE_DECLARE(uint64)
    TRMPLATE_DECLARE(bool)
    TRMPLATE_DECLARE(float)
    TRMPLATE_DECLARE(double)
    TRMPLATE_DECLARE(const char *)
    TRMPLATE_DECLARE(void *)
    TRMPLATE_DECLARE(const std::string&)
#undef TRMPLATE_DECLARE
    inline FLogInterface& operator<<(char v[]);
	inline FLogInterface& operator<< (FLogInterface& (*_f)(FLogInterface&));
	friend FLogInterface& endl(FLogInterface& v);
protected:
    virtual void _LogImpl() {}
    inline void Finish()
    {
        _LogImpl();
        _message.clear();
    }
protected:
    inline void _Logout(const char* str) { _message += str; }
    friend class FLogFinisher;
    friend class FLogTraceFunction;
    std::string _message;
    F_LOGLEVEL _level;
};
inline FLogInterface& endl(FLogInterface& v)
{
	v._Logout("\n");
	return v;
}

#define TRMPLATE_METHOD(T) \
	FLogInterface& FLogInterface::operator<<(T v) \
	{ \
		std::stringstream str;	\
		str << v; \
        _Logout(str.str().c_str()); \
		return *this; \
	}
TRMPLATE_METHOD(int8)
TRMPLATE_METHOD(int16)
TRMPLATE_METHOD(int32)
TRMPLATE_METHOD(int64)
TRMPLATE_METHOD(uint8)
TRMPLATE_METHOD(uint16)
TRMPLATE_METHOD(uint32)
TRMPLATE_METHOD(uint64)
TRMPLATE_METHOD(float)
TRMPLATE_METHOD(double)
TRMPLATE_METHOD(bool)
#undef TRMPLATE_METHOD

FLogInterface& FLogInterface::operator<<(const char* str)
{
	_Logout(str);
	return *this;
}
FLogInterface& FLogInterface::operator<<(const std::string& str)
{
	_Logout(str.c_str());
	return *this;
}
FLogInterface& FLogInterface::operator<<(char v[])
{
    _Logout((char*)v);
	return *this;
}
FLogInterface& FLogInterface::operator<<(void *p)
{
	_Logout(FFormat("%p", p).c_str());
	return *this;
}

FLogInterface& FLogInterface::operator<< (FLogInterface& (*_f)(FLogInterface&))
{
	return _f(*this);
}

class FLogFinisher
{
public:
	inline void operator=(FLogInterface &other)
    {
        other.Finish();
    }
};

class FLogTraceFunction
{
public:
    FLogTraceFunction(FLogInterface& other, const char *func, const char *file, int line)
    : _log(other)
    , _func(func)
	, _file(file)
	, _line(line){}
	~FLogTraceFunction()
    {
        char buff[200] = {0};
        tm* aTm = FGetNowTime();
        sprintf(buff, "%-4d-%02d-%02d %02d:%02d:%02d",
            aTm->tm_year + 1900,
            aTm->tm_mon + 1,
            aTm->tm_mday,
            aTm->tm_hour,
            aTm->tm_min,
            aTm->tm_sec);

        _log << "[" 
		<< FGetCurrentThreadId() << "|"
		<< FLogLevelName[_log._level] << "|" 
        << buff << "|"
		<< (_file ? _file : "<unknow source>") << ":" 
        << _line
        << "]"
        << _func << "() leave "
        << endl;

        *this = _log;
    }
	void operator=(FLogInterface &other)
    {
        other.Finish();
    }
private:
    FLogInterface& _log;
	const char *_func;
	const char *_file;
	int _line;
};

_FStdEnd

#endif//__FLogInterface_HPP__