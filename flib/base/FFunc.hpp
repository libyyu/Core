#ifndef __FFUNC_HPP__
#define __FFUNC_HPP__
#pragma once
#include "FType.hpp"
#include <time.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <functional>
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <algorithm>
#else
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#if FLIB_COMPILER_MACOSX
#include <libproc.h>
#endif
#endif
_FStdBegin

inline tm* FGetNowTime()
{
    time_t tt;
    time(&tt);
    return localtime(&tt);		
}

template<typename T>
inline void FSafeDelete(T* &p)
{
	delete p;
	p = NULL;
}

template<typename T>
inline void FSwap(T& r, T& l)
{
    T tmp = r;
    r = l;
    l = tmp;
}

inline void FSleep(unsigned int seconds)
{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    Sleep(seconds*1000);
#else
    sleep(seconds);
#endif
}

inline std::string FFormat(const char* format, ...)
{
	va_list va;
	va_start(va, format);
	const int MAX_BUFFLEN = 20480;
	char buff[MAX_BUFFLEN+1] = { 0 };
#if FLIB_COMPILER_CYGWIN
	vsnprintf(buff, MAX_BUFFLEN, format, va);
#elif FLIB_COMPILER_MSVC
	_vsnprintf_s(buff, MAX_BUFFLEN, format, va);
#else
	vsnprintf(buff, MAX_BUFFLEN, format, va);
#endif
	va_end(va);
	return std::string(buff);
}

inline uint32 FGetCurrentThreadId()
{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
	return (uint32)GetCurrentThreadId();
#elif FLIB_COMPILER_ANDROID
	return static_cast<uint32>(reinterpret_cast<long>(pthread_self()));
#else
	return static_cast<uint32>(reinterpret_cast<uintptr_t>(pthread_self()));
#endif
}

inline char* FGetPwd(char *buffer)
{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    return _getcwd(buffer,buffer ? sizeof(buffer) : 0);
#else
    return getcwd(buffer,buffer ? sizeof(buffer) : 0);
#endif
}

