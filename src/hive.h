#ifndef hive_h
#define hive_h
#define GUI_PORT 100
#define WM_HIVE_CELL 999

//message type 
#define TYPE_MSGPACK 0
struct message_buf {
	int type;
	int cmd; //cmd type from
	int len;
	char *b;
};

LUALIB_API int luaopen_cmsgpack (lua_State *L);
int mp_pack_raw(lua_State *L);
struct msg_ud {
	char * data;	
};

#if defined(_WIN32)
#include <windows.h>
#define HIVE_API __declspec(dllexport)
HIVE_API
int send_to_cell(lua_State *L,char * name,char * msg,int size);
HIVE_API
int regist_handle(lua_State *L,char * name,int size,HWND handle);
HIVE_API
void hive_free(char * ptr);
#endif

#endif
