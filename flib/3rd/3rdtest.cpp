#include <iostream>
#include <functional>
#include "lua/LuaEnv.hpp"

int main()
{
    LuaEnv* env = new LuaEnv;
    if(!lua::runString(*env, "print(a+1,'hello world')"))
    {
        std::cerr << lua_tostring(*env, -1) << std::endl;
    }
    const char* m = "local _G = _G\
    function glbFunc(...) \
        warn(...) \
        print(a+1) \
    end \
    ";
    env->doString(m);
    env->doString("print('hello world')");
    env->doGlobal("glbFunc", "ceshi");
    if(!lua::callFunction(*env, "glbFunc", "ceshi2"))
    {
        std::cerr << lua_tostring(*env, -1) << std::endl;
    }
    lua::lua_func_ref_t t(*env, "glbFunc");
    t.call("ceshi3");
    t.unref();

    delete env;
    return 0;
}