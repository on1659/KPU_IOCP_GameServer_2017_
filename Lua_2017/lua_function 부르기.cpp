#include <stdio.h>
#include <string>

#pragma comment(lib, "lua//lua53.lib")

extern "C"
{
#include "lua//lua.h"
#include "lua//lauxlib.h"
#include "lua//lualib.h"
}

int main()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	int error = luaL_loadfile(L, "smaple.lua");
	lua_pcall(L, 0, 0, 0);

	lua_getglobal(L, "plustwo");
	lua_pushnumber(L, 5);
	lua_pcall(L, 1, 1, 0);
	int result = static_cast<int>(lua_tonumber(L, -1));

	printf("result %d\n",result);


	lua_pop(L, 1);
	lua_close(L);
}