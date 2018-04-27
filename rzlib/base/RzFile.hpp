#ifndef __RZFILE_HPP__
#define __RZFILE_HPP__
#pragma once
#include "RzConsole.hpp"
#include <stdio.h>
#include <functional>
#include <memory>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <windows.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <direct.h>
#else
#include <fcntl.h>   
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#endif
_RzStdBegin

class CRzFile
{
public:
	enum ENUM_SEEK { SEEK_FILE_BEGIN = 0, SEEK_FILE_CURRENT = 1, SEEK_FILE_END = 2 };
    CRzFile():
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		_file(INVALID_HANDLE_VALUE)
#else
		_file(-1)
#endif
	{
		_filename[0] = '\0';
	}
	virtual ~CRzFile()
	{
		Close();
	}
public:
    inline int Open(const char* filename, bool readonly = true);
    inline int Close();
    inline int Flush();
    inline long GetSize();
    inline long GetOffset();	
    inline bool IsEOF();
    inline int SetEOF();
    inline int Seek(int offset, unsigned int mode);
    inline int Create(const char* filename);
    inline int Delete();

    inline long ReadAll(void* p_buffer);
    inline long Read(void* p_buffer, unsigned long n_bytes_2_read);
    inline long Write(const void* p_buffer, unsigned long n_bytes_2_write);

    inline operator bool() const 
    {
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        return _file != INVALID_HANDLE_VALUE;
#else
        return _file != -1;
#endif
    }
private:
    bool _readonly;
    char _filename[100];
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    HANDLE _file;
#else
    int _file;
#endif
};

typedef std::shared_ptr<CRzFile> spRzFileT;

int CRzFile::Open(const char* filename, bool readonly)
{
    assert(filename != NULL);
    Close();
    _readonly = readonly;
    if(readonly)
    {
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        if (readonly)
			_file = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (_file == INVALID_HANDLE_VALUE) 
		{
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", ::GetLastError());
            return -1;
        }
#else
        if((_file = open(filename, O_RDONLY)) < 0)
        {
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", errno);
            return -1;
        }
#endif
    }
    else
    {
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        _file = ::CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (_file == INVALID_HANDLE_VALUE) 
		{
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", ::GetLastError());
            return -1;
        }
#else
        if((_file = open(filename, O_RDWR)) < 0)
        {
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", errno);
            return -1;
        }
#endif
    }
    strcpy(_filename, filename);
    return 0;
}

int CRzFile::Close()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    if (_file != INVALID_HANDLE_VALUE)
    {
        if(!::CloseHandle(_file))
        {
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed close file, errno = %d \n", ::GetLastError());
            return -1;
        }
        _file = INVALID_HANDLE_VALUE;
    }
#else
    if(_file != -1)
    {
        if(close(_file) <0)
        {
            RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed close file, errno = %d \n", errno);
            return -1;
        }    
        _file = -1;
    }
#endif
    return 0;
}

int CRzFile::Flush()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return -1;
    return ::FlushFileBuffers(_file) ? 0 : -1;
#else
    assert(_file != -1);
    if(_file == -1) return -1;
#endif
    return 0;
}

long CRzFile::GetSize()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return 0;
    return static_cast<long>(::GetFileSize(_file, NULL));
#else
    assert(_file != -1);
    if(_file == -1) return 0;
    long offset = GetOffset();
    long filelen = lseek(_file,0L,SEEK_END);    
    lseek(_file,offset,SEEK_SET);
    return filelen;
#endif
}
long CRzFile::GetOffset()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return 0;
    return static_cast<long>(::SetFilePointer(_file, 0, NULL, SEEK_CUR));
#else
    assert(_file != -1);
    if(_file == -1) return 0;
    return lseek(_file, 0, SEEK_CUR);
#endif
}

bool CRzFile::IsEOF()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return true;
	return GetOffset() >= GetSize();
#else
    assert(_file != -1);
    if(_file == -1) return true;
    return GetOffset() >= GetSize();
#endif
}

