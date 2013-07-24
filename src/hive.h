#ifndef hive_h
#define hive_h
LUALIB_API int luaopen_cmsgpack (lua_State *L);


#if defined(_WIN32)
#include <windows.h>
#define HIVE_API __declspec(dllexport)
HIVE_API
int send_to_cell(lua_State *L,char * name,char * msg);
HIVE_API
int regist_handle(lua_State *L,char * name,int size,HWND handle);

#endif

#endif
