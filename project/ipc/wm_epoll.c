#include <pthread.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "wm_epoll.h"
#include "list.h"
#include "wm_log.h"

typedef struct wm_epoll {
    int     fd; 
    struct list_head head;
} wm_epoll_t;

typedef struct wm_event {
    int         fd;
    wm_pfn_t    pfn_event;
    void *      parg1;
    wm_pfn_t    pfn_exit;
    void *      parg2;
    struct list_head node;
} wm_event_t;

int wm_epoll_create(wm_handle_t *pHandle, int size)
{
    wm_epoll_t *p = NULL;

    CHK_ARG_RE(!pHandle, -1);
	p = (wm_epoll_t *)calloc(1, sizeof(wm_epoll_t));
    CHK_ARG_RE(!p, -1);
   	p->fd = epoll_create(size);
    CHK_ARG_GT(p->fd < 0, ERR0);
    INIT_LIST_HEAD(&p->head);
    *pHandle = (wm_handle_t)p;
	return 0;
ERR0:
    free(p);
    return -1;
}

int wm_epoll_destroy(wm_handle_t handle)
{
    int ret;
    wm_epoll_t *p = (wm_epoll_t *)handle;
    wm_event_t *pos, *n;

    CHK_ARG_RE(!p, -1);
    list_for_each_entry_safe(pos, n, &p->head, node)
    {
        list_del(&pos->node);
        CHK_FUN(epoll_ctl(p->fd, EPOLL_CTL_DEL, pos->fd, NULL), ret);   
        if (pos->pfn_exit)
        {
            pos->pfn_exit(pos->fd, pos->parg2);
        }
        else
        {
            CHK_FUN(close(pos->fd), ret);
        }
        free(pos);
    }

	CHK_FUN(close(p->fd), ret);
    free(p);
    return 0;
}

int wm_epoll_add(wm_handle_t handle, int fd, uint32_t events, 
        wm_pfn_t pevent, void *pArg1, 
        wm_pfn_t pexit, void *pArg2)
{
	struct epoll_event ep;
    wm_event_t *pe = NULL;
    wm_epoll_t *p = (wm_epoll_t *)handle;

    CHK_ARG_RE(!p || !pevent, -1);
	pe = (wm_event_t *)calloc(1, sizeof(wm_event_t));
    CHK_ARG_RE(!pe, -1);
    pe->fd = fd;
    pe->pfn_event = pevent;
    pe->parg1 = pArg1;
    pe->pfn_exit = pexit;
    pe->parg2 = pArg2;

	//EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
	//EPOLLOUT：表示对应的文件描述符可以写；
	//EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
    ep.events = events;
	ep.data.ptr = (void *)pe;
	//EPOLL_CTL_ADD：注册新的fd到epfd中；
	//EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
	//EPOLL_CTL_DEL：从epfd中删除一个fd；
	CHK_FUN_GT(epoll_ctl(p->fd, EPOLL_CTL_ADD, pe->fd, &ep), ERR0);
    list_add(&pe->node, &p->head);
    return 0;

ERR0:
    free(pe);
    return -1;
}

int wm_epoll_modify(wm_handle_t handle, int fd, uint32_t events)
{
    int ret;
    wm_epoll_t *p = (wm_epoll_t *)handle;
    wm_event_t *pos;

    CHK_ARG_RE(!p, -1);
    list_for_each_entry(pos, &p->head, node)
    {
        if (pos->fd == fd)
        {
	        struct epoll_event ep;

            ep.events = events;
            ep.data.ptr = (void *)pos;
            CHK_FUN(epoll_ctl(p->fd, EPOLL_CTL_MOD, pos->fd, &ep), ret);   
            return 0;
        }
    }

   return -1;
}

int wm_epoll_remove(wm_handle_t handle, int fd)
{
    int ret;
    wm_epoll_t *p = (wm_epoll_t *)handle;
    wm_event_t *pos, *n;

    CHK_ARG_RE(!p, -1);
    list_for_each_entry_safe(pos, n, &p->head, node)
    {
        if (pos->fd == fd)
        {
            list_del(&pos->node);
            CHK_FUN(epoll_ctl(p->fd, EPOLL_CTL_DEL, pos->fd, NULL), ret);   
            if (pos->pfn_exit)
            {
                pos->pfn_exit(pos->fd, pos->parg2);
            }
            else
            {
                CHK_FUN(close(pos->fd), ret);
            }
            free(pos);
            return 0;
        }
    }

   return -1;
}

int wm_epoll_wait(wm_handle_t handle, unsigned long ulMilsecond)
{
	struct epoll_event ep[32];
	int i, count;
    wm_epoll_t *p = (wm_epoll_t *)handle;
    wm_event_t *pe = NULL;

    CHK_ARG_RE(!p, -1);
	count = epoll_wait(p->fd, ep, sizeof(ep)/sizeof(ep[0]), ulMilsecond);
	if (count == 0 || count == -1)
		return -1;
	
	for (i = 0; i < count; i++) {
		pe = (wm_event_t *)ep[i].data.ptr;
        pe->pfn_event(pe->fd, pe->parg1);
	}

	return 0;
}

int wm_epoll_timer_open(int ms_delay, int ms_interval)
{
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);

    CHK_ARG_RE(fd == -1, -1);
    CHK_FUN_RE(wm_epoll_timer_set(fd, ms_delay, ms_interval), -1);
    return fd;
}

int wm_epoll_timer_set(int fd, int ms_delay, int ms_interval)
{
	struct itimerspec its;

	//after timeout start interval
	its.it_interval.tv_sec = ms_interval / 1000;
	its.it_interval.tv_nsec = (ms_interval % 1000) * 1000 * 1000;
	//after ms_delay timeout
	its.it_value.tv_sec = ms_delay / 1000;
	its.it_value.tv_nsec = (ms_delay % 1000) * 1000 * 1000;
    CHK_FUN_RE(timerfd_settime(fd, 0, &its, NULL), -1);
    return 0;
}

int wm_epoll_signal_open(int sig_num)
{
    int fd;
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, sig_num);
	fd = signalfd(-1, &mask, 0);
    CHK_ARG_RE(fd == -1, -1);
	sigprocmask(SIG_BLOCK, &mask, NULL);
    return fd;
}


