#ifndef __RZLOG_HPP__
#define __RZLOG_HPP__
#pragma once
#include <iostream>
#include <string>
#include <sstream> 
#include "RzType.hpp"
#include "RzFunc.hpp"

_RzStdBegin

enum RZ_LOGLEVEL 
{
	RZ_LOGLEVEL_TRACE,
	RZ_LOGLEVEL_DEBUG,
	RZ_LOGLEVEL_INFO,
	RZ_LOGLEVEL_WARN,
	RZ_LOGLEVEL_ERROR,
	RZ_LOGLEVEL_FATAL,
    RZ_LOGLEVEL_NUM_LOG_LEVELS
};

const char* RzLogLevelName[RZ_LOGLEVEL_NUM_LOG_LEVELS] =
{
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

class CRzGlobalLogLevel
{
public:
    static CRzGlobalLogLevel* get()
    {
        static CRzGlobalLogLevel hdl;
        return &hdl;
    }
    inline RZ_LOGLEVEL getLogLevel() const
    {
        return g_level;
    }
    inline void setLogLevel(RZ_LOGLEVEL level)
    {
        g_level = level;
    }
private:
    CRzGlobalLogLevel() : g_level(RZ_LOGLEVEL::RZ_LOGLEVEL_TRACE){}
    RZ_LOGLEVEL g_level;
};

class CRzLogFinisher;
class CRzLogTraceFunction;
class CRzILogMessage
{
public:
    CRzILogMessage(RZ_LOGLEVEL level) : _level(level){}
    CRzILogMessage(RZ_LOGLEVEL level, const char* filename, int line = -1)
    : _level(level)
    {
        char buff[100] = {0};
        tm* aTm = RzGetNowTime();
        sprintf(buff, "%-4d-%02d-%02d %02d:%02d:%02d",
            aTm->tm_year + 1900,
            aTm->tm_mon + 1,
            aTm->tm_mday,
            aTm->tm_hour,
            aTm->tm_min,
            aTm->tm_sec);

        *this << "[" 
		<< RzGetCurrentThreadId() << "|"
		<< RzLogLevelName[RZ_LOGLEVEL::RZ_LOGLEVEL_TRACE] << "|" 
        << buff << "|"
		<< (filename ? filename : "<unknow source>") << ":" 
        << line
        << "]";
    }
    virtual ~CRzILogMessage()
    {
        _message.clear();
    }
    template<typename T>
    inline CRzILogMessage& operator<<(T v); // will generate link error
    inline CRzILogMessage& operator<<(int8 v);
    inline CRzILogMessage& operator<<(int16 v);
    inline CRzILogMessage& operator<<(int32 v);
    inline CRzILogMessage& operator<<(int64 v);
    inline CRzILogMessage& operator<<(uint8 v);
    inline CRzILogMessage& operator<<(uint16 v);
    inline CRzILogMessage& operator<<(uint32 v);
    inline CRzILogMessage& operator<<(uint64 v);
    inline CRzILogMessage& operator<<(bool v);
    inline CRzILogMessage& operator<<(float v);
    inline CRzILogMessage& operator<<(double v);
    inline CRzILogMessage& operator<<(const char *str);
	inline CRzILogMessage& operator<<(void *p);
    inline CRzILogMessage& operator<<(char v[]);
	inline CRzILogMessage& operator<<(const std::string& str);
	inline CRzILogMessage& operator<< (CRzILogMessage& (*_f)(CRzILogMessage&));
	friend CRzILogMessage& endl(CRzILogMessage& v);
protected:
    inline void _LogImpl(const std::string& message) { _LogImpl(message.c_str()); }
    inline void _LogImpl(const char* str) { _message += str; }
    virtual void _LogHandle() {}
    inline void Finish()
    {
        _LogHandle();
        _message.clear();
        if (RZ_LOGLEVEL::RZ_LOGLEVEL_FATAL == _level) abort();
    }
protected:
    friend class CRzLogFinisher;
    friend class CRzLogTraceFunction;
    std::string _message;
    RZ_LOGLEVEL _level;
};

inline CRzILogMessage& endl(CRzILogMessage& v)
{
	v._LogImpl("\n");
	return v;
}

#define TRMPLATE_METHOD(T) \
	CRzILogMessage& CRzILogMessage::operator<<(T v) \
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

CRzILogMessage& CRzILogMessage::operator<<(const char* str)
{
	_LogImpl(str);
	return *this;
}
CRzILogMessage& CRzILogMessage::operator<<(const std::string& str)
{
	_LogImpl(str);
	return *this;
}
CRzILogMessage& CRzILogMessage::operator<<(char v[])
{
    _LogImpl((char*)v);
	return *this;
}
CRzILogMessage& CRzILogMessage::operator<<(void *p)
{
	_LogImpl(RzFormat("%p", p));
	return *this;
}

CRzILogMessage& CRzILogMessage::operator<< (CRzILogMessage& (*_f)(CRzILogMessage&))
{
	return _f(*this);
}

_RzStdEnd

_RzStdBegin
class CRzLogFinisher
{
public:
	inline void operator=(CRzILogMessage &other)
    {
        other.Finish();
    }
};

class CRzLogTraceFunction
{
public:
    CRzLogTraceFunction(CRzILogMessage& other, const char *func, const char *file, int line)
    : _log(other)
    , _func(func)
	, _file(file)
	, _line(line){}
	~CRzLogTraceFunction()
    {
        char buff[100] = {0};
        tm* aTm = RzGetNowTime();
        sprintf(buff, "%-4d-%02d-%02d %02d:%02d:%02d",
            aTm->tm_year + 1900,
            aTm->tm_mon + 1,
            aTm->tm_mday,
            aTm->tm_hour,
            aTm->tm_min,
            aTm->tm_sec);

        _log << "[" 
		<< RzGetCurrentThreadId() << "|"
		<< RzLogLevelName[RZ_LOGLEVEL::RZ_LOGLEVEL_TRACE] << "|" 
        << buff << "|"
		<< (_file ? _file : "<unknow source>") << ":" 
        << _line
        << "]"
        << _func << "() end "
        << endl;

        *this = _log;
    }
	void operator=(CRzILogMessage &other)
    {
        other.Finish();
    }
private:
    CRzILogMessage& _log;
	const char *_func;
	const char *_file;
	int _line;
};

_RzStdEnd

#endif//__RZLOG_HPP__