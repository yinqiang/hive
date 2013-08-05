#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "hive_cell.h"
#include "hive_env.h"
#include "hive_scheduler.h"
#include "hive_system_lib.h"
#include "stable.h"

#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#define DEFAULT_THREAD 4
#define MAX_GLOBAL_MQ 0x10000
#define GP(p) ((p) % MAX_GLOBAL_MQ)

struct global_queue {
	uint32_t head;
	uint32_t tail;
	int total;
	int thread;
	struct cell * queue[MAX_GLOBAL_MQ];
	bool flag[MAX_GLOBAL_MQ];
};

struct timer {
	uint32_t current;
	struct cell * sys;
	struct global_queue * mq;
};

static void 
globalmq_push(struct global_queue *q, struct cell * c) {
	uint32_t tail = GP(__sync_fetch_and_add(&q->tail,1));
	q->queue[tail] = c;
	__sync_synchronize();
	q->flag[tail] = true;
	__sync_synchronize();
}

static struct cell * 
globalmq_pop(struct global_queue *q) {
	uint32_t head =  q->head;
	uint32_t head_ptr = GP(head);
	if (head_ptr == GP(q->tail)) {
		return NULL;
	}

	if(!q->flag[head_ptr]) {
		return NULL;
	}

	struct cell * c = q->queue[head_ptr];
	if (!__sync_bool_compare_and_swap(&q->head, head, head+1)) {
		return NULL;
	}
	q->flag[head_ptr] = false;
	__sync_synchronize();

	return c;
}

static void
globalmq_init(struct global_queue *q, int thread) {
	memset(q, 0, sizeof(*q));
	q->thread = thread;
}

static inline void
globalmq_inc(struct global_queue *q) {
	__sync_add_and_fetch(&q->total,1);
}

static inline void
globalmq_dec(struct global_queue *q) {
	__sync_sub_and_fetch(&q->total,1);
}

static int
_message_dispatch(struct global_queue *q) {
	struct cell *c = globalmq_pop(q);
	if (c == NULL)
		return 1;
	int r =  cell_dispatch_message(c);
	switch(r) {
	case CELL_EMPTY:
	case CELL_MESSAGE:
		break;
	case CELL_QUIT:
		globalmq_dec(q);
		return 1;
	}
	globalmq_push(q, c);
	return r;
}

static uint32_t
_gettime(void) {
	uint32_t t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint32_t)(tv.tv_sec & 0xffffff) * 100;
	t += tv.tv_usec / 10000;
	return t;
}

static void
timer_init(struct timer *t, struct cell * sys, struct global_queue *mq) {
	t->current = _gettime();
	t->sys = sys;
	t->mq = mq;
}

static inline void
send_tick(struct cell * c) {
	cell_send(c, 0, NULL,0);
}

static void
_updatetime(struct timer * t) {
	uint32_t ct = _gettime();
	if (ct > t->current) {
		int diff = ct-t->current;
		t->current = ct;
		int i;
		for (i=0;i<diff;i++) {
			send_tick(t->sys);
		}
	}
}

static void *
_timer(void *p) {
	struct timer * t = p;
	for (;;) {
		_updatetime(t);
		usleep(2500);
		if (t->mq->total <= 1)
			return NULL;
	}
}

static void *
_worker(void *p) {
	struct global_queue * mq = p;
	for (;;) {
		int i;
		int n = mq->total;
		int ret = 1;
		for (i=0;i<n;i++) {
			ret &= _message_dispatch(mq);
			if (n < mq->total) {
				n = mq->total;
			}
		}
		if (ret) {
			usleep(1000);
			if (mq->total <= 1)
				return NULL;
		} 
	}
	return NULL;
}

static void
_start(struct global_queue *gmq, struct timer *t) {
	int thread = gmq->thread;
#if defined(_WIN32)
	HANDLE pid[thread+1];
	int i =0;
	pid[0]=CreateThread(NULL, 0, _timer, t, 0, NULL);
	

	for (i=1;i<=thread;i++) {
		pid[i]=CreateThread(NULL, 0, _worker, gmq, 0, NULL);
	}
	
	for (i=0;i<thread;i++) {
		 WaitForSingleObject(pid[i], INFINITE);
	}
#else
	pthread_t pid[thread+1];
	int i;

	pthread_create(&pid[0], NULL, _timer, t);

	for (i=1;i<=thread;i++) {
		pthread_create(&pid[i], NULL, _worker, gmq);
	}

	for (i=0;i<=thread;i++) {
		pthread_join(pid[i], NULL); 
	}
#endif
}

