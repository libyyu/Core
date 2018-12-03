#ifndef __FCONSOLE_HPP
#define __FCONSOLE_HPP
#pragma once
#include "FLock.hpp"
#include "FLogInterface.hpp"
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <sstream> 
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif
_FStdBegin


class FConsole : public FLogInterface
{
	class _FConsoleHandle
	{
		bool 	  m_isAlloced;
		FLock     m_lock;
	public:
		//Console
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		HANDLE 	 	m_stdOutputHandle;
		HANDLE 	 	m_stdErrHandle;
#endif
	protected:
		_FConsoleHandle() : m_isAlloced(false)
		{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
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
			m_isAlloced = true;
		}
	};
public:
	FConsole(F_LOGLEVEL level) : FLogInterface(level)
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
	}
	FConsole(F_LOGLEVEL level, const char* filename, int line = -1)
	:FLogInterface(level, filename, line)
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
	}
	virtual ~FConsole() { }
private:
	FLock     m_lock;
protected:
    virtual void _LogImpl()
	{
		_FConsoleHandle::get()->RedirectIOToConsole();
		lock_wrapper lock(&m_lock);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	if (_level == F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN);
	}
	else if (_level > F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdErrHandle, FOREGROUND_RED);
	}
	if (_level <= F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		std::cout << _message.c_str();
	}
	else
	{
		std::cerr << _message.c_str();
	}
	if (_level == F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	else if (_level > F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(_FConsoleHandle::get()->m_stdErrHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
#else
	const int MAX_BUFFLEN = 20480;
	char outbuff[MAX_BUFFLEN+1] = { 0 };
	if (_level == F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		sprintf(outbuff, "\e[1;33m%s\e[0m", _message.c_str()); 
		std::cout << outbuff;
	}
	else if(_level > F_LOGLEVEL::F_LOGLEVEL_WARN)
	{
		sprintf(outbuff, "\e[1;31m%s\e[0m", _message.c_str()); 
		std::cerr << outbuff;
	}
	else
	{
		std::cout << _message.c_str();
	}
#endif
	}
};
_FStdEnd
//////////////////////////////////////////////////////////////////////
_FStdBegin
#define F_CONSOLE(LEVEL) \
	FLogFinisher() = FConsole(F_LOGLEVEL::F_LOGLEVEL_##LEVEL)

#define F_CONSOLE_TRACE  \
	FConsole f_console_trace(F_LOGLEVEL::F_LOGLEVEL_TRACE, __FILE__, __LINE__);  \
	FLogTraceFunction f_logTraceFunction(f_console_trace, __FUNCTION__, __FILE__, __LINE__); \
	f_logTraceFunction = f_console_trace << __FUNCTION__ << "() enter " << endl;

_FStdEnd

#endif//__FCONSOLE_HPP