inline void FMakeDir(const char* path) {
	char* p = (char*)path;
	char* pd = (char*)path;
	char ch;
	char dir[1024] = { 0 };
	while ((ch = *p++))
	{
		if (ch == '\\' || ch == '/')
		{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
			_mkdir(dir);
#else
			mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
		}
		memcpy(dir, pd, p - pd);
	}
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
	_mkdir(dir);
#else
	mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
		S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

inline void FSplitpath(const char* s, char* path, char* name, char* ext)
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

inline const char* FGetModulePath()
{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    static char modulepath[MAX_PATH] = { 0 };
    ::GetModuleFileNameA(NULL, modulepath, MAX_PATH);
    return modulepath;
#elif FLIB_COMPILER_MACOSX
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

inline const char* FGetModuleName()
{
    static char name[100] = {0};
    const char* modulepath = FGetModulePath();
    if(!modulepath) return NULL;
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    const char* path_end = strrchr(modulepath,  '\\');
#else
	const char* path_end = strrchr(modulepath, '/');
#endif
    if(path_end == NULL) return NULL;
    ++path_end;
    strcpy(name, path_end);
    return name;
}

inline bool FFileExists(const char* filename)
{
    assert(filename);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
    if (filename != NULL && strlen(filename) > 0)
    {
#if FLIB_COMPILER_CYGWIN
        struct _stat stat_info;
        return _stat(filename, &stat_info) == 0;
#else
        struct _stat32 stat_info;
        return _stat32(filename, &stat_info) == 0;
#endif
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

inline bool FDirExists(const char* path)
{
    assert(path);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
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

inline int FGetAllFiles(const char* path, bool reversal = true, const std::function<void(const char*, bool)>& action = nullptr) 
{
	assert(path);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
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
					ret = FGetAllFiles(new_path, reversal, action);
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
				ret = FGetAllFiles(new_path, reversal, action);
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

inline void  FUnitPath(char *path) 
{
	assert(path);
	for (size_t i = 0; i < strlen(path); ++i)
	{
		if (path[i] == '\\') 
			path[i] = '/';
	}
}

inline void  FUnitPath(std::string& path) 
{
	std::replace(path.begin(), path.end(), '\\', '/');
}

inline const std::string FJoinPath(const std::string& path1, const std::string& path2) 
{
	std::string lp = path1;
	std::string rp = path2;
	if (lp.length() >0 && lp.back() == SEP)
		lp.erase(lp.size() - 1);
	if (rp.length() >0 && rp.front() == SEP)
		rp.erase(rp.begin());
	
	return lp + SEP + rp;
}

inline const std::string FNormalize(const char* path) 
{
	assert(path);
	std::string normalized(path);
	for (size_t i = 0; i < normalized.size(); ++i) 
	{
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
		if (normalized[i] == '/') 
		{
			normalized[i] = '\\';
		}
#else
		if (normalized[i] == '\\') 
		{
			normalized[i] = '/';
		}
#endif
	}
	return normalized;
}

inline const std::string FGetDirectory(const char* path) 
{
	std::string normalized = FNormalize(path);
	auto pos = normalized.rfind(SEP);
	if (pos == std::string::npos) 
	{
		return ".";
	}

	return normalized.substr(0, pos);
}

inline const std::string FGetFilename(const char* path) 
{
	std::string normalized = FNormalize(path);
	auto pos = normalized.rfind(SEP);
	if (pos == std::string::npos) 
	{
		return path;
	}

	return normalized.substr(pos + 1);
}

inline const std::string FGetExtension(const char* path) 
{
	std::string normalized = FNormalize(path);
	auto pos1 = normalized.rfind(SEP);
	auto pos2 = normalized.rfind('.');
	if (pos2 == std::string::npos) 
	{
		return "";
	}

	if (pos1 == std::string::npos || pos2 > pos1) 
	{
		return normalized.substr(pos2 + 1);
	}

	return "";
}

inline void FStringReplace(std::string &strBase, const std::string& strSrc, const std::string& strDes) 
{
	std::string::size_type srcLen = strSrc.size();
	std::string::size_type desLen = strDes.size();
	std::string::size_type pos = 0;

	pos = strBase.find(strSrc, pos);
	while (pos != std::string::npos)
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, pos + desLen);
	}
}

inline void FStringSplit(StringVec& outArr, const std::string &str, const std::string &pattern)
{
	if (str.empty() || pattern.empty())
        return;

    std::string strs = str + pattern;

	size_t pos = strs.find(pattern);
	size_t size = strs.size();

	while (pos != std::string::npos)
	{
		std::string x = strs.substr(0, pos);
		outArr.push_back(x);
		strs = strs.substr(pos + 1, size);
		pos = strs.find(pattern);
	}
}

inline bool FReadAllBytes(const char* filename, ByteArray& buffer) 
{
#if FLIB_COMPILER_MSVC
	FILE* fp = NULL;
	fopen_s(&fp, filename, "rb");
#else
	FILE* fp = fopen(filename, "rb");
#endif
	if (!fp) 
		return false;

	fseek(fp, 0, SEEK_END);
	ulong size = (ulong)ftell(fp);
	if (size == 0) 
	{
		fclose(fp);
		return true;
	}

	buffer.reserve(size);
	buffer.clear();
	fseek(fp, 0, SEEK_SET);
	fread(&buffer[0], sizeof(uchar), size, fp);
	fclose(fp);
	return true;
}

inline bool FWriteAllBytes(const char* filename, uchar* bytes, size_t size) 
{
#if FLIB_COMPILER_MSVC
	FILE* fp = NULL;
	fopen_s(&fp, filename, "wb");
#else
	FILE* fp = fopen(filename, "wb");
#endif
	if (!fp) 
	{
		return false;
	}

	if (fwrite(bytes, size, 1, fp) != 1) 
	{
		fclose(fp);
		return true;
	}

	fclose(fp);
	return false;
}

inline bool FReadAllText(const char* filename, std::string& text)
{
	ByteArray buffer;
	if(FReadAllBytes(filename, buffer))
	{
		const uchar* ptr = &buffer[0];
		text.append((const char*)ptr);
		return true;
	}
	return false;
}

inline bool FReadTextToArray(const char* filename, std::vector<std::string>& resVec)
{
	std::string text;
	if(FReadAllText(filename, text))
	{
		FStringSplit(resVec, text, "\n");
		return true;
	}
	return false;
}

_FStdEnd

_FStdBegin
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
                return (int)(srcsize - n - dstsize);
        }
    }
    return -1;
}

_FStdEnd

#define F_FORMAT FStd::FFormat

#endif//__FFUNC_HPP__
