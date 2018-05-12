#include "stdafx.h"
#include "LuaEnv.h"
#include <string>
#include <assert.h>
#if PLATFORM_TARGET == PLATFORM_WINDOWS
#include <direct.h>
#include <Windows.h>
#ifdef _DEBUG
#pragma comment(lib, "lpeg_d")
#else
#pragma comment(lib, "lpeg")
#endif
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include "rzlib/base/RzConsole.hpp"
using namespace RzStd;

extern "C" 
{
	extern int luaopen_lpeg(lua_State *L);
}

static LuaEnv* glb_luaEnv = nullptr;
LuaEnv* glb_LuaEnv()
{
	return glb_luaEnv;
}

LuaEnv::LuaEnv(bool open_) :m_L(nullptr)
{
	glb_luaEnv = this;
	if (open_) open();
}
LuaEnv::~LuaEnv()
{
	close();
	if (glb_luaEnv == this)
	{
		glb_luaEnv = nullptr;
	}
}
bool LuaEnv::open()
{
	close();
	m_L = luaL_newstate();
	luaL_openlibs(m_L);
	lua_atpanic(m_L, panic);

	luaopen_lpeg(m_L);

	lua_pushcfunction(m_L, print);
	lua_setfield(m_L, LUA_GLOBALSINDEX, "print");

	lua_pushcfunction(m_L, warn);
	lua_setfield(m_L, LUA_GLOBALSINDEX, "warn");

	m_open = true;
	return true;
}
void LuaEnv::close()
{
	if (m_L)
	{
		lua_close(m_L);
		m_L = nullptr;
	}
	m_open = false;
}
int LuaEnv::panic(lua_State* l)
{
	std::string reason = "";
	reason += "unprotected error in call to Lua API (";
	const char* s = lua_tostring(l, -1);
	reason += s;
	reason += ")\n";
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	OutputDebugStringA(reason.c_str());
#endif
	printf("Exception: %s\n", s);
	throw reason;
	return 0;
}
int LuaEnv::print(lua_State* l)
{
	std::string s;
	int n = lua_gettop(l);
	lua_getglobal(l, "tostring");

	for (int i = 1; i <= n; ++i)
	{
		lua_pushvalue(l, -1); // function to be called
		lua_pushvalue(l, i);  // value to print
		lua_call(l, 1, 1);

		const char* ret = lua_tostring(l, -1);
		if (!ret)
		{
			//log error
		}
		else
		{
			s.append(ret);
		}
		if (i < n)
		{
			s.append("\t");
		}
		else
		{
			s.append("\n");
		}
		lua_pop(l, 1); //pop result
	}

	//printf("[LUA]%s", s.c_str());
	RZ_CONSOLE(INFO) << "[LUA]" << s.c_str();
#if defined( _DEBUG ) && defined(_WIN32)
	OutputDebugStringA("[LUA]");
	OutputDebugStringA(s.c_str());
#endif
	return 0;
}
int LuaEnv::warn(lua_State* l)
{
	std::string s;
	int n = lua_gettop(l);
	lua_getglobal(l, "tostring");

	for (int i = 1; i <= n; ++i)
	{
		lua_pushvalue(l, -1); // function to be called
		lua_pushvalue(l, i);  // value to print
		lua_call(l, 1, 1);

		const char* ret = lua_tostring(l, -1);
		if (!ret)
		{
			//log error
		}
		else
		{
			s.append(ret);
		}
		if (i < n)
		{
			s.append("\t");
		}
		else
		{
			s.append("\n");
		}
		lua_pop(l, 1); //pop result
	}

	//printf("[LUA]%s", s.c_str());
	RZ_CONSOLE(WARN) << "[LUA]" << s.c_str();
#if defined( _DEBUG ) && defined(_WIN32)
	OutputDebugStringA("[LUA]");
	OutputDebugStringA(s.c_str());
#endif
	return 0;
}
int LuaEnv::error_traceback(lua_State* l)
{
	int oldTop = lua_gettop(l);
	lua_checkstack(l, 3);
	lua_getglobal(l, "debug");
	lua_getfield(l, -1, "traceback");
	lua_pushstring(l, "Lua traceback:");
	lua_pushnumber(l, 1);
	lua_call(l, 2, 1);
	std::string traceback = lua_tostring(l, -1);
	lua_settop(l, oldTop);

	std::string err = lua_tostring(l, -1);
	lua_pop(l, 1);
	std::stringstream msg;

	msg << " [" << err << "]"
		<< std::endl
		<< traceback.c_str();

#if defined( AZURE_CLIENT )
	a_UnityFormatLogError("Error: %s\n", msg.str().c_str());
#else
	//fprintf(stderr, "Error: %s\n", msg.str().c_str());
	RZ_CONSOLE(ERROR) << "Error" << msg.str().c_str() << endl;
#endif

	return 0;
}

std::string LuaEnv::getTraceback(const char* err /* = "Lua traceback:" */)
{
	lua_State* l = m_L;
	int oldTop = lua_gettop(l);
	lua_checkstack(l, 3);
	lua_getglobal(l, "debug");
	lua_getfield(l, -1, "traceback");
	lua_pushstring(l, err);
	lua_pushnumber(l, 1);
	lua_call(l, 2, 1);
	std::string trace = lua_tostring(l, -1);
	lua_settop(l, oldTop);
	return trace;
}

