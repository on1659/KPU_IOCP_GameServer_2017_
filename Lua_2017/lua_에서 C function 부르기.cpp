#include <stdio.h>
#include <string>

#pragma comment(lib, "lua//lua53.lib")

extern "C"
{
#include "lua//lua.h"
#include "lua//lauxlib.h"
#include "lua//lualib.h"
}

int addnum_c(lua_State * L)
{
	int a = static_cast<int>(lua_tonumber(L, -2));
	int b = static_cast<int>(lua_tonumber(L, -1));
	int result = a + b;
	lua_pop(L, 3);
	lua_pushnumber(L, result);
	return 1;
}

int main()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadfile(L, "c_func_call.lua");
	lua_pcall(L, 0, 0, 0);

	lua_register(L, "c_addnum", addnum_c);

	lua_getglobal(L, "addnum_lua");
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	lua_pcall(L, 2, 1, 0);
	int result = static_cast<int>(lua_tonumber(L, -1));
	printf("Result of addnum %d \n", result);
	lua_pop(L, 1);
	lua_close(L);
}