#if defined(_GUI) //for gui app,print to file
static void
luaV_addlstring(luaL_Buffer *b, const char *s, size_t l, int toline)
{
    while (l--)
    {
	luaL_addchar(b, *s);
	s++;
    }
}

static int
hive_print(lua_State *L)
{
    int i, n = lua_gettop(L); /* nargs */
    const char *s;
    size_t l;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    lua_getglobal(L, "tostring");
    for (i = 1; i <= n; i++)
    {
	lua_pushvalue(L, -1); /* tostring */
	lua_pushvalue(L, i); /* arg */
	lua_call(L, 1, 1);
	s = lua_tolstring(L, -1, &l);
	if (s == NULL)
	    return luaL_error(L, "cannot convert to string");
	if (i > 1) luaL_addchar(&b, ' '); /* use space instead of tab */
	luaV_addlstring(&b, s, l, 0);
	lua_pop(L, 1);
    }
    luaL_pushresult(&b);
    const char *p, *s2 = lua_tolstring(L, -1, &l);
    lua_pop(L,1);
    hive_getenv(L,"print_log");
    FILE * fp = lua_touserdata(L,-1);
    fwrite(s2,sizeof(char),l,fp);
    char line[2] = "\n";
    fwrite(line,sizeof(char),2,fp);
    fflush(fp);    
    lua_pop(L,1);
    return 0;
}
#endif 

lua_State *
scheduler_newtask(lua_State *pL) {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	hive_createenv(L);
#if defined(_GUI) //for gui app,print to file
	FILE * fp;
	fp=fopen("print.log","a+");
	lua_pushlightuserdata(L,fp);
	hive_setenv(L,"print_log");
	/* print */
    lua_pushcfunction(L, hive_print);
    lua_setglobal(L, "print");
#endif
	struct global_queue * mq = hive_copyenv(L, pL, "message_queue");
	globalmq_inc(mq);
	hive_copyenv(L, pL, "system_pointer");

	hive_copyenv(L, pL, "cell_registar");
	hive_copyenv(L, pL, "win_handle_registar");

	lua_newtable(L);
	lua_newtable(L);
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L,-2);
	hive_setenv(L, "cell_map");

	return L;
}

void
scheduler_deletetask(lua_State *L) {
	lua_close(L);
}

void
scheduler_starttask(lua_State *L) {
	hive_getenv(L, "message_queue");
	struct global_queue * gmq = lua_touserdata(L, -1);
	lua_pop(L,1);
	hive_getenv(L, "cell_pointer");
	struct cell * c= lua_touserdata(L, -1);
	lua_pop(L,1);
	globalmq_push(gmq, c);
}

int 
scheduler_start(lua_State *L) {
	luaL_checktype(L,1,LUA_TTABLE);
	const char * system_lua = luaL_checkstring(L,2);
	const char * main_lua = luaL_checkstring(L,3);
	lua_getfield(L,1, "thread");
	int thread = luaL_optinteger(L, -1, DEFAULT_THREAD);
	lua_pop(L,1);

	hive_createenv(L);
	struct global_queue * gmq = lua_newuserdata(L, sizeof(*gmq));
	globalmq_init(gmq, thread);

	lua_pushvalue(L,-1);
	hive_setenv(L, "message_queue");
	//  cell registar ,win handle registar
	//struct table * cell_registar = lua_newuserdata(L,sizeof(struct table));
	struct table * cell_registar = stable_create();
	lua_pushlightuserdata(L,cell_registar);
	hive_setenv(L,"cell_registar");
	
	struct table * handle_registar = stable_create();
	lua_pushlightuserdata(L,handle_registar);
	hive_setenv(L,"win_handle_registar");
	// end
	lua_State *sL;

	sL = scheduler_newtask(L);
	luaL_requiref(sL, "cell.system", cell_system_lib, 0);
	lua_pop(sL,1);

	lua_pushstring(sL, main_lua);
	lua_setglobal(sL, "maincell");

	struct cell * sys = cell_new(sL, system_lua);
	if (sys == NULL) {
		return 0;
	}
	scheduler_starttask(sL);

	struct timer * t = lua_newuserdata(L, sizeof(*t));
	timer_init(t,sys,gmq);

	_start(gmq,t);
	cell_close(sys);

	return 0;
}

