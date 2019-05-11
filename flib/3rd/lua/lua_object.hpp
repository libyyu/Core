#ifndef _LUA_OBJECT_HPP
#define _LUA_OBJECT_HPP
#pragma once
#include "lua_script.hpp"

namespace lua
{
	namespace internal
	{
		struct LuaUserData
		{
			void * object;
			LuaUserData * stamp;
			int	flag;
		};

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

		class LuaObjectWrapper
		{
			typedef void (*UnRefFunc)(int);
			int mobjWeakTableRef;
			UnRefFunc onUnRef;
			LuaObjectWrapper() :mobjWeakTableRef(LUA_NOREF), onUnRef(NULL)
			{
			}
			~LuaObjectWrapper()
			{
				if(onUnRef != NULL && mobjWeakTableRef != LUA_NOREF)
				{
					onUnRef(mobjWeakTableRef);
				}
			}
		public:
			static LuaObjectWrapper& Get()
			{
				static LuaObjectWrapper wrapper;
				return wrapper;
			}
			inline int PushObjectToLua(lua_State* l, const void* const_obj, const char* mtname, int flag = 0)
			{
				if (!const_obj)
				{
					lua_pushnil(l);
					return 1;
				}
				void* obj = const_cast<void*>(const_obj);
				return PushObjectToLua(l, obj, mtname);
			}

			inline int PushObjectToLua(lua_State* l, void* obj, const char* mtname, int flag = 0)
			{
				if (!obj)
				{
					lua_pushnil(l);
					return 1;
				}
				tryGetUserdataFromWeakTable(l, obj);
				if (lua_isnil(l, -1))
				{
					lua_pop(l, 1); //t
					LuaUserData* userdata = (LuaUserData*)lua_newuserdata(l, sizeof(LuaUserData)); //t,ud
					userdata->object = obj;
					userdata->flag = flag;
					userdata->stamp = userdata;
					ObjectsInLuaRec::Get().AddObject(obj, userdata->stamp);

					luaL_getmetatable(l, mtname);//t,ud,mt
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
				ud.flag = 0;
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

		protected:
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
	}

    inline int pushobject(lua_State* l, const void* const_obj, const char* meta, int flag = 0)
    {
        return internal::LuaObjectWrapper::Get().PushObjectToLua(l, const_obj, meta, flag);
    }
    inline int pushobject(lua_State* l, void* obj, const char* meta, int flag = 0)
    {
        return internal::LuaObjectWrapper::Get().PushObjectToLua(l, obj, meta, flag);
    }

	inline bool removeobject(lua_State* l, int* flag = NULL)
	{
		return internal::LuaObjectWrapper::Get().RemoveObjectFromLua(l, flag);
	}

	template<typename T>
	inline T* getobject(lua_State* l, int pos)
	{
		return internal::LuaObjectWrapper::Get().CheckObject<T>(l, pos);
	} 
}
#endif//_LUA_OBJECT_HPP