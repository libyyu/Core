#include <iostream>
#include "3rd/lua/LuaEnv.hpp"
#include "3rd/lua/lua_wrapper.hpp"
#include "flib.h"
LuaEnv* gL = nullptr;
#if 1//def OBJECT_HEAP_RECORD
typedef struct _HeapChunk {
  void* ptr;
  _HeapChunk(){memset(this, 0x00, sizeof(_HeapChunk));}
} HeapChunk;
#define MEM_MAX_RECORDS 4 * 1024
static HeapChunk s_chunks[MEM_MAX_RECORDS];
void AppendHeapChunk(void* ptr)
{
	for (int i = 0; i < MEM_MAX_RECORDS; i++) 
	{
    	HeapChunk* r = (HeapChunk*)s_chunks + i;
    	if (r->ptr == NULL) 
		{
			r->ptr = ptr;
			break;
		}
	}
}
void RemoveHeapChunk(void* ptr)
{
	for (int i = 0; i < MEM_MAX_RECORDS; i++) 
	{
    	HeapChunk* r = (HeapChunk*)s_chunks + i;
		if (r->ptr == ptr) 
		{
			memset(r, 0x00, sizeof(HeapChunk));
			if(gL) lua::get_luaobj_container().RemoveObject(*gL, ptr);
			break;
		}
  }
}
namespace lua
{
	int get_object_flag(void* ptr)
	{
		for (int i = 0; i < MEM_MAX_RECORDS; i++) 
		{
			HeapChunk* r = (HeapChunk*)s_chunks + i;
			if (r->ptr == ptr) 
			{
				return 1;
			}
		}
		return 0;
	}
}
#endif//OBJECT_HEAP_RECORD

class base_t
{
public:
	virtual const char* getname(){
		return "base_t";
	}
	virtual ~base_t(){
		printf("base_t::~base_t... %p\n", this);
	}
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

	void base_print()
	{
		printf("base_t::base_print...%s\n", getname());
	}
};

class foo_t : public base_t
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
	virtual ~foo_t(){
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

	virtual const char* getname(){
		return "foo_t";
	}
};

class boo_t : public foo_t
{
public:
	virtual const char* getname(){
		return "boo_t";
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

int deletebase(lua_State* l)
{
	base_t * b = nullptr;
	lua::get(l, 1, &b);
	printf("deletebase %p\n", b ? b : 0);
	if(b) delete b;
	return 0;
}

int test_cplus_lua()
{
	char this_path[256] = { 0 };
	FStd::FSplitpath(__FILE__, this_path, NULL, NULL);

	LuaEnv* env = new LuaEnv;
	lua_State * l = *env;
	gL = env;
	/////reg c++ obj
	lua::lua_register_t<base_t>(l, "base_t")
		.def(lua::constructor<>())
		.def(lua::destructor())
		.def("print", &base_t::print)
		.def("print2", &base_t::print2)
		.def("base_print", &base_t::base_print)
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

	lua::lua_register_t<boo_t>(l, "boo_t")
		.extend<foo_t>()
		.def(lua::constructor<>())
		.def(lua::destructor());

	lua::lua_register_t<void>(l)
		.def("foo", createfoo)
		.def("delbase", deletebase)
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
    //test_base();


	test_cplus_lua();

    return 0;
}
