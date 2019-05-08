#ifndef LUA_WRAPPER_HPP
#define LUA_WRAPPER_HPP
#pragma once
#include "lua_script.hpp"
namespace lua
{
	////////////////////////////////////////////////////////////
	////
	template < typename R, typename T, typename ...Args >
	struct object_wrapper_t
	{
		using return_t = R;
		using handler_t = R(T::*)(Args...);

		typedef std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> params_t;
		typedef std::tuple<Args...> tuple_t;

		T *obj_;
		handler_t handler_;

		object_wrapper_t(T *obj, handler_t handler)
			: obj_(obj)
			, handler_(handler)
		{}

		R operator()(const Args &...param) const
		{
			return (*obj_.*handler_)(param...);
		}
	};

	template < typename R, typename T, typename ...Args >
	struct lambda_wrapper_t
	{
		using return_t = R;
		using handler_t = R(T::*)(Args...);

		typedef std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> params_t;
		typedef std::tuple<Args...> tuple_t;

		T *obj_;

		lambda_wrapper_t(T *obj)
			: obj_(obj)
		{}

		return_t operator()(const Args &...args) const
		{
			return (*obj_)(args...);
		}
	};

	template < typename R, typename ...Args >
	struct function_wrapper_t
	{
		using return_t = R;
		using handler_t = R(*)(Args...);

		typedef std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> params_t;
		typedef std::tuple<Args...> tuple_t;

		handler_t handler_;

		function_wrapper_t(handler_t handler)
			: handler_(handler)
		{}

		R operator()(const Args &...param) const
		{
			return (*handler_)(param...);
		}
	};

	template < typename T, typename ...Args >
	struct constructor_wrapper_t
	{
		typedef void return_t;
		T **pobj_;

		typedef std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> params_t;
		typedef std::tuple<Args...> tuple_t;

		constructor_wrapper_t(T **pobj)
			:pobj_(pobj)
		{}

		return_t operator()(const Args &...param) const
		{
			*pobj_ = new T(param...);
			//*pbbj_ = malloc(sizeof(T));
			//::new (*pbbj_) T(param...);
		}
	};
	////////////////////////////////////////////////////////////
	////
	template < typename R, typename T, typename ...Args >
	object_wrapper_t<R, T, Args...> make_obj(T *obj, R(T::*handler)(Args...))
	{
		return object_wrapper_t<R, T, Args...>(obj, handler);
	}

	template < typename R, typename T, typename ...Args >
	lambda_wrapper_t<R, T, Args...> make_obj(T *obj)
	{
		return lambda_wrapper_t<R, T, Args...>(obj);
	}

	template < typename R, typename ...Args >
	function_wrapper_t<R, Args...> make_obj(R(*handler)(Args...))
	{
		return function_wrapper_t<R, Args...>(handler);
	}


	template < typename T, typename ...Args >
	constructor_wrapper_t<T, Args...> make_obj(T** ppobj, const std::tuple<Args...> &)
	{
		return constructor_wrapper_t<T, Args...>(ppobj);
	}
	////////////////////////////////////////////////////////////
	////
	template < int N >
	struct args_parser_t
	{
		template < typename ...Args >
		static void parse(lua_State* l, int index, std::tuple<Args...> &args)
		{
			typedef std::tuple<Args...> tuple_t;
			typedef typename std::tuple_element<N, tuple_t>::type type;
			type v;
			lua::get(l, index, &v);
			std::get<N>(args) = std::move(v);
			args_parser_t<N - 1>::parse(l, index - 1, args);
		}
	};

	template < >
	struct args_parser_t<0>
	{
		template < typename ...Args >
		static void parse(lua_State* l, int index, std::tuple<Args...> &args)
		{
			typedef std::tuple<Args...> tuple_t;
			typedef typename std::tuple_element<0, tuple_t>::type type;
			type v;
			lua::get(l, index, &v);
			std::get<0>(args) = std::move(v);
		}

