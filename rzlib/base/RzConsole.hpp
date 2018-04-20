#ifndef __RZCONSOLE_HPP
#define __RZCONSOLE_HPP
#pragma once
#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <string>
#include "RzLock.hpp"
#include "RzFunc.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif
_RzStdBegin
enum ELogOutput
{
	ELOG_NONE     = 0,
    ELOG_CONSOLE  = 1,
    ELOG_FILE 	  = 2,
    ELOG_VCOUTPUT = 4,
	ELOG_CALLBACK = 8,
};
enum ELOG_TYPE
{
	ELOG_ERROR = 0,
	ELOG_ASSERT = 1,
	ELOG_WARNING = 2,
	ELOG_INFO = 3,
	ELOG_EXCEPTION = 4,
};

class CRzConsole
{
public:
	typedef void(*PConsoleOutCallBack)(int, const char*);
protected:
	static CRzConsole* GetInstance() {
		static CRzConsole console;
		return &console;
	}
	void _SetMask(int mask)
	{
		lock_wrapper lock(&m_Lock);
		{
			m_Mask = mask;
		}
	}
	void _SetCallback(PConsoleOutCallBack callback)
	{
		lock_wrapper lock(&m_Lock);
		{
			m_callback = callback;
		}
	}
private:
	CRzConsole() {
		m_Mask = ELOG_CONSOLE | ELOG_VCOUTPUT;
		m_callback = NULL;
		m_bConsoleInit = false;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		m_stdOutputHandle = 0;
		m_stdErrHandle = 0;
#endif
		m_bFileInit = false;
		m_pFile = NULL;
	}
    virtual ~CRzConsole()
    {
        if(m_bFileInit)
        {
            if(m_pFile)
            {
                fclose(m_pFile);
                m_pFile = NULL;
            }
            m_bFileInit = false;
        }
        m_callback = NULL;
		m_bConsoleInit = false;
    }
	
private:
	//Lock
	CRzLock   	m_Lock;
	//Mask
	int		    m_Mask;
	//callback
	PConsoleOutCallBack  m_callback;
	//Console
	bool  	 	m_bConsoleInit;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	HANDLE 	 	m_stdOutputHandle;
	HANDLE 	 	m_stdErrHandle;
#endif
	//File
	bool   		m_bFileInit;
	FILE*       m_pFile;
public:
	static void SetMask(int mask)
	{
		CRzConsole::GetInstance()->_SetMask(mask);
	}
	static void SetCallback(void* callback)
	{
		CRzConsole::GetInstance()->_SetCallback((PConsoleOutCallBack)callback);
	}
	static void Log(const char* format,...)
	{
		va_list va;
		va_start(va, format);
		CRzConsole::GetInstance()->_LogFormatImpl(ELOG_INFO, format, va);
		va_end(va);
	}
	static void LogWarning(const char* format,...)
	{
		va_list va;
		va_start(va, format);
		CRzConsole::GetInstance()->_LogFormatImpl(ELOG_WARNING, format, va);
		va_end(va);
	}
	static void LogError(const char* format,...)
	{
		va_list va;
		va_start(va, format);
		CRzConsole::GetInstance()->_LogFormatImpl(ELOG_ERROR, format, va);
		va_end(va);
	}
	static void LogException(const char* format,...)
	{
		va_list va;
		va_start(va, format);
		CRzConsole::GetInstance()->_LogFormatImpl(ELOG_EXCEPTION, format, va);
		va_end(va);
	}

	static void SetTitle(const char* title)
	{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		::SetConsoleTitleA(title);
#endif
	}
protected:
	void _LogFormatImpl(ELOG_TYPE logType, const char* format, va_list va)
	{
		if (!format)
			return;

		if(m_Mask & ELOG_CONSOLE)
			_RedirectIOToConsole();

		if(m_Mask & ELOG_FILE)
			_RedirectIOToFile();

		lock_wrapper lock(&m_Lock);
		static const char* pLevel[] = {"Error","Assert","Warning","Info","Exception"};
        const int MAX_BUFFLEN = 20480;
		char buff[MAX_BUFFLEN] = { 0 };
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		_vsnprintf_s(buff, MAX_BUFFLEN, format, va);
#else
		vsnprintf(buff, MAX_BUFFLEN, format, va);
#endif
		if(m_Mask & ELOG_CONSOLE)
		{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
            char outbuff[MAX_BUFFLEN] = { 0 };
			sprintf(outbuff, "[%s]%s", pLevel[logType], buff);
			if (logType == ELOG_WARNING)
			{
				::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN);
			}
			else if (logType != ELOG_INFO)
			{
				::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED);
			}
            if (logType == ELOG_INFO || logType == ELOG_WARNING)
			{
				std::cout << outbuff;
			}
			else
			{
				std::cerr << outbuff;
			}
            if (logType == ELOG_WARNING)
			{
				::SetConsoleTextAttribute(m_stdOutputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
			else if (logType != ELOG_INFO)
			{
				::SetConsoleTextAttribute(m_stdErrHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
#else
            char outbuff[MAX_BUFFLEN] = { 0 };
            if (logType == ELOG_WARNING)
            {
                sprintf(outbuff, "\e[1;33m[%s]%s\e[0m", pLevel[logType], buff); 
                std::cout << outbuff;
            }
            else if(logType != ELOG_INFO)
            {
                sprintf(outbuff, "\e[1;31m[%s]%s\e[0m", pLevel[logType], buff); 
                std::cerr << outbuff;
            }
            else
            {
                sprintf(outbuff, "[%s]%s", pLevel[logType], buff); 
                std::cout << outbuff;
            }
#endif
		}

		if(m_bFileInit && (m_Mask & ELOG_FILE))
		{
			fprintf(m_pFile, "[%s]%s", pLevel[logType], buff);
		}
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		if(m_Mask & ELOG_VCOUTPUT)
		{
			char vsoutbuff[MAX_BUFFLEN] = { 0 };
			sprintf(vsoutbuff, "[%s]%s", pLevel[logType], buff);
			OutputDebugStringA(vsoutbuff);
		}
#endif

		if (m_callback && (m_Mask & ELOG_CALLBACK))
		{
			m_callback(logType, buff);
		}
	}

	void _RedirectIOToConsole( )
	{
		if(m_bConsoleInit)
			return ;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		lock_wrapper lock(&m_Lock);
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
		m_bConsoleInit = true;
	}

	void _RedirectIOToFile()
	{
		if(m_bFileInit)
			return;
		lock_wrapper lock(&m_Lock);
		m_pFile = fopen("_console_$_$_.log", "wt");
		if(m_pFile)
		{
			tm* aTm = RzGetNowTime();

			fprintf(m_pFile, "CREATED ON %-4d-%02d-%02d %02d:%02d:%02d\n\n", aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
			fflush(m_pFile);

			m_bFileInit = true;
		}
	}
};
_RzStdEnd

#endif//__RZCONSOLE_HPP