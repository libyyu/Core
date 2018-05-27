#ifndef __RZFUNC_HPP__
#define __RZFUNC_HPP__
#pragma once
#include <time.h>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vector>
#include "RzType.hpp"
#include <sys/types.h>  
#include <sys/stat.h>
#include <functional>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#if PLATFORM_TARGET == PLATFORM_MACOSX
#include <libproc.h>
#endif
#endif
_RzStdBegin

inline tm* RzGetNowTime()
{
    time_t tt;
    time(&tt);
    return localtime(&tt);		
}

template<typename T>
inline void SafeDelete(T &p)
{
	delete p;
	p = NULL;
}

template<typename T>
inline void RzSwap(T& r, T& l)
{
    T tmp = r;
    r = l;
    l = tmp;
}

inline void RzSleep(unsigned int seconds)
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    Sleep(seconds*1000);
#else
    sleep(seconds);
#endif
}

inline std::string RzFormat(const char* format, ...)
{
	va_list va;
	va_start(va, format);
	const int MAX_BUFFLEN = 20480;
	char buff[MAX_BUFFLEN+1] = { 0 };
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	_vsnprintf_s(buff, MAX_BUFFLEN, format, va);
#else
	vsnprintf(buff, MAX_BUFFLEN, format, va);
#endif
	va_end(va);
	return std::string(buff);
}

inline uint32 RzGetCurrentThreadId()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	return (uint32)GetCurrentThreadId();
#else
	return static_cast<uint32>(reinterpret_cast<uintptr_t>(pthread_self()));
#endif
}

inline void  RzUnitPath(char *path) 
{
	for (size_t i = 0; i < strlen(path); ++i)
	{
		if (path[i] == '\\') 
		{
			path[i] = '/';
		}
	}
}

inline char* RzGetPwd(char *buffer)
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    return _getcwd(buffer,buffer ? sizeof(buffer) : 0);
#else
    return getcwd(buffer,buffer ? sizeof(buffer) : 0);
#endif
}

inline void RzMakeDir(const char* path) {
	char* p = (char*)path;
	char* pd = (char*)path;
	char ch;
	char dir[256] = { 0 };
	while ((ch = *p++))
	{
		if (ch == '\\' || ch == '/')
		{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
			_mkdir(dir);
#else
			mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
		}
		memcpy(dir, pd, p - pd);
	}
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	_mkdir(dir);
#else
	mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
		S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

inline void RzSplitpath(const char* s, char* path, char* name, char* ext)
{
	if (path) memset(path, 0, strlen(path));
	if (name) memset(name, 0, strlen(name));
	if (ext) memset(ext, 0, strlen(ext));

	size_t n = strlen(s);
	char* p = (char*)s;
	p += n;
	bool skiped = false;
	while (p != s && *p != '.') {
		if (*p == '/' || *p == '\\'){
			skiped = true;
			break;
		}
		p--;
	}
	char pwholename[256] = { 0 };
	if (*p != '.' || skiped){
		strcpy(pwholename, s);
	}
	else{
		if (ext) strcpy(ext, p);
		memcpy(pwholename, s, p - s);
	}

	p = pwholename;
	p += n;
	while (p != pwholename && *p != '/' && *p != '\\') p--;
	if (*p == '/' || *p == '\\'){
		p++;
		if (name) strcpy(name, p);
		if (path) memcpy(path, pwholename, p - pwholename - 1);
	}
	else{
		if (name) strcpy(name, pwholename);
	}
}

inline const char* RzGetModulePath()
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    static char modulepath[MAX_PATH] = { 0 };
    ::GetModuleFileNameA(NULL, modulepath, MAX_PATH);
    strrchr(modulepath,  '/');
    return modulepath;
#elif PLATFORM_TARGET == PLATFORM_MACOSX
    pid_t pid = getpid();
    static char modulepath[PROC_PIDPATHINFO_MAXSIZE] = {0};
    int ret = proc_pidpath (pid, modulepath, sizeof(modulepath));
    if(ret <= 0) return NULL;
    return modulepath;
#else
    const int len = 1024;
	static char modulepath[len] = {0};
    if(readlink("/proc/self/exe", modulepath, len-1) <=0) return NULL;
    return modulepath;
#endif
}

inline const char* RzGetModuleName()
{
    static char name[100] = {0};
    const char* modulepath = RzGetModulePath();
    if(!modulepath) return NULL;
    const char* path_end = strrchr(modulepath,  '/');
    if(path_end == NULL) return NULL;
    ++path_end;
    strcpy(name, path_end);
    return name;
}

inline bool RzFileExists(const char* filename)
{
    assert(filename);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
    if (filename != NULL && strlen(filename) > 0)
    {
        struct _stat32 stat_info;
        return _stat32(filename, &stat_info) == 0;
    }
    else
        return false;
#else
    if (filename != NULL && strlen(filename) > 0)
        return access(filename, 0) == 0;
    else
        return false;
#endif
}