int CRzFile::SetEOF()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return -1;
    if(!::SetEndOfFile(_file)) return -1;
#else
    assert(_file != -1);
    if(_file == -1) return -1;
    if(lseek(_file,0L,SEEK_END) <0) return -1; 
#endif
    return 0;
}

int CRzFile::Seek(int offset, unsigned int mode)
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return -1;
    switch(mode)
    {
		case ENUM_SEEK::SEEK_FILE_BEGIN:
            ::SetFilePointer(_file, offset, NULL, SEEK_SET);
            break;
		case ENUM_SEEK::SEEK_FILE_CURRENT:
            ::SetFilePointer(_file, offset, NULL, SEEK_CUR);
            break;
		case ENUM_SEEK::SEEK_FILE_END:
            ::SetFilePointer(_file, offset, NULL, SEEK_END);
            break;
        default:
            assert(0);
            break;
    }
#else
    assert(_file != -1);
    if(_file == -1) return -1;
    switch(mode)
    {
		case ENUM_SEEK::SEEK_FILE_BEGIN:
            lseek(_file, offset, SEEK_SET);
            break;
		case ENUM_SEEK::SEEK_FILE_CURRENT:
            lseek(_file, offset, SEEK_CUR);
            break;
		case ENUM_SEEK::SEEK_FILE_END:
            lseek(_file, offset, SEEK_END);
            break;
        default:
            assert(0);
            break;
    }
#endif
    return 0;
}
long CRzFile::ReadAll(void* p_buffer)
{
    long nSize = GetSize();
    return Read(p_buffer, nSize);
}
long CRzFile::Read(void* p_buffer, unsigned long n_bytes_2_read)
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return 0;
    unsigned long n_total_bytes_read = 0;
	unsigned long n_bytes_left = n_bytes_2_read;
	BOOL b_result = TRUE;
	unsigned char* p_buf = (unsigned char*)p_buffer;

	bool has_read = false;
	DWORD n_bytes_read = 1; //has read
	while ((n_bytes_left > 0) && (n_bytes_read > 0) && b_result)
    {
        b_result = ::ReadFile(_file, &p_buf[n_bytes_2_read - n_bytes_left], n_bytes_left, &n_bytes_read, NULL);
		if (b_result == TRUE)
        {
            n_bytes_left -= n_bytes_read;
            n_total_bytes_read += n_bytes_read;
			has_read = true;
        }
    }
    n_bytes_read = n_total_bytes_read;
	return has_read ? (long)n_bytes_read : 0;
#else
    assert(_file != -1);
    if(_file == -1) return 0;
    return read(_file, p_buffer, n_bytes_2_read);  
#endif
}
long CRzFile::Write(const void* p_buffer, unsigned long n_bytes_2_write)
{
    if(_readonly) { assert(0 && "Can not write for a readonly file"); return 0; }
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    assert(_file);
    if(!_file) return 0;
	DWORD p_bytes_written = 0;
    ::WriteFile(_file, p_buffer, n_bytes_2_write, &p_bytes_written, NULL);
    return (long)p_bytes_written;
#else
    assert(_file != -1);
    if(_file == -1) return 0;
    return write(_file, p_buffer, n_bytes_2_write);  
#endif
}

int CRzFile::Create(const char* filename)
{
    assert(filename != NULL);
    Close();
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    _file = ::CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (_file == INVALID_HANDLE_VALUE) 
    {
        RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", ::GetLastError());
        return -1;
    }
#else
    if((_file = open(filename, O_RDWR | O_CREAT, S_IREAD | S_IWRITE)) < 0)
    {
        RZ_CONSOLE(ERROR) << RZ_FORMAT("Filed open file, errno = %d \n", errno);
        return -1;
    }
#endif
    _readonly = false;
    strcpy(_filename, filename);
    return 0;
}

int CRzFile::Delete()
{
    Close();
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    return ::DeleteFileA(_filename) ? 0 : -1;
#else
    return remove(_filename);
#endif  
}

_RzStdEnd
#endif//__RZFILE_HPP__
