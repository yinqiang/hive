#include "lua.h"
#include "lauxlib.h"
#include "hive_scheduler.h"
#if defined(_WIN32)
__declspec(dllexport)
#endif
int
luaopen_hivecore(lua_State *L) {
	luaL_Reg l[] = {
		{ "start", scheduler_start },
		{ NULL, NULL },
	};
	luaL_checkversion(L);
	luaL_newlib(L,l);

	return 1;
}