inline bool RzDirExists(const char* path)
{
    assert(path);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	_finddata_t finddata;
	intptr_t h = _findfirst(path, &finddata);
	if (h == -1) {
		return false;
	}

	_findclose(h);
	return (finddata.attrib & _A_SUBDIR) == _A_SUBDIR;
#else
	struct stat st;
	stat(path, &st);
	if (S_ISDIR(st.st_mode))
		return true;
	else
		return false;
#endif
}

inline int RzGetAllFiles(const char* path, bool reversal = true, const std::function<void(const char*, bool)>& action = nullptr) 
{
	assert(path);
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	char fullpath[256] = { 0 };
	sprintf(fullpath, "%s\\*", path);
	_finddata_t finddata;
	intptr_t h = _findfirst(fullpath, &finddata);
	if (h == -1) 
	{
		return -1;
	}

	int ret = 0;
	do {
		if ((finddata.attrib & _A_SUBDIR) == _A_SUBDIR) 
		{
			if (strcmp(finddata.name, ".") != 0 && strcmp(finddata.name, "..") != 0) 
			{
				char new_path[256] = { 0 };
				sprintf(new_path, "%s\\%s", path, finddata.name);
				if(action) action(new_path, true);
				if(reversal)
				{
					ret = RzGetAllFiles(new_path, reversal, action);
					if (ret != 0) 
					{
						break;
					}
				}
			}
		}
		else 
		{
			char new_path[256] = { 0 };
			sprintf(new_path, "%s\\%s", path, finddata.name);
			if(action) action(new_path, false);
		}

		if (_findnext(h, &finddata) != 0) 
		{
			break;
		}
	} while (true);

	_findclose(h);
	return ret;
#else
	DIR* dir = NULL;
	struct dirent* ptr = NULL;
	if ((dir = opendir(path)) == NULL)
	{
		return -1;
	}
	int ret = 0;
	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) //curent dir or parent dir
			continue;
		else if (ptr->d_type == 8) //file
		{
			char new_path[256] = { 0 };
			sprintf(new_path, "%s/%s", path, ptr->d_name);
			if(action) action(new_path, false);
		}
		else if (ptr->d_type == 10) // link file
			continue;
		else if (ptr->d_type == 4) //dir
		{
			char new_path[256] = { 0 };
			sprintf(new_path, "%s/%s", path, ptr->d_name);
			if(action) action(new_path, true);
			if(reversal)
			{
				ret = RzGetAllFiles(new_path, reversal, action);
				if (ret != 0)
				{
					break;
				}
			}
		}
	}
	closedir(dir);
	return ret;
#endif
}
_RzStdEnd

_RzStdBegin
inline void* memcpyFromRing(void * dst,
    const size_t cnt,
    const void * src,
    size_t b,
    size_t size) 
{
    assert(cnt <= size);
    assert(dst);
    assert(src);
    assert(cnt > 0);
    assert(size > 0);
    assert(b < size);

    char * pdst = (char *)dst;
    char * psrc = (char *)src;

    if((b + cnt) <= size) {
        memcpy(pdst, &psrc[b], cnt);
        return dst;
    }

    size_t l = size - b;
    memcpy(pdst, &psrc[b], l);
    memcpy((pdst + l), src, cnt -l);
    return dst;
}
inline void* memcpyToRing(const void *src,
    const size_t cnt,
    void * dst,
    size_t b,
    size_t size) 
{
    assert(cnt <= size);
    assert(dst);
    assert(src);
    assert(cnt > 0);
    assert(size > 0);
    assert(b < size);

    char * pdst = (char *)dst;
    char * psrc = (char *)src;

    if ((b + cnt) <= size) {
        memcpy(&pdst[b], psrc, cnt);
        return dst;
    }

    size_t l = size - b;
    memcpy(&pdst[b], psrc, l);
    memcpy(pdst,(psrc + l), cnt - l);
    return dst;
}

inline int memfind(const void* src,size_t srcsize,const void* dst,size_t dstsize)
{
    assert(src);
    assert(dst);
    assert(srcsize);
    assert(dstsize);
    if(!src || !dst) return -1;
    if(!srcsize || !dstsize) return -1;

    char* psrc = (char*)src;
    char* pdst = (char*)dst;
    size_t k = dstsize;
    size_t n = srcsize;
    while(n-- >= k)
    {
        if(*psrc++ != *pdst++)
        {
            pdst = (char*)dst;
            k    = dstsize;
        }else
        {
            --k;
            if(0 == k)
                return (srcsize - n - dstsize);
        }
    }
    return -1;
}

_RzStdEnd

_RzStdBegin
#define RZ_FORMAT RzFormat
_RzStdEnd

#endif//__RZFUNC_HPP__
