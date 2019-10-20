/* 
* eventloop.h
* wangkaichao2@163.com 2018-09-23
*/

#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H

#include <stdint.h>

#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
#include <sys/epoll.h>
typedef int (* event_loop_fd_func_t)(int fd, void* data, struct epoll_event* ep);
typedef int (* event_loop_timer_func_t)(void* data);
typedef int (* event_loop_signal_func_t)(int signal_number, void* data);
typedef void (* event_loop_idle_func_t)(void* data);

typedef struct list_head list_t;

typedef struct event_loop_st {
	int epoll_fd;
	list_t check_list;
	list_t idle_list;
} event_loop_t;

/*
* @brief Statement before define
*/
struct event_source_interface_st;
typedef struct event_source_interface_st event_source_interface_t;

typedef struct event_source_st {
	event_source_interface_t* interface;
	event_loop_t* loop;
	list_t link;
	void* data;
} event_source_t;

struct event_source_interface_st {
	int (* dispatch)(event_source_t* source, struct epoll_event* ep);
	int (* remove)(event_source_t* source);
};

typedef struct event_source_fd_st {
	event_source_t base;
	int fd;
	event_loop_fd_func_t func;
} event_source_fd_t;

typedef struct event_source_timer_st {
	event_source_t base;
	int fd;
	event_loop_timer_func_t func;
} event_source_timer_t;

typedef struct event_source_signal_st {
	event_source_t base;
	int fd;
	int signal_number;
	event_loop_signal_func_t func;
} event_source_signal_t;

typedef struct event_source_idle_st {
	event_source_t base;
	event_loop_idle_func_t func;
} event_source_idle_t;

event_loop_t* event_loop_create(void);
void event_loop_destroy(event_loop_t* loop);
int event_loop_dispatch(event_loop_t* loop, int timeout);
int event_loop_get_fd(event_loop_t* loop);

event_source_t* event_loop_add_fd(event_loop_t* loop, int fd, event_loop_fd_func_t func, int kind, void* data); //kind 0 lt,1 et
int event_source_fd_update(event_source_t* source, int kind);

event_source_t* event_loop_add_timer(event_loop_t* loop, event_loop_timer_func_t func, int kind, void *data);
int event_source_timer_update(event_source_t* source, int ms_delay, int ms_interval);

event_source_t* event_loop_add_signal(event_loop_t* loop, int signal_number, event_loop_signal_func_t func, int kind, void* data);
event_source_t* event_loop_add_idle(event_loop_t* loop, event_loop_idle_func_t func, void *data);

int event_source_remove(event_source_t* source);
void event_source_check(event_source_t* source);
#endif

#ifdef  __cplusplus
}
#endif

#endif
