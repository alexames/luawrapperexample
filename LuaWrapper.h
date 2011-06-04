/*
 * Copyright (c) 2010 Alexander Ames
 * Alexander.Ames@gmail.com
 * See Copyright Notice at the end of this file
 */

// API Summary:
//
// LuaWrapper is a library designed to help bridge the gab between Lua and
// C++. It is designed to be small (a single header file), simple, fast,
// and typesafe. It has no external dependencies, and does not need to be
// precompiled; the header can simply be dropped into a project and used
// immediately. It even supports class inheritance to a certain degree. Objects
// can be created in either Lua or C++, and passed back and forth.
//
// In Lua, the objects are userdata, but through tricky use of metatables, they
// can be treated almost identically to tables.
//
// The main functions of interest are the following:
//  luaW_is<T>
//  luaW_to<T>
//  luaW_check<T>
//  luaW_push<T>
//  luaW_register<T>
//  luaW_hold<T>
//  luaW_release<T>
//  luaW_clean<T>
//
// These functions allow you to manipulate arbitrary classes just like you
// would the primitive types (e.g. numbers or strings). When all references
// to a userdata removed, the userdata will be deleted. In some cases, this
// may not be what you want, such as cases where an object is created in Lua,
// then passed to C++ code which owns it from then on. In these cases, you
// can call luaW_release, which releases LuaWrapper's hold on the userdata.
// This prevents it from being deallocated when all references disappear.
// When this is called, you are now responsible for calling luaW_clean and
// luaW_destructor manually when you are done with the object. Conversely, if
// an object is created in C++, but would like to pass ownership over to Lua,
// luaW_hold may be used.
//
// Additionally, metamethods __ctor and __dtor are provided, and will run when
// objects are created or destroyed respectively. Objects can also declare a
// list of other tables that they extend, and they will inherit all functions
// from that class.

#ifndef LUA_WRAPPER_H_
#define LUA_WRAPPER_H_

#include <iostream>
extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
}

#define LUAW_BUILDER

#define luaW_getregistry(L, s) \
     lua_getfield(L, LUA_REGISTRYINDEX, s)

#define luaW_setregistry(L, s) \
     lua_setfield(L, LUA_REGISTRYINDEX, s)

#define LUAW_CTOR_KEY "__ctor"
#define LUAW_DTOR_KEY "__dtor"
#define LUAW_EXTENDS_KEY "__extends"
#define LUAW_STORAGE_KEY "__storage"
#define LUAW_COUNT_KEY "__counts"
#define LUAW_HOLDS_KEY "__holds"
#define LUAW_WRAPPER_KEY "LuaWrapper"