		static void parse(lua_State*, int, std::tuple<> &)
		{
		}
	};
	////////////////////////////////////////////////////////////
	////
	template<int ...> struct seq {};
	template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> {};
	template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };
	template<int N> using seq_t = typename gens<N>::type;

	struct caller_t
	{
		template<typename T, typename ...Args, int ...S>
		static typename T::return_t call(const T &val, std::tuple<Args...> &args, seq<S...>)
		{
			return val(std::get<S>(args)...);
		}
	};
	////////////////////////////////////////////////////////////
	////
	template < typename TupleT, int N >
	struct tuple_helper_t
	{
		typedef TupleT tuple_t;
		typedef typename std::tuple_element<N, tuple_t>::type value_t;

		static void from(lua_State* l, int index, tuple_t &tuple_val)
		{
			value_t val;
			lua::get(l, index, &val);
			std::get<N>(tuple_val) = std::move(val);

			tuple_helper_t<tuple_t, N - 1>::from(l, index - 1, tuple_val);
		}

		static void push(lua_State* l, const tuple_t &val)
		{
			lua::push(l, std::get<N>(val));

			tuple_helper_t<tuple_t, N - 1>::to(l, val);
		}
	};
	template < typename TupleT >
	struct tuple_helper_t<TupleT, 0>
	{
		typedef TupleT tuple_t;
		typedef typename std::tuple_element<0, tuple_t>::type value_t;


		static void from(lua_State* l, int index, tuple_t &tuple_val)
		{
			value_t val;
			lua::get(l, index, &val);
			std::get<0>(tuple_val) = std::move(val);
		}

		static void push(lua_State* l, const tuple_t &val)
		{
			lua::push(l, std::get<0>(val));
		}
	};
	template < typename ...Args >
	struct lua_op_t< std::tuple<Args...> >
	{
		typedef std::tuple<Args...> tuple_t;

		static tuple_t from_stack(lua_State* l, int index)
		{
			tuple_t val;
			tuple_helper_t<tuple_t, std::tuple_size<tuple_t>::value - 1>::from(l, -1, val);

			return val;
		}

		static int push_stack(lua_State* l, const tuple_t &val)
		{
			tuple_helper_t<tuple_t, std::tuple_size<tuple_t>::value - 1>::push(l, val);

			return std::tuple_size<tuple_t>::value;
		}
	};
	////////////////////////////////////////////////////////////
	////
	template < typename T >
	typename T::return_t call_impl(lua_State* l, const T &obj, int offset)
	{
		typedef typename T::params_t args_t;
		typedef typename T::tuple_t tuple_t;

		enum
		{
			TUPLE_SIZE = std::tuple_size<args_t>::value == 0 ? 0 : std::tuple_size<args_t>::value - 1
		};
		args_t args;
		args_parser_t<TUPLE_SIZE>::parse(l, std::tuple_size<args_t>::value + offset, args);

		return caller_t::call(obj, args, seq_t<std::tuple_size<args_t>::value>());
	}

	template < typename T, typename ...Args >
	inline void call(lua_State* l, T** ppobj, const std::tuple<Args...> &args)
	{
		call_impl(l, make_obj(ppobj, args), 1);
	}

	template < typename R, typename T, typename ...Args >
	int call(lua_State* l, T *obj, R(T::*handler)(Args...), int offset = 0,
		typename std::enable_if<!std::is_same<R, void>::value>::type * = nullptr)
	{
		using result_t = typename std::remove_cv<R>::type;
		return lua::push(l, call_impl(l, make_obj(obj, handler), offset));
	}

	template < typename R, typename T, typename ...Args >
	int call(lua_State* l, T *obj, R(T::*handler)(Args...), int offset = 0,
		typename std::enable_if<std::is_same<R, void>::value>::type * = nullptr)
	{
		call_impl(l, make_obj(obj, handler), offset);
		return 0;
	}

	template < typename R, typename ...Args >
	int call(lua_State* l, R(*handler)(Args...),
		typename std::enable_if<!std::is_same<R, void>::value>::type * = nullptr)
	{
		using result_t = typename std::remove_cv<R>::type;
		return lua::push(l, call_impl(l, make_obj(handler), 0));
	}

	template < typename R, typename ...Args >
	int call(lua_State* l, R(*handler)(Args...),
		typename std::enable_if<std::is_same<R, void>::value>::type * = nullptr)
	{
		call_impl(l, make_obj(handler), 0);
		return 0;
	}


	template < typename HandlerT, typename R, typename ...Args >
	int call(lua_State* l, HandlerT *handler,
		typename std::enable_if<!std::is_same<R, void>::value>::type * = nullptr)
	{
		using result_t = typename std::remove_cv<R>::type;
		return lua_op_t<result_t>::push_stack(l, call_impl(l, make_obj<R, HandlerT, Args...>(handler), 0));
	}

	template < typename HandlerT, typename R, typename ...Args >
	int call(lua_State* l, HandlerT *handler,
		typename std::enable_if<std::is_same<R, void>::value>::type * = nullptr)
	{
		call_impl(l, make_obj<R, HandlerT, Args...>(handler), 0);
		return 0;
	}


	////////////////////////////////////////////////////////////
	////
	template < typename R, typename ...Args >
	struct free_function_t
	{
		const char *name_;

		typedef R(*function_t)(Args...);
		function_t function_;

		free_function_t(const char *name, function_t func)
			: name_(name)
			, function_(func)
		{}

		free_function_t &operator=(const free_function_t &) = delete;
	};
	template < typename R, typename V >
	struct field_function_t
	{
		const char *name_;
		typedef V(R::*field_t);
		field_t field_;
		field_function_t(const char *name, V R::*val)
			: name_(name)
			, field_(val)
		{}

		field_function_t &operator=(const field_function_t &) = delete;
	};

	template < typename V>
	struct staticfield_function_t
	{
		const char *name_;
		typedef V* field_t;
		field_t field_;
		staticfield_function_t(const char *name, V *val)
			: name_(name)
			, field_(val)
		{}

		staticfield_function_t &operator=(const staticfield_function_t &) = delete;
	};

	template < typename R, typename T, typename ...Args >
	struct class_function_t
	{
		const char *name_;

		typedef R(T::*function_t)(Args...);
		typedef R(T::*const_function_t)(Args...) const;
		typedef R(T::*volatile_function_t)(Args...) volatile;
		T *obj_;
		function_t function_;

		class_function_t(const char *name, T *obj, function_t func)
			: name_(name)
			, obj_(obj)
			, function_(func)
		{}
		class_function_t(const char *name, T *obj, const_function_t func)
			: name_(name)
			, obj_(obj)
			, function_(reinterpret_cast<function_t>(func))
		{}
		class_function_t(const char *name, T *obj, volatile_function_t func)
			: name_(name)
			, obj_(obj)
			, function_(reinterpret_cast<function_t>(func))
		{}

		class_function_t &operator=(const class_function_t &) = delete;
	};
	
	template < typename ...Args >
	struct lambda_function_t;

	template < typename HandlerT, typename R, typename ...Args >
	struct lambda_function_t<HandlerT, R, std::tuple<Args...>>
	{
		using function_t = R(*)(Args...);
		using this_t = lambda_function_t<HandlerT, R, std::tuple<Args...>>;

		const char *name_;
		HandlerT obj_;

		lambda_function_t(const char *name, HandlerT &&handler)
			: name_(name)
			, obj_(std::forward<HandlerT>(handler))
		{}

		lambda_function_t &operator=(const lambda_function_t &) = delete;

		R on_handler(Args &&...args)
		{
			return HandlerT(std::forward<Args>(args)...);
		}
	};
	
	struct lua_function_t
	{
		using function_t = int(*)(lua_State*);
		const char *name_;
		function_t func_;

		lua_function_t(const char *name, const function_t& func)
			: name_(name)
			, func_(func)
		{}
		lua_function_t &operator=(const lua_function_t &) = delete;
	};
	////////////////////////////////////////////////////////////
	////
	template < typename ...Args >
	struct constructor_t {};

	struct destructor_t {};

	template < typename ...Args >
	inline constructor_t<Args...> constructor()
	{
		return constructor_t<Args...>();
	}

	inline destructor_t destructor()
	{
		return destructor_t();
	}

	template < typename T >
	struct class_name_t
	{
		static std::string name_;
		static std::string meta()
		{
			return name_ + "_meta";
		}
	};

	template < typename T >
	std::string class_name_t<T>::name_;

	template < typename T >
	class lua_register_t;

	struct LuaUserData
	{
		void * object;
		LuaUserData * stamp;
		int	flag;
	};

