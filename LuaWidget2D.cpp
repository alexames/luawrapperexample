#include "LuaWidget2D.hpp"
#include "LuaWrapper.hpp"
#include "Widget2D.hpp"
extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static int Widget2DL_SetY(lua_State* L)
{
    Widget2D* w = luaW_check<Widget2D>(L, 1);
    if (w)
    {
        int y = luaL_checknumber(L, 2);
        w->SetY(y);
    }
    return 0;
}

static int Widget2DL_GetY(lua_State* L)
{
    Widget2D* w = luaW_check<Widget2D>(L, 1);
    if (w)
    {
        lua_pushnumber(L, w->GetY());
        return 1;
    }
    return 0;
}

static int Widget2DL_Y(lua_State* L)
{
    Widget2D* w = luaW_check<Widget2D>(L, 1);
    if (lua_isnumber(L, 2))
    {
        int y = lua_tonumber(L, 2);
        w->SetY(y);
    }
    else
    {
        lua_pushnumber(L, w->GetY());
        return 1;
    }
    return 0;
}

static luaL_reg widgetMetatable[] =
{
    { "SetY", Widget2DL_SetY },
    { "GetY", Widget2DL_GetY },
    { "Y", Widget2DL_Y },
    { NULL, NULL }
};

int luaopen_Widget2D(lua_State* L)
{
    luaW_register<Widget2D>(L, "Widget2D", NULL, widgetMetatable);
    luaW_extend<Widget2D, Widget>(L);
}
