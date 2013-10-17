LUALIB_MINGW=-I/usr/local/include -L/usr/local/bin -llua52 
SRC=\
src/hive.c \
src/hive_cell.c \
src/hive_seri.c \
src/hive_scheduler.c \
src/hive_env.c \
src/hive_cell_lib.c \
src/hive_system_lib.c \
src/hive_socket_lib.c \
src/stable.c \
src/lua-stable.c \
src/lpack.c \
src/lua_cmsgpack.c 

all :
	echo 'make win or make posix or make macosx'

win : hivecore.dll hivecore.lib
posix : hivecore.so
macosx: hivecore.dylib

hivecore.so : $(SRC)
	gcc -g -Wall --shared -fPIC -o $@ $^ -lpthread

hivecore.dll : $(SRC)
	gcc -g -Wall -D_GUI --shared -o $@ $^ $(LUALIB_MINGW) -L./lua52  -march=i686 -lws2_32

hivecore.dylib : $(SRC)
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^ -lpthread

clean :
	rm -rf hivecore.dll hivecore.so hivecore.dylib hivecore.dylib.dSYM hivecore.lib
hivecore.lib :
	dlltool -d hivecore.def -l hivecore.lib
