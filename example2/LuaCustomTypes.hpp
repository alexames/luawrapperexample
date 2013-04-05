#ifndef LUACUSTOMTYPES_H_
#define LUACUSTOMTYPES_H_

#include <string>

#include "lua.h"
#include "luawrapper.hpp"
#include "luawrapperutil.hpp"

#include "Vector2D.hpp"

// LuaWrapper knows about primitive types like ints and floats, but it doesn't
// know about things like std::strings or other more complicated types.
// Sometimes, rather than register the type with LuaWrapper, it's easier to
// be able to convert it to and from Lua's primitive types, like strings or
// tables.
//
// To do this, you must write luaU_check, luaU_to and luaU_push functions for
// your type. You don't always need all three, it depends on if you're pushing
// objects to Lua, getting objects from Lua, or both.

// This example uses std::string, but if you have other custom string types it
// should be easy to write versions of those functions too

template<>
struct luaU_Impl<std::string>
{
    static std::string luaU_check(lua_State* L, int index)
    {
        return std::string(luaL_checkstring(L, index));
    }

    static std::string luaU_to(lua_State* L, int index )
    {
        return std::string(lua_tostring(L, index));
    }

    static void luaU_push (lua_State* L, const std::string& val)
    {
        lua_pushstring(L, val.c_str());
    }
};


// These two functions let me convert a simple Vector2D structure into a Lua
// table holding the x and y values
template<>
struct luaU_Impl<Vector2D>
{
    static Vector2D luaU_check(lua_State* L, int index)
    {
        return Vector2D(
            luaU_getfield<float>(L, index, "x"),
            luaU_getfield<float>(L, index, "y"));
    }

    static Vector2D luaU_to(lua_State* L, int index )
    {
        return Vector2D(
            luaU_getfield<float>(L, index, "x"),
            luaU_getfield<float>(L, index, "y"));
    }

    static void luaU_push (lua_State* L, const Vector2D& val)
    {
        lua_newtable(L);
        luaU_setfield<float>(L, -1, "x", val.x);
        luaU_setfield<float>(L, -1, "y", val.y);
    }
};

#endif