#include <iostream>
#include <string>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include "luawrapper.hpp"

#include "BankAccount.hpp"

using namespace std;

//
// Allocator
//
// Types that do not have a default contructor require you to write an allocator function.
// This function is passed to luaW_register.
//

BankAccount* BankAccount_new(lua_State *L)
{
    const char* owner = luaL_checkstring(L, 1);
    float balance = luaL_checknumber(L, 2);
    return new BankAccount(owner, balance);
}

//
// Static Functions
//
// These functions can be called directly from the BankAccount table in lua
//

int BankAccount_checkTotalMoneyInBank(lua_State *L)
{
    lua_pushnumber(L, BankAccount::checkTotalMoneyInBank());
    return 1;
}

//
// Member Functions
//
// These functions are stored on the BankAccount.metatable table.
// All BankAccount objects in Lua have access to the functions defined there
// by the use of special a __index metatmethod that is set up by LuaWrapper.
//

int BankAccount_getOwnerName(lua_State *L)
{
    BankAccount* account = luaW_check<BankAccount>(L, 1);
    lua_pushstring(L, account->getOwnerName());
    return 1;
}

int BankAccount_deposit(lua_State *L)
{
    BankAccount* account = luaW_check<BankAccount>(L, 1);
    float amount = luaL_checknumber(L, 2);
    account->deposit(amount);
    return 0;
}

int BankAccount_withdraw(lua_State *L)
{
    BankAccount* account = luaW_check<BankAccount>(L, 1);
    float amount = luaL_checknumber(L, 2);
    account->withdraw(amount);
    return 0;
}

int BankAccount_checkBalance(lua_State *L)
{
    BankAccount* account = luaW_check<BankAccount>(L, 1);
    lua_pushnumber(L, account->checkBalance());
    return 1;
}

static luaL_Reg BankAccount_table[] =
{
    { "checkTotalMoneyInBank", BankAccount_checkTotalMoneyInBank },
    { NULL, NULL }
};

static luaL_Reg BankAccount_metatable[] =
{
    { "getOwnerName", BankAccount_getOwnerName },
    { "deposit", BankAccount_deposit },
    { "withdraw", BankAccount_withdraw },
    { "checkBalance", BankAccount_checkBalance },
    { NULL, NULL }
};

int luaopen_BankAccount(lua_State* L)
{
    luaW_register<BankAccount>(L,
        "BankAccount",
        BankAccount_table,
        BankAccount_metatable,
        BankAccount_new // If your class has a default constructor you can omit this argument,
                        // LuaWrapper will generate a default allocator for you.
    );
    return 1;
}

