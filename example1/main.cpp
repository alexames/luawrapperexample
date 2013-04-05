#include <iostream>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "LuaBankAccount.hpp"

using namespace std;

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        luaopen_BankAccount(L);
        if (luaL_dofile(L, argv[1]))
            cout << lua_tostring(L, -1) << endl;
        lua_close(L);
    }
    return 0;
}
