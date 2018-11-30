#ifndef __RZPLUGIN_HPP__
#define __RZPLUGIN_HPP__
#pragma once
#include <string>
#include <locale.h>
#include <assert.h>
#include <iostream>
#include <sstream> 
#include <stdarg.h>
#include "RzType.hpp"
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

_RzStdBegin

class CRzPlugin
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	typedef HINSTANCE  plugin_t;
#else
	typedef void*		plugin_t;
#endif
public:
	CRzPlugin() :_plugin(NULL) {}
	CRzPlugin(const char* plugin_name) :_plugin(NULL)
	{
		LoadPlugin(plugin_name);
	}
	~CRzPlugin()
	{
		Unload();
	}
	inline void Unload()
	{
		if (!IsLoaded())
			return;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		::FreeLibrary(_plugin);
#else
		dlclose(_plugin);
#endif
		_plugin = NULL;
	}
	inline bool LoadPlugin(const char* plugin)
	{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		_plugin = ::LoadLibraryA(plugin);
#else
		_plugin = dlopen(plugin, RTLD_NOW);
#endif		
		return IsLoaded();
	}

	inline void* GetSymbol(const char* symbol)
	{
		if (!IsLoaded())
		{
			return NULL;
		}
#if PLATFORM_TARGET == PLATFORM_WINDOWS
		void* const addr = ::GetProcAddress(_plugin, symbol);
#else
		void* const addr = dlsym(_plugin, symbol);
#endif
		return addr;
	}

	inline bool Has(const char* symbol_name)
	{
		return IsLoaded() && !!GetSymbol(symbol_name);
	}

	template <typename T>
	inline T Get(const char* symbol_name)
	{
		void* addr = GetSymbol(symbol_name);
		if (!addr)
		{
			return NULL;
		}
		return (T)(addr);
	}

	inline bool IsLoaded()
	{
		return _plugin != NULL;
	}

	operator plugin_t() const
	{
		return _plugin;
	}
protected:
private:
	plugin_t _plugin;
};


_RzStdEnd


#endif//__RZPLUGIN_HPP__