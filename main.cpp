#include "LuaWidget.hpp"
#include "LuaWidget2D.hpp"
extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        lua_State* L = luaL_newstate();

        luaL_openlibs(L);
        luaopen_Widget(L);
        luaopen_Widget2D(L);

        luaL_dofile(L, argv[1]);
        lua_close(L);
    }
    return 0;
}
