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
	int rows, cols;
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadfile(L, "smaple.lua");
	lua_pcall(L, 0, 0, 0);

	lua_getglobal(L, "rows");
	lua_getglobal(L, "cols");

	rows = static_cast<int>(lua_tonumber(L, -2));
	cols = static_cast<int>(lua_tonumber(L, -1));

	printf("row : %d // col :  %d\n", rows, cols);

	lua_pop(L, 2);
	lua_close(L);
}