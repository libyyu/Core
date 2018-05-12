#ifndef _LUA_ENV_H
#define _LUA_ENV_H

#include "rzlib/3rd/lua/lua_script.hpp"

class LuaEnv
{
public:
	LuaEnv(bool open_ = true);
	~LuaEnv();
	bool open();
	void close();
	operator lua_State* () { return m_L; }
	operator bool() { return m_L != nullptr; }
public:
	std::string getTraceback(const char* err = "Lua traceback:");

	bool doString(const char* buff);
	bool doFile(const char* file);
	bool doGlobal(const char* method);
	bool doTableFunc(int refTable, const char* method);

	template <typename ... Args>
	bool doString(const char* buff, const Args &... args);
	template <typename ... Args>
	bool doFile(const char* file, const Args &... args);
	template <typename ... Args>
	bool doGlobal(const char* method, const Args &... args);
	template <typename ... Args>
	bool doTableFunc(int refTable, const char* method, const Args &... args);
protected:
	static int panic(lua_State* l);
	static int print(lua_State* l);
	static int warn(lua_State* l);
	static int error_traceback(lua_State* l);
	static void on_error_handler(const char* msg);
private:
	lua_State* m_L;
	bool       m_open;
};

LuaEnv* glb_LuaEnv();


#endif//_LUA_ENV_H