void LuaEnv::on_error_handler(const char* msg)
{
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	OutputDebugStringA(msg);
	//MessageBoxA(NULL,msg, "Lua Error", 0);
#endif
	//fprintf(stderr, "Error: %s\n", msg);
	RZ_CONSOLE(ERROR) << "Error" << msg << endl;
}

bool LuaEnv::doString(const char* buff)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	int oldtop = lua_gettop(l);
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

	if (0 != luaL_loadstring(l, buff))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);
		lua_remove(l, pos_err);
		return false;
	}

	int n = 0;
	int error = lua_pcall(l, n, -1, pos_err);
	if (error != 0)
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

bool LuaEnv::doFile(const char* filename)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

#if defined (AZURE_CLIENT)
	unsigned char* pBuffer = NULL;
	unsigned int   nLength = 0;
	bool ret = af_ReadFileAllBytes(filename, &pBuffer, &nLength);
	if (!ret || !nLength || !pBuffer)
	{
		a_UnityFormatLogError("Error: Can't ReadFileAllBytes %s\n", filename);
		lua_remove(l, pos_err);
		return false;
	}
	std::string chunk = std::string("@") + filename;
	if (0 != luaL_loadbuffer(l, (char*)pBuffer, nLength, chunk.c_str()))
	{
		af_ReleaseFileBuffer(pBuffer);
		a_UnityFormatLogError("Error: luaL_loadbuffer %s\n", filename);
		lua_remove(l, pos_err);
		return false;
	}
	af_ReleaseFileBuffer(pBuffer);
#else
	if (0 != luaL_loadfile(l, filename))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);
		lua_remove(l, pos_err);
		return false;
	}
#endif

	int n = 0;
	int error = lua_pcall(l, n, -1, pos_err);
	if (error != 0)
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

bool LuaEnv::doGlobal(const char* method)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

	lua_getglobal(l, method);
	if (!lua_isfunction(l, -1))
	{
		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	int n = 0;
	if (0 != lua_pcall(l, n, -1, pos_err))
	{
		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

bool LuaEnv::doTableFunc(int refTable, const char* method)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

	lua_rawgeti(l, (int)LUA_REGISTRYINDEX, refTable); //t
	lua_getfield(l, -1, method);//t,func
	if (lua_isnil(l, -1))
	{
		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	int n = 0;
	if (0 != lua_pcall(l, n, -1, pos_err))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

template <typename ... Args>
bool LuaEnv::doString(const char* buff, const Args &... args)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);
	if (0 != luaL_loadstring(l, buff))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_remove(l, pos_err);
		return false;
	}

	int n = lua::push(l, args...);
	int error = lua_pcall(l, n, -1, pos_err);
	if (error != 0)
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

template <typename ... Args>
bool LuaEnv::doFile(const char* filename, const Args &... args)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

#if defined (AZURE_CLIENT)
	unsigned char* pBuffer = NULL;
	unsigned int   nLength = 0;
	bool ret = af_ReadFileAllBytes(filename, &pBuffer, &nLength);
	if (!ret || !nLength || !pBuffer)
	{
		a_UnityFormatLogError("Error: Can't ReadFileAllBytes %s\n", filename);
		lua_remove(l, pos_err);
		return false;
	}
	std::string chunk = std::string("@") + filename;
	if (0 != luaL_loadbuffer(l, (char*)pBuffer, nLength, chunk.c_str()))
	{
		af_ReleaseFileBuffer(pBuffer);
		a_UnityFormatLogError("Error: luaL_loadbuffer %s\n", filename);
		lua_remove(l, pos_err);
		return false;
	}
	af_ReleaseFileBuffer(pBuffer);
#else
	if (0 != luaL_loadfile(l, filename))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_remove(l, pos_err);
		return false;
	}
#endif

	int n = lua::push(l, args...);
	int error = lua_pcall(l, n, -1, pos_err);
	if (error != 0)
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

template <typename ... Args>
bool LuaEnv::doGlobal(const char* method, const Args &... args)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

	lua_getglobal(l, method);
	if (!lua_isfunction(l, -1))
	{
		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	int n = lua::push(l, args...);
	if (0 != lua_pcall(l, n, -1, pos_err))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}

template <typename ... Args>
bool LuaEnv::doTableFunc(int refTable, const char* method, const Args &... args)
{
	lua_State* l = m_L;
	assert(m_L != nullptr && "lua vm is closed.");
	lua_pushcfunction(l, error_traceback);
	lua_pushvalue(l, -1);
	luaL_ref(l, LUA_REGISTRYINDEX);
	int pos_err = lua_gettop(l);

	lua_rawgeti(l, (int)LUA_REGISTRYINDEX, refTable); //t
	lua_getfield(l, -1, method);//t,func
	if (lua_isnil(l, -1))
	{
		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	int n = lua::push(l, args...);
	if (0 != lua_pcall(l, n, -1, pos_err))
	{
		const char* errmsg = lua_tostring(l, -1);
		on_error_handler(errmsg);

		lua_pop(l, 1);
		lua_remove(l, pos_err);
		return false;
	}
	lua_remove(l, pos_err);
	return true;
}