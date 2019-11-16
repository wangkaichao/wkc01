#ifndef WM_EPOLL_H
#define WM_EPOLL_H

#include <stdint.h>
#include <sys/epoll.h>
#include "wm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*wm_pfn_t)(int, void *);

int wm_epoll_create(wm_handle_t *pHandle, int size);
int wm_epoll_destroy(wm_handle_t handle);
//EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
//EPOLLOUT：表示对应的文件描述符可以写；
//EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
int wm_epoll_add(wm_handle_t handle, int fd, uint32_t events, 
        wm_pfn_t pevent, void *pArg1, 
        wm_pfn_t pexit, void *pArg2);
int wm_epoll_modify(wm_handle_t handle, int fd, uint32_t events);
int wm_epoll_remove(wm_handle_t handle, int fd);
int wm_epoll_wait(wm_handle_t handle, unsigned long ulMilsecond);
int wm_epoll_timer_open(int ms_delay, int ms_interval);
int wm_epoll_timer_set(int fd, int ms_delay, int ms_interval);
int wm_epoll_signal_open(int sig_num);

int wm_epoll_start(void);
int wm_epoll_stop(void);
int wm_epoll_handle(wm_handle_t *phandle);
#ifdef __cplusplus
}
#endif

#endif
