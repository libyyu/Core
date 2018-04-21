#ifndef __RZCONSOLE_HPP
#define __RZCONSOLE_HPP
#pragma once
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <string>
#include <sstream> 
#include "RzLock.hpp"
#include "RzFunc.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif
_RzStdBegin

enum ECONSOLE_LOG_TYPE
{
	ECONSOLE_LOG_ERROR,
	ECONSOLE_LOG_WARN,
	ECONSOLE_LOG_INFO,
};

class CRzConsole
{
	class _RzConsoleHandle
	{
		bool _isAlloced;
	protected:
		_RzConsoleHandle() : _isAlloced(false){}
	public:
		static _RzConsoleHandle* get()
		{
			static _RzConsoleHandle hdl;
			return &hdl;
		}
		bool isAlloced() { return _isAlloced; }
		void setAlloced() { _isAlloced = true; }
	};
public:
	CRzConsole(ECONSOLE_LOG_TYPE level, const char* title = NULL) 
	{
		m_level = level;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		m_stdOutputHandle = 0;
		m_stdErrHandle = 0;
#endif
		_RedirectIOToConsole();
		if(title)
		{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
			::SetConsoleTitleA(title);
#endif
		}
	}
	
private:
	ECONSOLE_LOG_TYPE 	m_level;
	CRzLock     m_lock;
	//Console
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	HANDLE 	 	m_stdOutputHandle;
	HANDLE 	 	m_stdErrHandle;
#endif
public:
	template<typename T>
    inline CRzConsole& operator<<(T v); // will generate link error
    inline CRzConsole& operator<<(int8 v);
    inline CRzConsole& operator<<(int16 v);
    inline CRzConsole& operator<<(int32 v);
    inline CRzConsole& operator<<(int64 v);
    inline CRzConsole& operator<<(uint8 v);
    inline CRzConsole& operator<<(uint16 v);
    inline CRzConsole& operator<<(uint32 v);
    inline CRzConsole& operator<<(uint64 v);
    inline CRzConsole& operator<<(bool v);
    inline CRzConsole& operator<<(float v);
    inline CRzConsole& operator<<(double v);
    inline CRzConsole& operator<<(const char *str);
	inline CRzConsole& operator<<(const std::string& str);
	inline CRzConsole& operator<< (CRzConsole& (*_f)(CRzConsole&));
	friend CRzConsole& endl(CRzConsole& v);
protected:
	inline void _LogImpl(const char* message);
	inline void _RedirectIOToConsole( );
};

void CRzConsole::_RedirectIOToConsole( )
{
	lock_wrapper lock(&m_lock);
	if(_RzConsoleHandle::get()->isAlloced())
		return ;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
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
	_RzConsoleHandle::get()->setAlloced();
}

void CRzConsole::_LogImpl(const char* message)
{
	if (!message)
		return;
	_RedirectIOToConsole();

	lock_wrapper lock(&m_lock);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	if (m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_WARN)
	{
		::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN);
	}
	else if (m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_ERROR)
	{
		::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED);
	}
	if (m_level != ECONSOLE_LOG_TYPE::ECONSOLE_LOG_ERROR)
	{
		std::cout << message;
	}
	else
	{
		std::cerr << message;
	}
	if (m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_WARN)
	{
		::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	else if (m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_ERROR)
	{
		::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
#else
	const int MAX_BUFFLEN = 20480;
	char outbuff[MAX_BUFFLEN+1] = { 0 };
	if (m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_WARN)
	{
		sprintf(outbuff, "\e[1;33m%s\e[0m", message); 
		std::cout << outbuff;
	}
	else if(m_level == ECONSOLE_LOG_TYPE::ECONSOLE_LOG_ERROR)
	{
		sprintf(outbuff, "\e[1;31m%s\e[0m", message); 
		std::cerr << outbuff;
	}
	else
	{
		std::cout << message;
	}
#endif


#if PLATFORM_TARGET == PLATFORM_WINDOWS
#ifdef _DEBUG
	OutputDebugStringA(message);
#endif
#endif
}

inline CRzConsole& operator<<(CRzConsole& str,const std::string &v)
{
    (str) << v.c_str();
    return str;
}

#define TRMPLATE_METHOD(T) \
	CRzConsole& CRzConsole::operator<<(T v) \
	{ \
		std::stringstream str;	\
		str << v; \
		_LogImpl(str.str().c_str()); \
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
CRzConsole& CRzConsole::operator<<(const char* str)
{
	_LogImpl(str);
	return *this;
}
CRzConsole& CRzConsole::operator<<(const std::string& str)
{
	_LogImpl(str.c_str());
	return *this;
}

CRzConsole& CRzConsole::operator<< (CRzConsole& (*_f)(CRzConsole&))
{
	return _f(*this);
}
CRzConsole& endl(CRzConsole& v)
{
	v << "\n";
	return v;
}
_RzStdEnd

_RzStdBegin
#define RZ_CONSOLE(LEVEL) CRzConsole(ECONSOLE_LOG_##LEVEL)
#define RZ_CONSOLE_TITLE(LEVEL, title) CRzConsole(ECONSOLE_LOG_##LEVEL, title)
_RzStdEnd

#endif//__RZCONSOLE_HPP