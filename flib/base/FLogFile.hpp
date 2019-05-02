#ifndef __FLOGFILE_HPP__
#define __FLOGFILE_HPP__
#pragma once
#include "FLock.hpp"
#include "FFunc.hpp"
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <sstream> 

_FStdBegin
class FAutoFile
{
public:
    FAutoFile(FILE* fp):_file(fp){}
    FAutoFile(const char* path)
    {
        _file = fopen(path, "w");
        assert(_file && "create log file failed!");
    }
    ~FAutoFile()
    {
        if(_file){
            fflush(_file);
            fclose(_file);
        }
    }
    inline operator FILE*(){ return _file; }
    inline operator FLock*() { return &_lock; }
private:
    FILE* _file;
    FLock _lock;
};
static FLIB_LOGLEVEL _logfile_level = FLIB_LOGLEVEL::FLIB_LOGLEVEL_DEBUG;
class FLogFileFinisher;
class FLogFileTraceFunction;
class FLogFile
{
public:
    FLogFile(FAutoFile& fp, FLIB_LOGLEVEL level) : m_level(level), _flog(fp)
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
        << FLIB_LogLevelName[m_level] << "|" 
		<< FGetCurrentThreadId() << "|"
        << buff << "|"
        << "]";
	}
	FLogFile(FAutoFile& fp, FLIB_LOGLEVEL level, const char* filename, int32 line = -1)
	:m_level(level), _flog(fp)
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
        << FLIB_LogLevelName[m_level] << "|" 
		<< FGetCurrentThreadId() << "|"
        << buff << "|"
		<< (filename ? filename : "<unknow source>") << ":" 
        << line
        << "]";
	}
    virtual ~FLogFile() { m_message.clear(); }
public:
    inline FLogFile& operator<<(char v[]);
	inline FLogFile& operator<< (FLogFile& (*_f)(FLogFile&));
	friend FLogFile& endl(FLogFile& v);
	template<typename T>
    inline FLogFile& operator<< (T v); // will generate link error
#define TRMPLATE_DECLARE(T) \
    inline FLogFile& operator<< (T v);

    TRMPLATE_DECLARE(int8)
    TRMPLATE_DECLARE(int16)
    TRMPLATE_DECLARE(int32)
    TRMPLATE_DECLARE(int64)
    TRMPLATE_DECLARE(uint8)
    TRMPLATE_DECLARE(uint16)
    TRMPLATE_DECLARE(uint32)
    TRMPLATE_DECLARE(uint64)
#if FLIB_COMPILER_64BITS
    TRMPLATE_DECLARE(int)
    TRMPLATE_DECLARE(uint)
#endif
    TRMPLATE_DECLARE(bool)
    TRMPLATE_DECLARE(float)
    TRMPLATE_DECLARE(double)
    TRMPLATE_DECLARE(const char *)
    TRMPLATE_DECLARE(void *)
    TRMPLATE_DECLARE(const std::string&)
#undef TRMPLATE_DECLARE
protected:
    inline void _Logout(const char* str) { m_message += str; }
    inline void Finish()
    {
        _LogImpl();
        m_message.clear();
    }
    virtual void _LogImpl()
    {
        lock_wrapper lock(_flog);
        FILE* fp = _flog;
        assert(fp && "log file handle is null.");
        fwrite(m_message.c_str(), m_message.size(), sizeof(char), fp);
    }
protected:
    FAutoFile& _flog;
    FLock     m_lock;
	std::string m_message;
    FLIB_LOGLEVEL m_level;
    friend class FLogFileFinisher;
    friend class FLogFileTraceFunction;
};

inline FLogFile& endl(FLogFile& v)
{
	v._Logout("\n");
	return v;
}

FLogFile& FLogFile::operator<<(const char* str)
{
	_Logout(str);
	return *this;
}
FLogFile& FLogFile::operator<<(const std::string& str)
{
	_Logout(str.c_str());
	return *this;
}
FLogFile& FLogFile::operator<<(char v[])
{
    _Logout((char*)v);
	return *this;
}
FLogFile& FLogFile::operator<<(void *p)
{
	_Logout(FFormat("%p", p).c_str());
	return *this;
}
FLogFile& FLogFile::operator<< (FLogFile& (*_f)(FLogFile&))
{
	return _f(*this);
}

#define TRMPLATE_METHOD(T) \
	FLogFile& FLogFile::operator<<(T v) \
	{ \
        if(m_level<_logfile_level) \
			return *this; \
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
#if FLIB_COMPILER_64BITS
    TRMPLATE_METHOD(int)
    TRMPLATE_METHOD(uint)
#endif
TRMPLATE_METHOD(float)
TRMPLATE_METHOD(double)
TRMPLATE_METHOD(bool)
#undef TRMPLATE_METHOD
/////////////////////////////////////////////////////////////////////////////////////////
////
class FLogFileFinisher
{
public:
	inline void operator=(FLogFile &other)
    {
        other.Finish();
    }
};

class FLogFileTraceFunction
{
public:
    FLogFileTraceFunction(FLogFile& other, const char *func, const char *file, int32 line)
    : _log(other)
    , _func(func)
	, _file(file)
	, _line(line){}
	~FLogFileTraceFunction()
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
        << FLIB_LogLevelName[_log.m_level] << "|" 
		<< FGetCurrentThreadId() << "|"
        << buff << "|"
		<< (_file ? _file : "<unknow source>") << ":" 
        << _line
        << "]"
        << _func << "() leave "
        << endl;

        *this = _log;
    }
	void operator=(FLogFile &other)
    {
        other.Finish();
    }
private:
    FLogFile& _log;
	const char *_func;
	const char *_file;
	int32 _line;
};
_FStdEnd

//////////////////////////////////////////////////////////////////////
#define F_LOGFILE(LEVEL, file) \
	FStd::FLogFileFinisher() = FStd::FLogFile(file, FStd::FLIB_LOGLEVEL::FLIB_LOGLEVEL_##LEVEL)

#define F_LOGFILE_TRACE(file)  \
	FStd::FLogFile f_logfile_trace(file, FStd::FLIB_LOGLEVEL::FLIB_LOGLEVEL_TRACE, __FILE__, __LINE__);  \
	FStd::FLogFileTraceFunction f_logfileTraceFunction(f_logfile_trace, __FUNCTION__, __FILE__, __LINE__); \
	f_logfileTraceFunction = f_logfile_trace << __FUNCTION__ << "() enter " << endl;

#endif//__FLOGFILE_HPP__