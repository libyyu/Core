#ifndef __FCONSOLE_HPP
#define __FCONSOLE_HPP
#pragma once
#include "FLock.hpp"
#include "FFunc.hpp"
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <sstream> 
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
#include <io.h>
#include <fcntl.h>
#endif

_FStdBegin
static FLIB_LOGLEVEL _console_level = FLIB_LOGLEVEL::FLIB_LOGLEVEL_DEBUG;
class FConsoleFinisher;
class FConsoleTraceFunction;
class FConsole
{
	class _FConsoleHandle
	{
		bool 	  m_isAlloced;
		FLock     m_lock;
	public:
		//Console
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
		HANDLE 	 	m_stdOutputHandle;
		HANDLE 	 	m_stdErrHandle;
#endif
	protected:
		_FConsoleHandle() : m_isAlloced(false)
		{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
			m_stdOutputHandle = 0;
			m_stdErrHandle = 0;
#endif
		}
	public:
		static _FConsoleHandle* get()
		{
			static _FConsoleHandle hdl;
			return &hdl;
		}
	
		void RedirectIOToConsole( )
		{
			lock_wrapper lock(&m_lock);
			if(m_isAlloced)
				return ;
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
			// 分配一个控制台，以便于输出一些有用的信息
			AllocConsole();
			// 取得 STDOUT 的文件系统
			m_stdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
			m_stdErrHandle = GetStdHandle( STD_ERROR_HANDLE );
			// Redirect STDOUT to the new console by associating STDOUT's file 
			// descriptor with an existing operating-system file handle.
			if (m_stdOutputHandle)
			{
				int hConsoleHandle = _open_osfhandle((intptr_t)m_stdOutputHandle, _O_TEXT);
				FILE *pFile = _fdopen(hConsoleHandle, "w");
				*stdout = *pFile;
				setvbuf(stdout, NULL, _IONBF, 0);
			}
			if (m_stdErrHandle)
			{
				int hConsoleErrHandle = _open_osfhandle((intptr_t)m_stdErrHandle, _O_TEXT);
				FILE *pFileErr = _fdopen(hConsoleErrHandle, "w");
				*stderr = *pFileErr;
				setvbuf(stderr, NULL, _IONBF, 0);
			}
			// 这个调用确保 iostream 和 C run-time library 的操作在源代码中有序。 
			std::ios::sync_with_stdio();
#endif
			m_isAlloced = true;
		}
	};
public:
	FConsole(FLIB_LOGLEVEL level):m_level(level)
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
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
	FConsole(FLIB_LOGLEVEL level, const char* filename, int32 line = -1):m_level(level)
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
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
	virtual ~FConsole() { m_message.clear(); }
public:
	inline FConsole& operator<<(char v[]);
	inline FConsole& operator<< (FConsole& (*_f)(FConsole&));
	friend FConsole& endl(FConsole& v);
	template<typename T>
    inline FConsole& operator<< (T v); // will generate link error
#define TRMPLATE_DECLARE(T) \
    inline FConsole& operator<< (T v);

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
private:
	FLock     m_lock;
	std::string m_message;
    FLIB_LOGLEVEL m_level;
	friend class FConsoleFinisher;
    friend class FConsoleTraceFunction;
protected:
	inline void _Logout(const char* str) { m_message += str; }
    inline void Finish()
    {
        _LogImpl();
        m_message.clear();
    }
	virtual void _LogImpl()
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
		lock_wrapper lock(&m_lock);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
		if (m_level == FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN);
		}
		else if (m_level > FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdErrHandle, FOREGROUND_RED);
		}
		if (m_level <= FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			std::cout << m_message.c_str();
		}
		else
		{
			std::cerr << m_message.c_str();
		}
		if (m_level == FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		else if (m_level > FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdErrHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
#else
		const int MAX_BUFFLEN = 20480;
		char outbuff[MAX_BUFFLEN+1] = { 0 };
		if (m_level == FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			sprintf(outbuff, "\e[1;33m%s\e[0m", m_message.c_str()); 
			std::cout << outbuff;
		}
		else if(m_level > FLIB_LOGLEVEL::FLIB_LOGLEVEL_WARN)
		{
			sprintf(outbuff, "\e[1;31m%s\e[0m", m_message.c_str()); 
			std::cerr << outbuff;
		}
		else
		{
			std::cout << m_message.c_str();
		}
#endif
	}
};

inline FConsole& endl(FConsole& v)
{
	v._Logout("\n");
	return v;
}

FConsole& FConsole::operator<<(const char* str)
{
	_Logout(str);
	return *this;
}
FConsole& FConsole::operator<<(const std::string& str)
{
	_Logout(str.c_str());
	return *this;
}
FConsole& FConsole::operator<<(char v[])
{
    _Logout((char*)v);
	return *this;
}
FConsole& FConsole::operator<<(void *p)
{
	_Logout(FFormat("%p", p).c_str());
	return *this;
}
FConsole& FConsole::operator<< (FConsole& (*_f)(FConsole&))
{
	return _f(*this);
}

#define TRMPLATE_METHOD(T) \
	FConsole& FConsole::operator<<(T v) \
	{ \
		if(m_level<_console_level) \
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
class FConsoleFinisher
{
public:
	inline void operator=(FConsole &other)
    {
        other.Finish();
    }
};

class FConsoleTraceFunction
{
public:
    FConsoleTraceFunction(FConsole& other, const char *func, const char *file, int32 line)
    : _log(other)
    , _func(func)
	, _file(file)
	, _line(line){}
	~FConsoleTraceFunction()
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
	void operator=(FConsole &other)
    {
        other.Finish();
    }
private:
    FConsole& _log;
	const char *_func;
	const char *_file;
	int32 _line;
};

_FStdEnd
/////////////////////////////////////////////////////////////////////////////////////////
////
#define F_CONSOLE(LEVEL) \
	FStd::FConsoleFinisher() = FStd::FConsole(FStd::FLIB_LOGLEVEL::FLIB_LOGLEVEL_##LEVEL)

#define F_CONSOLE_TRACE  \
	FStd::FConsole f_console_trace(FStd::FLIB_LOGLEVEL::FLIB_LOGLEVEL_TRACE, __FILE__, __LINE__);  \
	FStd::FConsoleTraceFunction f_consoleTraceFunction(f_console_trace, __FUNCTION__, __FILE__, __LINE__); \
	f_consoleTraceFunction = f_console_trace << __FUNCTION__ << "() enter " << endl;

#endif//__FCONSOLE_HPP
