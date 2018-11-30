#ifndef __FLOG_HPP__
#define __FLOG_HPP__
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

class FGlobalLogLevel
{
public:
    static FGlobalLogLevel* get()
    {
        static FGlobalLogLevel hdl;
        return &hdl;
    }
    inline F_LOGLEVEL getLogLevel() const
    {
        return g_level;
    }
    inline void setLogLevel(F_LOGLEVEL level)
    {
        g_level = level;
    }
private:
    FGlobalLogLevel() : g_level(F_LOGLEVEL::F_LOGLEVEL_TRACE){}
    F_LOGLEVEL g_level;
};

class FLogFinisher;
class FLogTraceFunction;
class FILogMessage
{
public:
    FILogMessage(F_LOGLEVEL level) : _level(level){}
    FILogMessage(F_LOGLEVEL level, const char* filename, int line = -1)
    : _level(level)
    {
        char buff[100] = {0};
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
		<< FLogLevelName[F_LOGLEVEL::F_LOGLEVEL_TRACE] << "|" 
        << buff << "|"
		<< (filename ? filename : "<unknow source>") << ":" 
        << line
        << "]";
    }
    virtual ~FILogMessage()
    {
        _message.clear();
    }
    template<typename T>
    inline FILogMessage& operator<<(T v); // will generate link error
    inline FILogMessage& operator<<(int8 v);
    inline FILogMessage& operator<<(int16 v);
    inline FILogMessage& operator<<(int32 v);
    inline FILogMessage& operator<<(int64 v);
    inline FILogMessage& operator<<(uint8 v);
    inline FILogMessage& operator<<(uint16 v);
    inline FILogMessage& operator<<(uint32 v);
    inline FILogMessage& operator<<(uint64 v);
    inline FILogMessage& operator<<(bool v);
    inline FILogMessage& operator<<(float v);
    inline FILogMessage& operator<<(double v);
    inline FILogMessage& operator<<(const char *str);
	inline FILogMessage& operator<<(void *p);
    inline FILogMessage& operator<<(char v[]);
	inline FILogMessage& operator<<(const std::string& str);
	inline FILogMessage& operator<< (FILogMessage& (*_f)(FILogMessage&));
	friend FILogMessage& endl(FILogMessage& v);
protected:
    inline void _LogImpl(const std::string& message) { _LogImpl(message.c_str()); }
    inline void _LogImpl(const char* str) { _message += str; }
    virtual void _LogHandle() {}
    inline void Finish()
    {
        _LogHandle();
        _message.clear();
        if (F_LOGLEVEL::F_LOGLEVEL_FATAL == _level) abort();
    }
protected:
    friend class FLogFinisher;
    friend class FLogTraceFunction;
    std::string _message;
    F_LOGLEVEL _level;
};

inline FILogMessage& endl(FILogMessage& v)
{
	v._LogImpl("\n");
	return v;
}

#define TRMPLATE_METHOD(T) \
	FILogMessage& FILogMessage::operator<<(T v) \
	{ \
		std::stringstream str;	\
		str << v; \
        _LogImpl(str.str()); \
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

FILogMessage& FILogMessage::operator<<(const char* str)
{
	_LogImpl(str);
	return *this;
}
FILogMessage& FILogMessage::operator<<(const std::string& str)
{
	_LogImpl(str);
	return *this;
}
FILogMessage& FILogMessage::operator<<(char v[])
{
    _LogImpl((char*)v);
	return *this;
}
FILogMessage& FILogMessage::operator<<(void *p)
{
	_LogImpl(FFormat("%p", p));
	return *this;
}

FILogMessage& FILogMessage::operator<< (FILogMessage& (*_f)(FILogMessage&))
{
	return _f(*this);
}

_FStdEnd

_FStdBegin
class FLogFinisher
{
public:
	inline void operator=(FILogMessage &other)
    {
        other.Finish();
    }
};

class FLogTraceFunction
{
public:
    FLogTraceFunction(FILogMessage& other, const char *func, const char *file, int line)
    : _log(other)
    , _func(func)
	, _file(file)
	, _line(line){}
	~FLogTraceFunction()
    {
        char buff[100] = {0};
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
		<< FLogLevelName[F_LOGLEVEL::F_LOGLEVEL_TRACE] << "|" 
        << buff << "|"
		<< (_file ? _file : "<unknow source>") << ":" 
        << _line
        << "]"
        << _func << "() end "
        << endl;

        *this = _log;
    }
	void operator=(FILogMessage &other)
    {
        other.Finish();
    }
private:
    FILogMessage& _log;
	const char *_func;
	const char *_file;
	int _line;
};

_FStdEnd

#endif//__FLOG_HPP__