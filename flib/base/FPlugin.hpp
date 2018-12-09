#ifndef __FPLUGIN_HPP__
#define __FPLUGIN_HPP__
#pragma once
#include "FType.hpp"
#include <locale.h>
#include <iostream>
#include <sstream> 
#include <stdarg.h>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

_FStdBegin

class FPlugin
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	typedef HINSTANCE   plugin_t;
#else
	typedef void*		plugin_t;
#endif
public:
	FPlugin(bool auto_unload = true) :_plugin(NULL), _auto_unload(auto_unload) {}
	FPlugin(const char* plugin_name, bool auto_unload = true) :_plugin(NULL), _auto_unload(auto_unload)
	{
		LoadPlugin(plugin_name);
	}
	~FPlugin()
	{
		if(_auto_unload)
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

	template <typename FUNC>
	inline FUNC Get(const char* symbol_name)
	{
		void* addr = GetSymbol(symbol_name);
		if (!addr)
		{
			return NULL;
		}
		return (FUNC)(addr);
	}

	template <typename R, typename ... Args>
	inline R Call(const char* symbol_name, const Args &... args)
	{
		typedef R(*function_t)(Args...);
		function_t function_ = Get<function_t>(symbol_name);
		if(!function_) return R();
		return function_(args...);
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
	bool _auto_unload;
};


_FStdEnd


#endif//__FPLUGIN_HPP__