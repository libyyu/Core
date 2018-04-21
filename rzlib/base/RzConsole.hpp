#ifndef __RZCONSOLE_HPP
#define __RZCONSOLE_HPP
#pragma once
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <string>
#include <sstream> 
#include "RzLock.hpp"
#include "RzLog.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif
_RzStdBegin


class CRzConsole : public CRzILogMessage
{
	class _RzConsoleHandle
	{
		bool 		_isAlloced;
		CRzLock     m_lock;
		//Console
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		HANDLE 	 	m_stdOutputHandle;
		HANDLE 	 	m_stdErrHandle;
#endif
	protected:
		_RzConsoleHandle() : _isAlloced(false)
		{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
			m_stdOutputHandle = 0;
			m_stdErrHandle = 0;
#endif
		}
	public:
		static _RzConsoleHandle* get()
		{
			static _RzConsoleHandle hdl;
			return &hdl;
		}
	
		void RedirectIOToConsole( )
		{
			lock_wrapper lock(&m_lock);
			if(_isAlloced)
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
			_isAlloced = true;
		}
	};
public:
	CRzConsole(RZ_LOGLEVEL level) : CRzILogMessage(level)
	{
		_RzConsoleHandle::get()->RedirectIOToConsole();
	}
	CRzConsole(RZ_LOGLEVEL level, const char* filename, int line = -1)
		:CRzILogMessage(level, filename, line)
	{
		_RzConsoleHandle::get()->RedirectIOToConsole();
	}
	virtual ~CRzConsole() { }
private:
	CRzLock     m_lock;
protected:
    virtual void _LogHandle()
	{
		_RzConsoleHandle::get()->RedirectIOToConsole();
		lock_wrapper lock(&m_lock);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	if (_level == RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN);
	}
	else if (_level > RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED);
	}
	if (_level <= RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		std::cout << _message.c_str();
	}
	else
	{
		std::cerr << _message.c_str();
	}
	if (_level == RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	else if (_level > RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
#else
	const int MAX_BUFFLEN = 20480;
	char outbuff[MAX_BUFFLEN+1] = { 0 };
	if (_level == RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
	{
		sprintf(outbuff, "\e[1;33m%s\e[0m", _message.c_str()); 
		std::cout << outbuff;
	}
	else if(_level > RZ_LOGLEVEL::RZ_LOGLEVEL_WARN)
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
_RzStdEnd

_RzStdBegin
#define RZ_CONSOLE(LEVEL) \
	CRzLogFinisher() = CRzConsole(RZ_LOGLEVEL::RZ_LOGLEVEL_##LEVEL)

#define RZ_CONSOLE_TRACE  \
	CRzConsole rz_console_trace(RZ_LOGLEVEL::RZ_LOGLEVEL_TRACE, __FILE__, __LINE__);  \
	CRzLogTraceFunction rz_logTraceFunction(rz_console_trace, __FUNCTION__, __FILE__, __LINE__); \
	rz_logTraceFunction = rz_console_trace << __FUNCTION__ << "() begin " << endl;

_RzStdEnd

#endif//__RZCONSOLE_HPP