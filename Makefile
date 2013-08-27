# Your platform. See PLATS for possible values.
PLAT= none

# Convenience platforms targets.
PLATS= posix macosx cygwin

RM= rm -rf

LUALIB_CYGWIN=/usr/local/bin/lua52.dll

SRC=src/hive.c \
src/hive_cell.c \
src/hive_seri.c \
src/hive_scheduler.c \
src/hive_env.c \
src/hive_cell_lib.c \
src/hive_system_lib.c \
src/hive_socket_lib.c

none:
	@echo "Please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "   $(PLATS)"

<<<<<<< HEAD
all:	$(PLAT)

posix:	$(SRC)
	gcc -g -Wall --shared -fPIC -o hive.so $^ -lpthread

cygwin:	$(SRC)
	gcc -g -Wall --shared -o hive.dll $^ $(LUALIB_CYGWIN) -lpthread -march=i686 -lws2_32

macosx:	$(SRC)
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o hive.dylib $^ -lpthread

clean:
	$(RM) hive.dll hive.so hive.dylib hive.dylib.dSYM
=======
win : hive/core.dll
posix : hive/core.so
macosx: hive/core.dylib

hive/core.so : $(SRC)
	gcc -g -Wall --shared -fPIC -o $@ $^ -lpthread

hive/core.dll : $(SRC)
	gcc -g -Wall --shared -o $@ $^ $(LUALIB_MINGW) -lpthread -march=i686 -lws2_32

hive/core.dylib : $(SRC)
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^ -lpthread

clean :
	rm -rf hive/core.dll hive/core.so hive/core.dylib hive/core.dylib.dSYM
>>>>>>> 9d9b498265f37d5283731265ca796b8de9ded2f6