#if 0
// For Debugging
// Prints the current Lua stack, including the values for some types
template <typename T>
void luaW_printstack(lua_State* L)
{
    int stack = lua_gettop(L);
    for (int i = 1; i <= stack; i++)
    {
        std::cout << std::dec << i << ": " << lua_typename(L, lua_type(L, i));
        switch(lua_type(L, i))
        {
        case LUA_TBOOLEAN: std::cout << " " << lua_toboolean(L, i); break;
        case LUA_TSTRING: std::cout << " " << lua_tostring(L, i); break;
        case LUA_TNUMBER: std::cout << " " << std::dec << (uintptr_t)lua_tointeger(L, i) << " (0x" << std::hex << lua_tointeger(L, i) << ")"; break;
        default: std::cout << " " << std::hex << lua_topointer(L, i); break;
        }
        std::cout << std::endl;
    }
}
#define LUAW_TRACE() \
    printf("%s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#else
#define LUAW_TRACE()
#endif

// These are the default allocator and deallocator. If you would prefer an
// alternative option, you may select a different function when registering
// your class
template <typename T>
T* luaW_defaultallocator()
{
    return new T();
}
template <typename T>
void luaW_defaultdeallocator(T* obj)
{
    delete obj;
}

// This class is used with luaW_register as an alternative to using the
// normal constructor. Sometimes it's just easier to fill in the fields
// of a struct than to file in all the arguments in luaW_register,
// especially if you just want to set the last one or two.
template <typename T>
struct LuaWrapperOptions
{
    LuaWrapperOptions(
        const luaL_reg* table = NULL, const luaL_reg* metatable = NULL, const char** extends = NULL, bool disablenew = false, T* (*allocator)() = luaW_defaultallocator<T>, void (*deallocator)(T*) = luaW_defaultdeallocator<T>)
        : table(table), metatable(metatable), extends(extends), disablenew(disablenew), allocator(allocator), deallocator(deallocator) { }

    const luaL_reg* table;
    const luaL_reg* metatable;
    const char** extends;
    bool disablenew;
    T* (*allocator)();
    void (*deallocator)(T*);
};

// This class cannot actually to be instantiated. It is used only hold the
// table name and other information.
template <typename T>
class LuaWrapper
{
public:
    static const char* classname;
    static T* (*allocator)();
    static void (*deallocator)(T*);
private:
    LuaWrapper();
};
template <typename T> const char* LuaWrapper<T>::classname;
template <typename T> T* (*LuaWrapper<T>::allocator)();
template <typename T> void (*LuaWrapper<T>::deallocator)(T*);

// [-0, +0, -]
//
// Analogous to lua_is(boolean|string|*)
//
// Returns 1 if the value at the given acceptable index is of type T (or if
// strict is false, convertable to type T) and 0 otherwise.
template <typename T>
bool luaW_is(lua_State *L, int index, bool strict = false)
{
    LUAW_TRACE();
    bool equal = false;
    if (lua_touserdata(L, index) && lua_getmetatable(L, index))
    {
        // ... ud ... udmt
        luaL_getmetatable(L, LuaWrapper<T>::classname); // ... ud ... udmt Tmt
        equal = lua_rawequal(L, -1, -2);
        if (!equal && !strict)
        {
            lua_getfield(L, -2, LUAW_EXTENDS_KEY); // ... ud ... udmt Tmt udmt.__extends
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
            {
                // ... ud ... udmt Tmt udmt.__extends k v
                equal = lua_rawequal(L, -1, -4);
                if (equal)
                {
                    lua_pop(L, 2); // ... ud ... udmt Tmt udmt.__extends
                    break;
                }
            }
            lua_pop(L, 1); // ... ud ... udmt Tmt
        }
        lua_pop(L, 2); // ... ud ...
    }
    return equal;
}

// [-0, +0, -]
//
// Analogous to lua_to(boolean|string|*)
//
// Converts the given acceptable index to a T*. That value must be of type T;
// otherwise, returns NULL.
template <typename T>
T* luaW_to(lua_State* L, int index)
{
    LUAW_TRACE();
    T* obj = NULL;
    if (luaW_is<T>(L, index))
    {
        obj = *(T**)lua_touserdata(L, index);
    }
    return obj;
}

// [-0, +0, -]
//
// Analogous to luaL_check(boolean|string|*)
//
// Checks whether the function argument at index is a T and returns this object
template <typename T>
T* luaW_check(lua_State* L, int index)
{
    LUAW_TRACE();
    T* obj = NULL;
    if (luaW_is<T>(L, index))
    {
        obj = *(T**)lua_touserdata(L, index);
    }
    else
    {
        luaL_typerror(L, index, LuaWrapper<T>::classname);
    }
    return obj;
}

// [-0, +1, -]
//
// Analogous to lua_push(boolean|string|*)
//
// Pushes a userdata of type T onto the stack. If this object already exists in
// the Lua environment, it will assign the existing store to it. Otherwise, a
// new store will be created for it.
template <typename T>
void luaW_push(lua_State* L, T* obj)
{
    LUAW_TRACE();
    T** ud = (T**)lua_newuserdata(L, sizeof(T*)); // ... obj
    *ud = obj;
    luaL_getmetatable(L, LuaWrapper<T>::classname); // ... obj mt
    lua_setmetatable(L, -2); // ... obj
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // ... obj LuaWrapper
    lua_getfield(L, -1, LUAW_COUNT_KEY); // ... obj LuaWrapper LuaWrapper.counts
    lua_pushlightuserdata(L, obj); // ... obj LuaWrapper LuaWrapper.counts lud
    lua_gettable(L, -2); // ... obj LuaWrapper LuaWrapper.counts count
    int count = lua_tointeger(L, -1);
    lua_pushlightuserdata(L, obj); // ... obj LuaWrapper LuaWrapper.counts count lud
    lua_pushinteger(L, count+1); // ... obj LuaWrapper LuaWrapper.counts count lud count+1
    lua_settable(L, -4); // ... obj LuaWrapper LuaWrapper.counts count
    lua_pop(L, 3); // ... obj
}

// Instructs LuaWrapper that it owns the userdata, and can manage its memory.
// When all references to the object are removed, Lua is free to garbage
// collect it and delete the object.
//
// Returns true if luaW_hold took hold of the object, and false if it was
// already held
template <typename T>
bool luaW_hold(lua_State* L, T* obj)
{
    LUAW_TRACE();
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // ... LuaWrapper

    lua_getfield(L, -1, LUAW_HOLDS_KEY); // ... LuaWrapper LuaWrapper.holds
    lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.holds lud
    lua_rawget(L, -2); // ... LuaWrapper LuaWrapper.holds hold
    bool held = lua_toboolean(L, -1);
    // If it's not held, hold it
    if (!held)
    {
        // Apply hold boolean
        lua_pop(L, 1); // ... LuaWrapper LuaWrapper.holds
        lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.holds lud
        lua_pushboolean(L, true); // ... LuaWrapper LuaWrapper.holds lud true
        lua_rawset(L, -3); // ... LuaWrapper LuaWrapper.holds

        // Check count, if there's at least one, add a storage table
        lua_pop(L, 1); // ... LuaWrapper
        lua_getfield(L, -1, LUAW_COUNT_KEY); // ... LuaWrapper LuaWrapper.counts
        lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.counts lud
        lua_rawget(L, -2); // ... LuaWrapper LuaWrapper.counts count
        if (lua_tointeger(L, -1) > 0)
        {
            // Add the storage table if there isn't one already
            lua_pop(L, 2);
            lua_getfield(L, -1, LUAW_STORAGE_KEY); // ... LuaWrapper LuaWrapper.storage
            lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.storage lud
            lua_rawget(L, -2); // ... LuaWrapper LuaWrapper.storage store
            if (lua_isnoneornil(L, -1))
            {
                lua_pop(L, 1); // ... LuaWrapper LuaWrapper.storage
                lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.storage lud
                lua_newtable(L); // ... LuaWrapper LuaWrapper.storage lud store
                lua_rawset(L, -3); // ... LuaWrapper LuaWrapper.storage
                lua_pop(L, 2); // ...
            }
        }
        return true;
    }
    lua_pop(L, 3); // ...
    return false;
}

// Releases LuaWrapper's hold on an object. This allows the user to remove
// all references to an object in Lua and ensure that Lua will not attempt to
// garbage collect it.
template <typename T>
void luaW_release(lua_State* L, T* obj)
{
    LUAW_TRACE();
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // ... LuaWrapper
    lua_getfield(L, -1, LUAW_HOLDS_KEY); // ... LuaWrapper LuaWrapper.holds
    lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.holds lud
    lua_pushnil(L); // ... LuaWrapper LuaWrapper.counts lud nil
    lua_settable(L, -3); // ... LuaWrapper LuaWrapper.counts count
    lua_pop(L, 1); // ... LuaWrapper
}

// When luaW_clean is called on an object, values stored on it's Lua store
// become no longer accessible.
template <typename T>
void luaW_clean(lua_State* L, T* obj)
{
    LUAW_TRACE();
    lua_getfield(L, -1, LUAW_STORAGE_KEY); // ... LuaWrapper LuaWrapper.storage
    lua_pushlightuserdata(L, obj); // ... LuaWrapper LuaWrapper.storage lud
    lua_pushnil(L); // ... LuaWrapper LuaWrapper.storage lud nil
    lua_settable(L, -3);  // ... LuaWrapper LuaWrapper.store
    lua_pop(L, 2); // ...
}

// This function is called from Lua, not C++
//
// Calls the lua defined constructor ("__ctor") on a userdata. Assumes the
// userdata is on top of the stack, and numargs arguments are below it. This
// runs the CTOR_KEY function on T's metatable, using the object as the first
// argument and whatever else is below it as the rest of the arguments
template <typename T>
void luaW_constructor(lua_State* L, int numargs)
{
    LUAW_TRACE();
    // ... ud
    lua_getfield(L, -1, LUAW_CTOR_KEY); // ... ud ud.__ctor
    if (lua_type(L, -1) == LUA_TFUNCTION)
    {
        lua_pushvalue(L, -2); // ... ud ud.__ctor ud
        lua_insert(L, 1); // ud ... ud ud.__ctor
        lua_insert(L, 2); // ud ud.__ctor ... ud
        lua_insert(L, 3); // ud ud.__ctor ud  ...
        lua_call(L, numargs+1, 0); // ud
    }
    else
    {
        lua_pop(L, 1); // ... ud
    }
}

template <typename T>
void luaW_destructor(lua_State* L, T* obj)
{
    LUAW_TRACE();
    luaW_push<T>(L, obj); // ... obj
    lua_getfield(L, -1, LUAW_DTOR_KEY); // ... obj obj.__dtor
    if (lua_type(L, -1) == LUA_TFUNCTION)
    {
        lua_pushvalue(L, -2); // ... obj obj.__ctor obj
        lua_call(L, 1, 0); // ... obj
        lua_pop(L, 1); // ...
    }
    else
    {
        lua_pop(L, 2); // ...
    }
}

// This function is generally called from Lua, not C++
//
// Creates an object of type T and calls the constructor on it with the values
// on the stack as arguments to it's constructor
template <typename T>
int luaW_new(lua_State* L)
{
    LUAW_TRACE();
    int numargs = lua_gettop(L);
    T* obj = LuaWrapper<T>::allocator();
    luaW_push<T>(L, obj);
    luaW_hold<T>(L, obj);
    luaW_constructor<T>(L, numargs);
    return 1;
}

#ifdef LUAW_BUILDER

// This function is called from Lua, not C++
//
// This is an alternative way to construct objects. Instead of using new and a
// constructor, you can use a builder instead. A builder is called like this:
//
// f = Foo.build
// {
//     X = 10;
//     Y = 20;
// }
//
// This will then create a new Foo object, and then call f:X(10) and f:Y(20) on
// that object. The constructor is not called at any point. The keys in this
// table are used as function names on the metatable.
//
// This is sort of experimental, just to see if it ends up being useful.
template <typename T>
void luaW_builder(lua_State* L)
{
    LUAW_TRACE();
    if (lua_type(L, 1) == LUA_TTABLE)
    {
        // {} ud
        for (lua_pushnil(L); lua_next(L, 1); lua_pop(L, 1))
        {
            // {} ud k v
            lua_pushvalue(L, -2); // {} ud k v k
            lua_gettable(L, -4); // {} ud k v ud[k]
            lua_pushvalue(L, -4); // {} ud k v ud[k] ud
            lua_pushvalue(L, -3); // {} ud k v ud[k] ud v
            lua_call(L, 2, 0); // {} ud k v
        }
        // {} ud
    }
}

// This function is generally called from Lua, not C++
//
// Creates an object of type T and initializes it using its builder to
// initialize it
template <typename T>
int luaW_build(lua_State* L)
{
    LUAW_TRACE();
    T* obj = LuaWrapper<T>::allocator();
    luaW_push<T>(L, obj);
    luaW_hold<T>(L, obj);
    luaW_builder<T>(L);
    return 1;
}

#endif

// This function is called from Lua, not C++
//
// The default metamethod to call when indexing into lua userdata representing
// an object of type T. This will fisrt check the userdata's environment table
// and if it's not found there it will check the metatable. This is done so
// individual userdata can be treated as a table, and can hold thier own
// values.
template <typename T>
int luaW__index(lua_State* L)
{
    LUAW_TRACE();
    // obj key
    T* obj = luaW_to<T>(L, 1);
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // obj key LuaWrapper
    lua_getfield(L, -1, LUAW_STORAGE_KEY); // obj key LuaWrapper LuaWrapper.storage
    lua_pushlightuserdata(L, obj); // obj key LuaWrapper LuaWrapper.table lud
    lua_rawget(L, -2); // obj key LuaWrapper LuaWrapper.table table
    if (!lua_isnoneornil(L, -1))
    {
        lua_pushvalue(L, -4); // obj key LuaWrapper LuaWrapper.table table key
        lua_rawget(L, -2); // obj key LuaWrapper LuaWrapper.table table table[k]
        if (lua_isnoneornil(L, -1))
        {
            lua_pop(L, 4); // obj key
            lua_getmetatable(L, -2); // obj key mt
            lua_pushvalue(L, -2); // obj key mt k
            lua_rawget(L, -2); // obj key mt mt[k]
        }
    }
    else
    {
        lua_pop(L, 3); // obj key
        lua_getmetatable(L, -2); // obj key mt
        lua_pushvalue(L, -2); // obj key mt k
        lua_rawget(L, -2); // obj key mt mt[k]
    }
    return 1;
}

// This function is called from Lua, not C++
//
// The default metamethod to call when createing a new index on lua userdata
// representing an object of type T. This will index into the the userdata's
// environment table that it keeps for personal storage. This is done so
// individual userdata can be treated as a table, and can hold thier own
// values.
template <typename T>
int luaW__newindex(lua_State* L)
{
    LUAW_TRACE();
    // obj key value
    T* obj = luaW_to<T>(L, 1);
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // obj key value LuaWrapper
    lua_getfield(L, -1, LUAW_STORAGE_KEY); // obj key value LuaWrapper LuaWrapper.storage
    lua_pushlightuserdata(L, obj); // obj key value LuaWrapper LuaWrapper.storage lud
    lua_rawget(L, -2); // obj key value LuaWrapper LuaWrapper.storage store
    if (!lua_isnoneornil(L, -1))
    {
        lua_pushvalue(L, -5); // obj key value LuaWrapper LuaWrapper.storage store key
        lua_pushvalue(L, -5); // obj key value LuaWrapper LuaWrapper.storage store key value
        lua_rawset(L, -3); // obj key value LuaWrapper LuaWrapper.storage store
    }
    return 0;
}

// This function is called from Lua, not C++
//
// The __gc metamethod handles cleaning up userdata. The userdata's reference
// count is decremented, and if this is the final reference to the userdata,
// the __dtor metamethod is called, its environment table is nil'd and pointer
// deleted.
template <typename T>
int luaW__gc(lua_State* L)
{
    LUAW_TRACE();
    // obj
    T* obj = luaW_to<T>(L, 1);
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // obj LuaWrapper
    lua_getfield(L, -1, LUAW_COUNT_KEY); // obj LuaWrapper LuaWrapper.counts
    lua_pushlightuserdata(L, obj); // obj LuaWrapper LuaWrapper.counts lud
    lua_gettable(L, -2); // obj LuaWrapper LuaWrapper.counts count
    int count = lua_tointeger(L, -1);
    lua_pushlightuserdata(L, obj); // obj LuaWrapper LuaWrapper.counts count lud
    lua_pushinteger(L, count-1); // obj LuaWrapper LuaWrapper.counts count lud count-1
    lua_settable(L, -4); // obj LuaWrapper LuaWrapper.counts count
    lua_pop(L, 3); // obj LuaWrapper

    if (obj && 1 == count)
    {
        luaW_destructor<T>(L, obj);
        luaW_release<T>(L, obj);
        luaW_clean<T>(L, obj);
        delete obj;
    }
    return 0;
}

// Run this to create a table and metatable for your class. You must have a
// correctly initialized LuaWrapper<T> for your class in order for this to
// properly initilize your class wrapper. This function will also take care of
// extending any classes T inherits from by copying the values in the metatable
// of the extended class to T's metatable (assuming T's metatable doesn't have
// something in that key already).
template <typename T>
void luaW_register(lua_State* L, const char* classname, const luaL_reg* table, const luaL_reg* metatable, const char** extends = NULL, bool disablenew = false, T* (*allocator)() = luaW_defaultallocator<T>, void (*deallocator)(T*) = luaW_defaultdeallocator<T>)
{
    LUAW_TRACE();
    LuaWrapper<T>::classname = classname;
    LuaWrapper<T>::allocator = allocator;
    LuaWrapper<T>::deallocator = deallocator;
    const luaL_reg defaulttable[] =
    {
        { "new", luaW_new<T> },
#ifdef LUAW_BUILDER
        { "build", luaW_build<T> },
#endif
        { NULL, NULL }
    };
    const luaL_reg defaultmetatable[] = { { "__index", luaW__index<T> }, { "__newindex", luaW__newindex<T> }, { "__gc", luaW__gc<T> }, { NULL, NULL } };
    const luaL_reg emptytable[] = { { NULL, NULL } };

    table = table ? table : emptytable;
    metatable = metatable ? metatable : emptytable;

    // Ensure that the LuaWrapper table is set up
    luaW_getregistry(L, LUAW_WRAPPER_KEY); // LuaWrapper
    if (lua_isnil(L, -1))
    {
        lua_newtable(L); // nil {}
        luaW_setregistry(L, LUAW_WRAPPER_KEY); // nil
        luaW_getregistry(L, LUAW_WRAPPER_KEY); // nil LuaWrapper
        lua_newtable(L); // nil LuaWrapper {}
        lua_setfield(L, -2, LUAW_COUNT_KEY); // nil LuaWrapper
        lua_newtable(L); // nil LuaWrapper {}
        lua_setfield(L, -2, LUAW_STORAGE_KEY); // nil LuaWrapper
        lua_newtable(L); // nil LuaWrapper {}
        lua_setfield(L, -2, LUAW_HOLDS_KEY); // nil LuaWrapper
        lua_pop(L, 1); // nil
    }
    lua_pop(L, 1); //

    // Open table
    if (!disablenew)
    {
        luaL_register(L, LuaWrapper<T>::classname, defaulttable); // T
        luaL_register(L, NULL, table); // T
    }
    else
    {
        luaL_register(L, LuaWrapper<T>::classname, table); // T
    }

    // Open metatable, set up extends table
    luaL_newmetatable(L, LuaWrapper<T>::classname); // T mt
    lua_newtable(L); // T mt {}
    lua_setfield(L, -2, LUAW_EXTENDS_KEY); // T mt
    luaL_register(L, NULL, defaultmetatable); // T mt
    luaL_register(L, NULL, metatable); // T mt

    // Copy key/value pairs from extended metatables
    for (const char** e = extends; e && *e; ++e)
    {
        luaL_getmetatable(L, *e); // T mt emt
        if(lua_isnoneornil(L, -1))
        {
            lua_pop(L, 1); // T mt
            std::cout << "Error: did not open table " << *e << " before " << LuaWrapper<T>::classname << std::endl;
            continue;
        }
        lua_getfield(L, -2, LUAW_EXTENDS_KEY); // T mt emt mt.__extends
        lua_pushvalue(L, -2); // T mt emt mt.__extends emt
        lua_setfield(L, -2, *e); // T mt emt mt.__extends
        lua_getfield(L, -2, LUAW_EXTENDS_KEY); // T mt emt mt.__extends emt.__extends

        for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
        {
            // T mt emt mt.__extends emt.__extends k v
            lua_pushvalue(L, -2); // T mt emt mt.__extends emt.__extends k v k
            lua_pushvalue(L, -2); // T mt emt mt.__extends emt.__extends k v k
            lua_rawset(L, -6); // T mt emt mt.__extends emt.__extends k v
        }

        lua_pop(L, 2); // T mt emt

        for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
        {
            // T mt emt k v
            lua_pushvalue(L, -2); // T mt emt k v k
            lua_gettable(L, -5); // T mt emt k v mt[k]
            if(lua_isnoneornil(L, -1))
            {
                lua_pop(L, 1); // T mt emt k v
                lua_pushvalue(L, -2); // T mt emt k v k
                lua_pushvalue(L, -2); // T mt emt k v k v
                lua_rawset(L, -6); // T mt emt k v
            }
            else
            {
                lua_pop(L, 1); // T mt k v
            }
        }
        lua_pop(L, 1); // T mt
    }
    lua_setmetatable(L, -2); // T
    lua_pop(L, 1); //
}

// Same as above, except sometimes its nice to be able to only have to set the
// fields you care about using a struct.
template<typename T>
void luaW_register(lua_State* L, const char* classname, LuaWrapperOptions<T>& options)
{
    luaW_register(L, classname, options.table, options.metatable, options.extends, options.disablenew, options.allocator, options.deallocator);
}

#undef luaW_getregistry
#undef luaW_setregistry

/*
 * Copyright (c) 2010 Alexander Ames
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#endif // LUA_WRAPPER_H_