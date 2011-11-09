#include "LuaWrapper.hpp"
#include "Widget.hpp"
extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static int WidgetL_SetX(lua_State* L)
{
    Widget* w = luaW_check<Widget>(L, 1);
    if (w)
    {
        int x = luaL_checknumber(L, 2);
        w->SetX(x);
    }
    return 0;
}

static int WidgetL_GetX(lua_State* L)
{
    Widget* w = luaW_check<Widget>(L, 1);
    if (w)
    {
        lua_pushnumber(L, w->GetX());
        return 1;
    }
    return 0;
}

static int WidgetL_X(lua_State* L)
{
    Widget* w = luaW_check<Widget>(L, 1);
    if (lua_isnumber(L, 2))
    {
        int x = lua_tonumber(L, 2);
        w->SetX(x);
    }
    else
    {
        lua_pushnumber(L, w->GetX());
        return 1;
    }
    return 0;
}

static luaL_reg widgetMetatable[] =
{
    { "SetX", WidgetL_SetX },
    { "GetX", WidgetL_GetX },
    { "X", WidgetL_X },
    { NULL, NULL }
};

int luaopen_Widget(lua_State* L)
{
    luaW_register<Widget>(L, "Widget", NULL, widgetMetatable);
}
