#include <iostream>
#include "3rd/lua/LuaEnv.hpp"
#include "3rd/lua/lua_wrapper.hpp"
#include "base/FFunc.hpp"
class base_t
{
public:
	void print(){
		printf("base_t::print...\n");
	}

	static void print2(){
		printf("static base_t::print2...\n");
	}

	const char* tostring(){
		printf("call by __tostring\n");
		return "base_t::tostring";
	}
};

class foo_t
{
	std::string _name;
public:
	const int flag = 0;
	int num;
	static int counter;
	foo_t(){
		_name = "class foot";
		num = 0;
		counter++;
	}
	~foo_t(){
		printf("~foo_t\n");
	}
	void setname(const char* name){
		_name = name;
	}
	void fprint(){
		printf("foo_t::print..... name = %s, num = %d, counter = %d\n",_name.c_str(), num, counter);
	}
	static void fprint2(){
		printf("static foo_t::print2.....,counter = %d\n", counter);
	}
};
int foo_t::counter = 0;
int writeonly = 8;

int createfoo(lua_State* l)
{
	foo_t * pf = new foo_t();
	lua::push(l, pf);
	return 1;
}

int printwriteonly(lua_State*)
{
	printf("writeonly = %d\n", writeonly);
	return 0;
}

int test_cplus_lua()
{
	char this_path[256] = { 0 };
	FStd::FSplitpath(__FILE__, this_path, NULL, NULL);

	LuaEnv* env = new LuaEnv;
	lua_State * l = *env;
	/////reg c++ obj
	lua::lua_register_t<base_t>(l, "base_t")
		.def(lua::constructor<>())
		.def(lua::destructor())
		.def("print", &base_t::print)
		.def("print2", &base_t::print2)
		.def("__tostring", &base_t::tostring);

	lua::lua_register_t<foo_t>(l, "foo_t")
		.extend<base_t>()
		.def(lua::constructor<>())
		.def(lua::destructor())
		.def("fprint", &foo_t::fprint)
		.def("fprint2", &foo_t::fprint2)
		.def("setname", &foo_t::setname)
		.def("num", &foo_t::num)
		.readonly("flag", &foo_t::flag)
		.writeonly("writeonly", &writeonly)
		.def("counter", &foo_t::counter);

	lua::lua_register_t<void>(l)
		.def("foo", createfoo)
		.def("printwriteonly", printwriteonly);

	lua::lua_table_ref_t player;
	if(env->doFile("./class.lua"))
	{
		lua::pop(l, &player);
		player.call("speak", player, "hello lua.");
		player.call("speak", player, "hello C++.");
		player.unref();
	}
	else
	{
		const char* err = lua_tostring(l, -1);
		printf("can not run class.lua, err = %s!\n", err);
	}

	delete env;

	printf("Success to exit.\n");

#ifdef _WIN32
	system("pause");
#endif
    return 0;
}

void test_base()
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
}

int main()
{
    test_base();


	test_cplus_lua();

    return 0;
}