#ifdef OBJECT_HEAP_RECORD
	extern int get_object_flag(void* p);
#endif

	class ObjectsInLuaRec
	{
		typedef LuaUserData *  StampT;
		struct ObjectRec
		{
			void * object;
			StampT stamp;

			ObjectRec(void * obj, StampT stamp_) : object(obj), stamp(stamp_)
			{}
		};

		struct Comp_Rec
		{
			bool operator() (const ObjectRec& lhs, const ObjectRec& rhs) const
			{
				return (static_cast<char*>(lhs.object) - static_cast<char*>(rhs.object)) < 0;
			}
		};

		ObjectsInLuaRec() {}

		std::set<ObjectRec, Comp_Rec> ObjectsInLua;

	public:
		static ObjectsInLuaRec& Get()
		{
			static ObjectsInLuaRec s_inst;
			return s_inst;
		}

		inline void AddObject(void * object, StampT stamp)
		{
			ObjectRec rec = ObjectRec(object, stamp);
			auto found = ObjectsInLua.find(rec);
			if (found != ObjectsInLua.end())
			{
				const ObjectRec & rec2 = *found;
				if (rec2.stamp != stamp)
				{
					ObjectsInLua.erase(found);
					ObjectsInLua.insert(rec);
				}
			}
			else
			{
				ObjectsInLua.insert(rec);
			}
		}
		inline bool RemoveObject(void * object, StampT stamp, bool force = false)
		{
			auto found = ObjectsInLua.find(ObjectRec(object, 0));
			if (found != ObjectsInLua.end())
			{
				const ObjectRec & rec = *found;
				if (rec.stamp == stamp || force)
				{
					ObjectsInLua.erase(found);
					return true;
				}
			}
			return false;
		}
		inline bool FindObject(void * object, StampT stamp)
		{
			auto found = ObjectsInLua.find(ObjectRec(object, stamp));
			if (found != ObjectsInLua.end())
			{
				const ObjectRec&  rec = *found;
				return rec.stamp == stamp;
			}
			return false;
		}
	};

	struct lua_hash_objs
	{
		std::vector<void*> functions;
		int mobjWeakTableRef;
		lua_hash_objs() :mobjWeakTableRef(LUA_NOREF)
		{
			functions.clear();
		}
		~lua_hash_objs()
		{
			for (void*p : functions)
			{
				free(p);
			}
			functions.clear();
		}
		inline int PushObjectToLua(lua_State* l, const void* const_obj, const std::string& metatable)
		{
			if (!const_obj)
			{
				lua_pushnil(l);
				return 1;
			}
			void* obj = const_cast<void*>(const_obj);
			return PushObjectToLua(l, obj, metatable);
		}

		inline int PushObjectToLua(lua_State* l, void* obj, const std::string& metatable)
		{
			if (!obj)
			{
				lua_pushnil(l);
				return 1;
			}
			tryGetUserdataFromWeakTable(l, obj);
#ifdef OBJECT_HEAP_RECORD
			int flag = get_object_flag(obj);
#else
			int flag = 0;
#endif
			if (lua_isnil(l, -1))
			{
				lua_pop(l, 1); //t
				LuaUserData* userdata = (LuaUserData*)lua_newuserdata(l, sizeof(LuaUserData)); //t,ud
				userdata->object = obj;
				userdata->flag = flag;
				userdata->stamp = userdata;
				ObjectsInLuaRec::Get().AddObject(obj, userdata->stamp);

				luaL_getmetatable(l, metatable.c_str());//t,ud,mt
				lua_setmetatable(l, -2);//t,ud
				lua_pushlightuserdata(l, obj); //t,ud,obj
				lua_pushvalue(l, -2); //t,ud,obj,ud
				lua_settable(l, -4); //t,ud
			}
			else
			{
				LuaUserData * userdata = (LuaUserData*)lua_touserdata(l, -1);
				userdata->object = obj;
				assert(userdata->flag == flag);
			}

			lua_remove(l, -2); //ud

			return 1;
		}
		
		inline void* GetObject(lua_State * L, int ParamIndex, const char* mtname, bool poperror=true)
		{
			LuaUserData * userdata = GetObjectUserData(L, ParamIndex, mtname);
			if (userdata && !userdata->object)
			{
				if(poperror)
				{
					lua_pushstring(L, "Content of LuaUserData has been removed!");
					lua_error(L);
				}
				return nullptr;
			}
			if (userdata && userdata->stamp && !ObjectsInLuaRec::Get().FindObject(userdata->object, userdata->stamp))
			{
				userdata->object = nullptr;
				if(poperror)
				{
					lua_pushstring(L, "Content of LuaUserData has been disappeared!");
					lua_error(L);
				}
				return nullptr;
			}
			return userdata != nullptr ? userdata->object : nullptr;
		}

		inline void* GetObject(lua_State* L, int ParamIndex, bool poperror=true)
		{
			if (!lua_isuserdata(L, ParamIndex))
			{
				if(poperror)
				{
					lua_pushstring(L, "GetObject #1 must be userdata");
					lua_error(L);
				}
				return nullptr;
			}
			lua_getmetatable(L, ParamIndex);//ud, mt
			lua_pushstring(L, "__mtname");//ud, mt, key
			lua_rawget(L, -2); //ud, mt, mtname
			if (lua_isstring(L, -1))
			{
				const char* mtname = lua_tostring(L, -1);
				lua_pop(L, 2); //ud
				return GetObject(L, ParamIndex, mtname, poperror);
			}
			else
			{
				lua_pop(L, 2); //ud
				return GetObject(L, ParamIndex, nullptr, poperror);
			}
		}

		bool RemoveObjectFromLua(lua_State* l, int* flag = nullptr)
		{
			LuaUserData* ud = GetObjectUserData(l, 1);
			if (ud && ud->object != nullptr)
			{
				bool  owner = ObjectsInLuaRec::Get().RemoveObject(ud->object, ud->stamp);
				if (owner)
				{
					if(flag) *flag = ud->flag;
					removeUserdataFromWeakTable(l, ud->object);
				}
				ud->object = nullptr;
			}
			return true;
		}

		bool RemoveObject(lua_State* l, void* ptr)
		{
			if(!ptr) return true;
			LuaUserData ud;
			ud.flag = 1;
			ud.object = ptr;
			ud.stamp = &ud;
			bool owner = ObjectsInLuaRec::Get().RemoveObject(ud.object, ud.stamp, true);
			if (owner)
			{
				removeUserdataFromWeakTable(l, ptr);
			}
			return true;
		}

		template<typename T>
		T* CheckObject(lua_State* l, int index, bool poperror=true)
		{
			if (!lua_isuserdata(l, index))
			{
				char err[100] = { 0 };
				sprintf(err, "param #%d must be a userdata", index);
				lua_pushstring(l, err);
				lua_error(l);
				return NULL;
			}
			void* obj = GetObject(l, index, poperror);
			return static_cast<T *>(obj);
		}

		template<typename F>
		void* castVoid(F f)
		{
			//return &f;
			void* p = malloc(sizeof(F));
			::new(p) F(f);
			functions.push_back(p);
			return p;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline LuaUserData* GetObjectUserData(lua_State * L, int ParamIndex, const char* mtname)
		{
			if (!lua_isuserdata(L, ParamIndex))
			{
				lua_pushstring(L, "GetObject #1 must be userdata");
				lua_error(L);
				return nullptr;
			}
			LuaUserData * userdata = nullptr;
			if(mtname)
				userdata = (LuaUserData*)luaL_checkudata(L, ParamIndex, mtname);
			else
				userdata = (LuaUserData *)lua_touserdata(L, ParamIndex);
			return userdata;
		}

		inline LuaUserData* GetObjectUserData(lua_State * L, int ParamIndex)
		{
			if (!lua_isuserdata(L, ParamIndex))
			{
				lua_pushstring(L, "GetObject #1 must be userdata");
				lua_error(L);
				return nullptr;
			}
			lua_getmetatable(L, ParamIndex);//mt
			lua_pushstring(L, "__mtname");//mt,key
			lua_rawget(L, -2); //mt, mtname
			if (lua_isstring(L, -1))
			{
				const char* mtname = lua_tostring(L, -1);
				lua_pop(L, -2);
				return GetObjectUserData(L, ParamIndex, mtname);
			}
			else
			{
				lua_pop(L, -2);
				return GetObjectUserData(L, ParamIndex, nullptr);
			}
		}
	
		void InitObjsWeakTable(lua_State* l)
		{
			lua_newtable(l); // t
			lua_newtable(l); // t,mt
			lua_pushstring(l, "v"); // t,mt,"v"
			lua_setfield(l, -2, "__mode"); //t,mt
			lua_setmetatable(l, -2); //t
			mobjWeakTableRef = luaL_ref(l, LUA_REGISTRYINDEX);
		}

		void tryGetUserdataFromWeakTable(lua_State * L, void * Obj)
		{
			if (mobjWeakTableRef == LUA_NOREF)
			{
				InitObjsWeakTable(L);
			}
			lua_rawgeti(L, LUA_REGISTRYINDEX, mobjWeakTableRef); //t
			lua_pushlightuserdata(L, Obj); //t,obj
			lua_gettable(L, -2); //t,ud
		}

		void removeUserdataFromWeakTable(lua_State * L, void * Obj)
		{
			if (mobjWeakTableRef == LUA_NOREF)
			{
				return;
			}
			lua_rawgeti(L, LUA_REGISTRYINDEX, mobjWeakTableRef); //t
			lua_pushlightuserdata(L, Obj); //t,obj
			lua_pushnil(L);
			lua_settable(L, -3);
			lua_pop(L, 1);
		}
	};
	//需要定义该变量
	static lua_hash_objs& get_luaobj_container() {
		static lua_hash_objs lua_objs;
		return lua_objs;
	}
	//extern lua_hash_objs& get_luaobj_container();


	template<typename T>
	struct lua_op_t < T* >
	{
		static int push_stack(lua_State* l, T* value)
		{
			if (value)
			{
				if (class_name_t<T>::name_.empty())
					lua_pushlightuserdata(l, (void *)(value));
				else
				{
					get_luaobj_container().PushObjectToLua(l, (void *)(value), class_name_t<T>::meta());
				}
			}
			else
				lua_pushnil(l);
			return 1;
		}
		static void from_stack(lua_State* l, int pos, T** value)
		{
			if (lua_isnoneornil(l, pos))
			{
				*value = 0;
				return;
			}
			LUA_CHECK_ERROR(lua_isuserdata(l, pos) != 0, LUA_TUSERDATA, pos);
			*value = get_luaobj_container().CheckObject<T>(l, pos);
		}

		static bool try_get(lua_State * l, int pos, T** value)
		{
			if (lua_isnil(l, pos) || lua_isuserdata(l, pos))
			{
				if (lua_isnoneornil(l, pos))
				{
					*value = 0;
					return true;
				}
				LUA_CHECK_ERROR(lua_isuserdata(l, pos) != 0, LUA_TUSERDATA, pos);
				*value = get_luaobj_container().CheckObject<T>(l, pos, false);
				return true;
			}
			else
				return false;
		}
	};

	template < typename R, typename ...Args >
	inline free_function_t<R, Args...> def(const char *name, R(*func)(Args...))
	{
		return free_function_t<R, Args...>(name, func);
	}

	//template < typename HandlerT >
	//inline auto def(const char *name, HandlerT &&handler)->
	// 	typename lambda_function_t<HandlerT, typename function_traits_t<HandlerT>::result_type, typename function_traits_t<HandlerT>::args_type>
	//{
	// 	typedef typename function_traits_t<HandlerT>::args_type	tuple_t;
	// 	typedef typename function_traits_t<HandlerT>::result_type	R;

	// 	return lambda_function_t<HandlerT, R, tuple_t>(name, std::forward<HandlerT>(handler));
	//}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, T *obj, R(T::*func)(Args...))
	{
		return class_function_t<R, T, Args...>(name, obj, func);
	}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, T *obj, R(T::*func)(Args...) const)
	{
		return class_function_t<R, T, Args...>(name, obj, func);
	}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, T *obj, R(T::*func)(Args...) volatile)
	{
		return class_function_t<R, T, Args...>(name, obj, func);
	}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, R(T::*func)(Args...))
	{
		return class_function_t<R, T, Args...>(name, nullptr, func);
	}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, R(T::*func)(Args...) const)
	{
		return class_function_t<R, T, Args...>(name, nullptr, func);
	}

	template < typename R, typename T, typename ...Args >
	inline class_function_t<R, T, Args...> def(const char *name, R(T::*func)(Args...) volatile)
	{
		return class_function_t<R, T, Args...>(name, nullptr, func);
	}

	template < typename R, typename V>
	inline field_function_t<R, V> def(const char* name, V R::*val)
	{
		return field_function_t<R, V>(name, val);
	}
	template <typename V>
	inline staticfield_function_t<V> def(const char* name, V *val)
	{
		return staticfield_function_t<V>(name, val);
	}

	inline lua_function_t def(const char *name, int(*func)(lua_State*))
	{
		return lua_function_t(name, func);
	}
	////////////////////////////////////////////////////////////
	////
	////////////////////////////////////////////////////////////
	template<typename C>
	static void __delete(void* obj)
	{
		C* p = static_cast<C*>(obj);
		delete p;
	}
	typedef void(*__deleteObjdelegate)(void*);
	template<typename C>
	struct deleteDelegate
	{
		static __deleteObjdelegate m_deleteObjdelegate;
	};
	template<typename C>
	__deleteObjdelegate deleteDelegate<C>::m_deleteObjdelegate = nullptr;

	class lua_register_base_t
	{
	public:
		lua_register_base_t(lua_State* l_) : m_ls(l_) {}
	protected:
		lua_State* 	m_ls;

		template<typename A>
		struct base_type { typedef A type; };
		template<typename A>
		struct base_type<A*> { typedef A type; };
		template<typename A>
		struct base_type<A&> { typedef A type; };

		template<bool C, typename A, typename B> struct if_ {};
		template<typename A, typename B>		struct if_<true, A, B> { typedef A type; };
		template<typename A, typename B>		struct if_<false, A, B> { typedef B type; };

		template<typename A>
		struct is_ptr { static const bool value = false; };
		template<typename A>
		struct is_ptr<A*> { static const bool value = true; };

		template<typename A>
		struct is_ref { static const bool value = false; };
		template<typename A>
		struct is_ref<A&> { static const bool value = true; };

		template<typename C>
		struct void2val { static C invoke(void* input) { return *(C*)input; } };
		template<typename C>
		struct void2ptr { static C* invoke(void* input) { return (C*)input; } };
		template<typename C>
		struct void2ref { static C& invoke(void* input) { return *(C*)input; } };

		template<typename C>
		struct void2type
		{
			static C invoke(void* ptr)
			{
				return	if_<is_ptr<C>::value
					, void2ptr<typename base_type<C>::type>
					, typename if_<is_ref<C>::value
					, void2ref<typename base_type<C>::type>
					, void2val<typename base_type<C>::type>
					>::type
				>::type::invoke(ptr);
			}
		};
		// member variable
		struct var_base
		{
			virtual void get(lua_State *L) = 0;
			virtual void set(lua_State *L) = 0;
		};

		template<typename C, typename V>
		struct mem_var_readwrite : var_base
		{
			V C::*_var;
			mem_var_readwrite(V C::*val) : _var(val) {}
			void get(lua_State *L) {
				C* obj = NULL;
				lua::get(L, 1, &obj);
				lua::push(L, obj->*(_var));
			}
			void set(lua_State *L) {
				C* obj = NULL;
				lua::get(L, 1, &obj);
				V val = V();
				lua::get(L, 3, &val);
				obj->*(_var) = val;
			}
		};
		template<typename C, typename V>
		struct mem_var_read : var_base
		{
			V C::*_var;
			mem_var_read(V C::*val) : _var(val) {}
			void get(lua_State *L) {
				C* obj = NULL;
				lua::get(L, 1, &obj);
				lua::push(L, obj->*(_var));
			}
			void set(lua_State *L) {
				luaL_error(L, "can't set to a readonly variale '%s'", lua_tostring(L, 2));
			}
		};
		template<typename C, typename V>
		struct mem_var_write : var_base
		{
			V C::*_var;
			mem_var_write(V C::*val) : _var(val) {}
			void get(lua_State *L) {
				luaL_error(L, "can't access a writeonly variable '%s'", lua_tostring(L, 2));
			}
			void set(lua_State *L) {
				C* obj = NULL;
				lua::get(L, 1, &obj);
				V val = V();
				lua::get(L, 3, &val);
				obj->*(_var) = val;
			}
		};

		template<typename V>
		struct mem_var_static_readwrite : var_base
		{
			V *_var;
			mem_var_static_readwrite(V *val) : _var(val) {}
			void get(lua_State *L) {
				lua::push(L, *_var);
			}
			void set(lua_State *L) {
				V val = V();
				lua::get(L, 3, &val);
				*_var = val;
			}
		};
		template<typename V>
		struct mem_var_static_read : var_base
		{
			V *_var;
			mem_var_static_read(V *val) : _var(val) {}
			void get(lua_State *L) {
				lua::push(L, *_var);
			}
			void set(lua_State *L) {
				luaL_error(L, "can't set to a readonly variale '%s'", lua_tostring(L, 2));
			}
		};
		template<typename V>
		struct mem_var_static_write : var_base
		{
			V *_var;
			mem_var_static_write(V *val) : _var(val) {}
			void get(lua_State *L) {
				luaL_error(L, "can't access a writeonly variable '%s'", lua_tostring(L, 2));
			}
			void set(lua_State *L) {
				V val = V();
				lua::get(L, 3, &val);
				*_var = val;
			}
		};
		
		static void invoke_parent(lua_State* L)
		{
			lua_pushstring(L, "__parent");//t, key, mt, k
			lua_rawget(L, -2);//t, key, mt, t
			if (lua_istable(L, -1))
			{
				lua_pushvalue(L, 2);//t, key, mt, t, key
				lua_rawget(L, -2);//t, key, mt, t, v
				if (!lua_isnil(L, -1))
				{
					lua_remove(L, -2);//t, key, mt, v
				}
				else
				{
					lua_remove(L, -1);
					invoke_parent(L);
					lua_remove(L, -2);
				}
			}
		}

		static int __index(lua_State* l, int index, bool strict = true)
		{
			int type = lua_type(l, 1);//t, key
			lua_getmetatable(l, 1);//t, key, mt
			lua_pushvalue(l, 2);//t, key, mt, key
			lua_rawget(l, -2);//t, key, mt, value

			if (lua_isuserdata(l, -1))
			{
				void* obj = lua_touserdata(l, -1);
				void2type<var_base*>::invoke(obj)->get(l);//t, key, mt, value, val
				lua_remove(l, -2);
			}
			else if (lua_isnil(l, -1))
			{
				lua_remove(l, -1);//t, key, mt
				invoke_parent(l);//t, key, mt, v
				if (strict && lua_isnil(l, -1) && LUA_TUSERDATA == type)
				{
					luaL_error(l, "can't find '%s' class variable. (forgot registering class variable ?)", lua_tostring(l, 2));
				}
			}
			lua_remove(l, -2);

			return 1;
		}
		static int __newindex(lua_State* l)
		{//t,key,value
			lua_getmetatable(l, 1);//t,key,value,mt
			lua_pushvalue(l, 2);//t,key,value,mt,key
			lua_rawget(l, -2);//t,key,value, mt, val
			if (lua_isuserdata(l, -1))
			{
				void* obj = lua_touserdata(l, -1);
				lua_pop(l, 2);//t,key,value
				void2type<var_base*>::invoke(obj)->set(l); //t, key, value
			}
			else if (lua_isnil(l, -1))
			{
				lua_pushvalue(l, 2);//t, key, value, mt, nil, b
				lua_pushvalue(l, 3);
				lua_rawset(l, -4);
			}
			lua_settop(l, 3);
			return 0;
		}

		void createtable(lua_State* l, const char* name)
		{
			lua_getglobal(l, "_G");
			std::string tn;
			for (size_t i = 0; i < strlen(name); ++i)
			{
				if (name[i] == '.')
				{
					if (tn.empty()) continue;
					lua_pushstring(l, tn.c_str());
					lua_rawget(l, -2);
					if (lua_isnil(l, -1))
					{
						lua_pop(l, 1);
						lua_createtable(l, 0, 0);
						lua_pushstring(l, tn.c_str());
						lua_pushvalue(l, -2);
						lua_rawset(l, -4);
					}
					lua_remove(l, -2);
					tn.clear();
				}
				else
				{
					tn += name[i];
				}
			}
			if (!tn.empty())
			{
				lua_pushstring(l, tn.c_str());
				lua_rawget(l, -2);
				if (lua_isnil(l, -1))
				{
					lua_pop(l, 1);
					lua_createtable(l, 0, 0);
					lua_pushstring(l, tn.c_str());
					lua_pushvalue(l, -2);
					lua_rawset(l, -4);
				}
				lua_remove(l, -2);
			}
		}
		void gettable(lua_State* l, const char* name)
		{
			lua_getglobal(l, "_G");
			std::string tn;
			for (size_t i = 0; i < strlen(name); ++i)
			{
				if (name[i] == '.')
				{
					if (tn.empty()) continue;
					lua_pushstring(l, tn.c_str());
					lua_rawget(l, -2);
					if (lua_isnil(l, -1))
					{
						lua_pop(l, 1);
					}
					lua_remove(l, -2);
					tn.clear();
				}
				else
				{
					tn += name[i];
				}
			}
			if (!tn.empty())
			{
				lua_pushstring(l, tn.c_str());
				lua_rawget(l, -2);
				if (lua_isnil(l, -1))
				{
					lua_pop(l, 1);
				}
				lua_remove(l, -2);
			}
		}
	};

	template < typename T >
	class lua_register_t : lua_register_base_t
	{
	protected:
		static int __gc(lua_State* l, bool gc)
		{
			T *obj = NULL; lua::get(l, 1, &obj);
			if (!obj)
			{
				luaL_error(l, "__gc method on handler error,%s:%d", __FILE__, __LINE__);
				return 1;
			}
			int flag = 0;
			bool success = get_luaobj_container().RemoveObjectFromLua(l, &flag);
#ifdef OBJECT_HEAP_RECORD
			if (1==flag && success && deleteDelegate<T>::m_deleteObjdelegate) deleteDelegate<T>::m_deleteObjdelegate(obj);
#else
			if (!gc && success && deleteDelegate<T>::m_deleteObjdelegate) deleteDelegate<T>::m_deleteObjdelegate(obj);
#endif
			return 0;
		}

		lua_register_t & tryget()
		{
			auto lambda = [](lua_State *l)->int
			{
				T *obj = NULL;
				lua::get(l, 1, &obj);
				if (!obj)
				{
					luaL_error(l, "tryget method on handler error,%s:%d", __FILE__, __LINE__);
					return 0;
				}
				return __index(l, 2, false);
			};

			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, "tryget");
			lua_pushcfunction(m_ls, lambda);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//成员函数
		template < typename R, typename U, typename ...Args >
		lua_register_t& def(const class_function_t<R, U, Args...> &func)
		{
			typedef typename class_function_t<R, U, Args...>::function_t function_t;

			auto lambda = [](lua_State *l)->int
			{
				U *obj = NULL;
				lua::get(l, 1, &obj);
				if (!obj)
				{
					luaL_error(l, "class method on handler error,%s:%d", __FILE__, __LINE__);
					return 0;
				}

				auto function = static_cast<function_t *>(lua_touserdata(l, lua_upvalueindex(1)));

				return call(l, obj, *function, 1);
			};

			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, func.name_);
			lua_pushlightuserdata(m_ls, get_luaobj_container().castVoid(func.function_));
			lua_pushcclosure(m_ls, lambda, 1);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//静态以及全局函数
		template < typename R, typename ...Args >
		lua_register_t& def(const free_function_t<R, Args...> &func)
		{
			auto lambda = [](lua_State *l)->int
			{
				typedef typename free_function_t<R, Args...>::function_t function_t;
				auto function = (function_t)(lua_touserdata(l, lua_upvalueindex(1)));

				return call(l, function);
			};
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, func.name_);
			lua_pushlightuserdata(m_ls, (void *)func.function_);
			lua_pushcclosure(m_ls, lambda, 1);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//lua函数
		lua_register_t& def(const lua_function_t &func)
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, func.name_);
			lua_pushcfunction(m_ls, func.func_);
			lua_rawset(m_ls, -3);

			return *this;
		}
	public:
		explicit lua_register_t(lua_State* ls_, const char* name, __deleteObjdelegate deleteFunc = __delete<T>) : lua_register_base_t(ls_)
		{
			stack_gurad scope_check(m_ls);

			assert(class_name_t<T>::name_.empty());
			class_name_t<T>::name_ = name;
			deleteDelegate<T>::m_deleteObjdelegate = deleteFunc;
		
			createtable(m_ls, class_name_t<T>::name_.c_str());
			int table = lua_gettop(m_ls);
			lua_pushstring(m_ls, "__typetable");
			lua_newtable(m_ls); // mt for method table
			int typetable = lua_gettop(m_ls);
			lua_settable(m_ls, -3);

			luaL_newmetatable(m_ls, class_name_t<T>::meta().c_str());
			int meta = lua_gettop(m_ls);

			lua_pushstring(m_ls, "__mtname");
			lua_pushstring(m_ls, class_name_t<T>::meta().c_str());
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__name");
			lua_pushstring(m_ls, class_name_t<T>::name_.c_str());
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__index");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				const char* key = lua_tostring(l, -1);

				if (strcmp(key, "isnil") == 0)
				{
					T* self = nullptr;
					bool isnil = true;
					if(lua::tryget(l, 1, &self))
						isnil = self == nullptr;
					lua::push(l, isnil);
					return 1;
				}

				return __index(l, -1, true);
			}, 0);
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__newindex");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __newindex(l);
			}, 0);
			lua_rawset(m_ls, -3);

			//gc
			lua_pushstring(m_ls, "__gc");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __gc(l, true);
			}, 0);
			lua_rawset(m_ls, -3);
			//tryget
			tryget();
			////meta
			lua_pushstring(m_ls, "__metatable");
			lua_pushvalue(m_ls, table);
			lua_settable(m_ls, meta);

			//type
			lua_pushvalue(m_ls, table);
			lua_pushstring(m_ls, "__typetable");
			lua_rawget(m_ls, -2);
			int mt = lua_gettop(m_ls);

			lua_pushstring(m_ls, "__index");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __index(l, -1, true);
			}, 0);
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__newindex");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __newindex(l);
			}, 0);
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__parent");
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__name");
			lua_pushstring(m_ls, class_name_t<T>::name_.c_str());
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__mtname");
			lua_pushstring(m_ls, class_name_t<T>::meta().c_str());
			lua_rawset(m_ls, -3);

			gettable(m_ls, class_name_t<T>::name_.c_str());
			lua_pushvalue(m_ls, mt);
			lua_setmetatable(m_ls, -2);
		}

		template<typename P>
		lua_register_t& extend()
		{
			stack_gurad scope_check(m_ls);
			assert(!class_name_t<P>::name_.empty());
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, "__parent");
			luaL_getmetatable(m_ls, class_name_t<P>::meta().c_str());
			lua_rawset(m_ls, -3);

			return (*this);
		}
		//构造函数
		template < typename ...Args >
		lua_register_t& def(const constructor_t<Args...> &)
		{
			stack_gurad scope_check(m_ls);

			typedef std::tuple<Args...> tuple_t;
			auto lambda = [](lua_State *l)->int
			{
				T* val = NULL;
				call(l, &val, tuple_t());
				lua::push(l, val);

				return 1;
			};

			gettable(m_ls, class_name_t<T>::name_.c_str());
			lua_pushstring(m_ls, "__typetable");
			lua_rawget(m_ls, -2);

			lua_pushliteral(m_ls, "__call");
			lua_pushcclosure(m_ls, lambda, 0);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//析构函数
		lua_register_t& def(const destructor_t &func)
		{
			auto lambda = [](lua_State *l)->int
			{
				return __gc(l, false);
			};
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, "delete");
			lua_pushcfunction(m_ls, lambda);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//静态以及全局函数
		template < typename R, typename ...Args >
		lua_register_t& def(const char* name, R(*func)(Args...))
		{
			return def(lua::def(name, func));
		}
		//lua函数
		lua_register_t& def(const char* name, int(*func)(lua_State*))
		{
			return def(lua::def(name, func));
		}
		//成员函数
		template < typename R, typename U, typename ...Args >
		lua_register_t& def(const char *name, R(U::*func)(Args...))
		{
			return def(lua::def(name, func));
		}
		template < typename R, typename U, typename ...Args >
		lua_register_t& def(const char *name, R(U::*func)(Args...) const)
		{
			return def(lua::def(name, func));
		}
		template < typename R, typename U, typename ...Args >
		lua_register_t& def(const char *name, R(U::*func)(Args...) volatile)
		{
			return def(lua::def(name, func));
		}
		//成员属性	
		template < typename R, typename V >
		lua_register_t& def(const char *name, V(R::*field))
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_readwrite<R, V>))) mem_var_readwrite<R, V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		template < typename R, typename V >
		lua_register_t& readonly(const char *name, V(R::*field))
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_read<R, V>))) mem_var_read<R, V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		template < typename R, typename V >
		lua_register_t& writeonly(const char *name, V(R::*field))
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_write<R, V>))) mem_var_write<R, V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//静态以及全局成员属性
		template <typename V >
		lua_register_t& def(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_readwrite<V>))) mem_var_static_readwrite<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		template <typename V >
		lua_register_t& readonly(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_read<V>))) mem_var_static_read<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		template <typename V >
		lua_register_t& writeonly(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			luaL_getmetatable(m_ls, class_name_t<T>::meta().c_str());
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_write<V>))) mem_var_static_write<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
	};

	template <>
	class lua_register_t<void> : lua_register_base_t
	{
		int         m_table;
		int         m_top;
	protected:
		//静态以及全局函数
		template < typename R, typename ...Args >
		lua_register_t& def(const free_function_t<R, Args...> &func)
		{
			auto lambda = [](lua_State *l)->int
			{
				typedef typename free_function_t<R, Args...>::function_t function_t;
				auto function = (function_t)(lua_touserdata(l, lua_upvalueindex(1)));

				return call(l, function);
			};
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, func.name_);
			lua_pushlightuserdata(m_ls, (void *)func.function_);
			lua_pushcclosure(m_ls, lambda, 1);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//lua函数
		lua_register_t& def(const lua_function_t &func)
		{
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, func.name_);
			lua_pushcfunction(m_ls, func.func_);
			lua_rawset(m_ls, -3);

			return *this;
		}
	public:
		~lua_register_t()
		{
			lua_settop(m_ls, m_top);
		}
		explicit lua_register_t(lua_State* ls_, const char* name = "_G") : lua_register_base_t(ls_)
		{
			assert(name);
			m_top = lua_gettop(m_ls);
			
			lua_newtable(m_ls);
			m_table = lua_gettop(m_ls);

			lua_pushstring(m_ls, "__index");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __index(l, -1, true);
			}, 0);
			lua_rawset(m_ls, -3);

			lua_pushstring(m_ls, "__newindex");
			lua_pushcclosure(m_ls, [](lua_State *l)->int
			{
				return __newindex(l);
			}, 0);
			lua_rawset(m_ls, -3);

			createtable(m_ls, name);
			lua_pushvalue(m_ls, m_table);
			lua_setmetatable(m_ls, -2);
		}

		//静态以及全局函数
		template < typename R, typename ...Args >
		lua_register_t& def(const char* name, R(*func)(Args...))
		{
			return def(lua::def(name, func));
		}
		//lua函数
		lua_register_t& def(const char* name, int(*func)(lua_State*))
		{
			return def(lua::def(name, func));
		}
		//全局成员属性
		template <typename V >
		lua_register_t& def(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_readwrite<V>))) mem_var_static_readwrite<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		template <typename V >
		lua_register_t& readonly(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_read<V>))) mem_var_static_read<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
		//这里必须是简单类型，复杂类型，只能用指针
		template <typename V >
		lua_register_t& readonly(const char *name, const V& val)
		{
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, name);
			lua::push(m_ls, val);
			lua_rawset(m_ls, -3);

			return *this;
		}

		template <typename V >
		lua_register_t& writeonly(const char *name, V* field)
		{
			stack_gurad scope_check(m_ls);
			lua_pushvalue(m_ls, m_table);
			lua_pushstring(m_ls, name);
			new(lua_newuserdata(m_ls, sizeof(mem_var_static_write<V>))) mem_var_static_write<V>(field);
			lua_rawset(m_ls, -3);

			return *this;
		}
	};
}
#endif//LUA_WRAPPER_HPP

