#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#include "evt.h"
#include "wm_sem.h"
#include "wm_epoll.h"
#include "wm_log.h"

#ifdef TAG
#undef TAG
#define TAG "demoipc"
#endif

static int gs32Run = 0;
static int gs32RunP = 0;
static int gs32RunV = 0;
static pthread_t thId;
static pthread_t thIdP;
static pthread_t thIdV;
static wm_handle_t hHandle;

static void* thread_fun(void* pArg)
{
    struct timespec ts;
    struct tm tm;

    //pthread_detach(pthread_self());

    LOGD("enter....");

    while (gs32Run)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        LOGD("~~~:%ld-%lu", ts.tv_sec, ts.tv_nsec);

        ts.tv_sec  -= 60; 
        ts.tv_nsec = 0;
        int ret = clock_settime(CLOCK_REALTIME, &ts);
        if (ret != 0)
        {
            LOGD("set time error. %m");
            exit(EXIT_FAILURE);
        }

        usleep(1000 * 1000);
    }

    LOGD("quit....");
    return NULL;
}

static void* thread_P(void* pArg)
{
    //pthread_detach(pthread_self());

    LOGD("enter....");

    while (gs32RunP)
    {
        evt_mtx_signal(hHandle);
        usleep(3000 * 1000);
    }

    LOGD("quit....");
    return NULL;
}

static void* thread_V(void* pArg)
{
    int rc = -1;

    //pthread_detach(pthread_self());

    LOGD("enter....");

    while (gs32RunV)
    {
        rc = evt_mtx_wait(hHandle, 1000);
        LOGD("%d~~~~~~~ret:%d", __LINE__, rc);
    }

    LOGD("quit....");
    return NULL;
}

static void sample_evt_start(void)
{
    evt_mtx_create(&hHandle, 0, 1);

    gs32Run = 1;
    pthread_create(&thId, NULL, thread_fun, NULL);
    gs32RunP = 1;
    pthread_create(&thIdP, NULL, thread_P, NULL);
    gs32RunV = 1;
    pthread_create(&thIdV, NULL, thread_V, NULL);
}

static void sample_evt_stop(void)
{
    gs32Run = 0;
    gs32RunP = 0;
    gs32RunV = 0;
    pthread_join(thId, NULL);
    pthread_join(thIdP, NULL);
    pthread_join(thIdV, NULL);
    evt_mtx_destroy(hHandle);
}

//---------------------------------------------------------------------

static int gs32EvtW = 0, gs32EvtR1 = 0, gs32EvtR2 = 0;
static pthread_t evt_w, evt_r1, evt_r2;
static wm_handle_t evt_rw_handle;

static void *thread_evt_w(void *pArg)
{
    int cnt = 10;
    LOGD("enter....");

    while (gs32EvtW && cnt > 0)
    {
        evt_wr_broadcast(evt_rw_handle);
        LOGD("~~~~~~~~~~~~~~~~~~~[%d]", cnt);
        cnt--;
        usleep(500*1000);
    }

    LOGD("quit....");
    return NULL;
}

static void *thread_evt_r1(void *pArg)
{
    int rc = -1;
    int mis = 100;

    LOGD("enter....");
        
    if (mis == 0)
        evt_rd_reset(evt_rw_handle);

    while (gs32EvtR1)
    {
        rc = evt_rd_wait(evt_rw_handle, mis);
        if (rc == 0)
        {
            LOGD("ok");
        }
    }

    LOGD("quit....");
    return NULL;
}

static void *thread_evt_r2(void *pArg)
{
    int rc = -1;
    int mis = 0;

    LOGD("enter....");

    if (mis == 0)
        evt_rd_reset(evt_rw_handle);

    while (gs32EvtR2)
    {
        rc = evt_rd_wait(evt_rw_handle, mis);
        if (rc == 0)
        {
            LOGD("ok");
        }
    }

    LOGD("quit....");
    return NULL;
}

static void sample_evt_rw_start(void)
{
    if (evt_rw_handle)
        return;

    evt_rw_create(&evt_rw_handle, 0, 1);

    gs32EvtW = 1;
    pthread_create(&evt_w, NULL, thread_evt_w, NULL);
    gs32EvtR1 = 1;
    pthread_create(&evt_r1, NULL, thread_evt_r1, NULL);
    gs32EvtR2 = 1;
    pthread_create(&evt_r2, NULL, thread_evt_r2, NULL);
}

static void sample_evt_rw_stop(void)
{
    if (!evt_rw_handle)
        return;

    gs32EvtW = 0;
    gs32EvtR1 = 0;
    gs32EvtR2 = 0;
    pthread_join(evt_w, NULL);
    evt_w = 0;
    pthread_join(evt_r1, NULL);
    evt_r1 = 0;
    pthread_join(evt_r2, NULL);
    evt_r2 = 0;
    evt_rw_destroy(evt_rw_handle);
    evt_rw_handle = 0;
}

//---------------------------------------------------------------------

static int gs32SemP = 0, gs32SemV1 = 0, gs32SemV2 = 0;
static pthread_t sem_th_p, sem_th_v1, sem_th_v2;
static wm_handle_t sem_handle;

static void *thread_sem_p(void *pArg)
{
    int cnt = 10;
    LOGD("enter....");

    while (gs32SemP && cnt > 0)
    {
        //wm_sem_post(sem_handle);
        wm_sem_broadcast(sem_handle, 2);
        LOGD("~~~~~~~~~~~~~~~~~~~[%d]", cnt);
        cnt--;
        usleep(1000*1000);
    }

    LOGD("quit....");
    return NULL;
}

static void *thread_sem_v1(void *pArg)
{
    int rc = -1;
    int cnt = 0;
    LOGD("enter....");

    while (gs32SemV1)
    {
        rc = wm_sem_wait(sem_handle, 0);
        if (rc == 0)
        {
            wm_sem_getvalue(sem_handle, &cnt);
            LOGD("%d", cnt);
        }
    }

    LOGD("quit....");
    return NULL;
}

static void *thread_sem_v2(void *pArg)
{
    int rc = -1;
    int cnt = 0;
    LOGD("enter....");

    while (gs32SemV2)
    {
        rc = wm_sem_wait(sem_handle, 500);
        if (rc == 0)
        {
            wm_sem_getvalue(sem_handle, &cnt);
            LOGD("%d", cnt);
        }
   }

    LOGD("quit....");
    return NULL;
}

static void sample_sem_start(void)
{
    if (sem_handle)
        return;

    wm_sem_create(&sem_handle, 0, 0, 1);

    gs32SemP = 1;
    pthread_create(&sem_th_p, NULL, thread_sem_p, NULL);
    gs32SemV1 = 1;
    pthread_create(&sem_th_v1, NULL, thread_sem_v1, NULL);
    gs32SemV2 = 1;
    pthread_create(&sem_th_v2, NULL, thread_sem_v2, NULL);
}

static void sample_sem_stop(void)
{
    if (!sem_handle)
        return;

    gs32SemP = 0;
    gs32SemV1 = 0;
    gs32SemV2 = 0;
    pthread_join(sem_th_p, NULL);
    sem_th_p = 0;
    pthread_join(sem_th_v1, NULL);
    sem_th_v1 = 0;
    pthread_join(sem_th_v2, NULL);
    sem_th_v2 = 0;
    wm_sem_destroy(sem_handle);
    sem_handle = 0;
}

//------------------------------------------------------------
static int fd1 = -1;
static int fd2 = -1;

static void *timer_cb1(int fd, void *pArg)
{
	uint64_t expires;
	int len;

	len = read(fd, &expires, sizeof(expires));
    LOGD("%ld", (long)pArg);
    return NULL;
}

static void *timer_cb2(int fd, void *pArg)
{
    uint64_t expires;
	int len;

	len = read(fd, &expires, sizeof(expires));
    LOGD("%ld", (long)pArg);
    return NULL;
}

static void sample_epoll_timer_start(void)
{
    wm_handle_t handle = NULL;

    wm_epoll_handle(&handle);
    if (fd1 < 0)
        fd1 = wm_epoll_timer_open(2000, 0);
    else
        wm_epoll_timer_set(fd1, 2000, 0);
    wm_epoll_add(handle, fd1, EPOLLIN, timer_cb1, (void*)111, NULL, NULL);
}

static void sample_epoll_timer_stop(void)
{
    wm_handle_t handle = NULL;

    if (fd1 < 0)
        return;
    wm_epoll_handle(&handle);
    wm_epoll_remove(handle, fd1);
}

static void *queue_cb(int fd, void *pArg)
{
    int size;
    char ptr[50] = {0};
    size = mq_receive(fd, ptr, sizeof(ptr), NULL);
    LOGD("msg:%s, size:%d", ptr, size);
}

static void sample_epoll_queue_open(void)
{
    wm_handle_t handle;

    fd2 = wm_epoll_queue_open("/test1", O_CREAT | O_RDWR | O_NONBLOCK, 0, 100, 50);
    if (fd2 < 0)
        return;
    wm_epoll_handle(&handle);
    wm_epoll_add(handle, fd2, EPOLLIN, queue_cb, NULL, NULL, NULL);
    LOGD("queue open");
}

static void sample_epoll_queue_close(void)
{
    wm_handle_t handle = NULL;

    if (fd2 < 0)
        return;
    wm_epoll_handle(&handle);
    wm_epoll_remove(handle, fd2);
    LOGD("queue close");
}

static void sample_epoll_queue_send(void)
{
    static int cnt = 0;
    static char ptr[50];

    if (fd2 < 0)
       return;
    snprintf(ptr, sizeof(ptr), "I'm msg[%d]", cnt++);
    mq_send(fd2, ptr, strlen(ptr), 0);
}

static void printUsage(void)
{
    LOGD("Usage:");
    LOGD("    input ? :print usage");
    LOGD("    input 1 :evt start");
    LOGD("    input 2 :evt stop");
    LOGD("    input 3 :sem start");
    LOGD("    input 4 :sem stop");
    LOGD("    input 5 :evt rw start");
    LOGD("    input 6 :evt rw stop");
    LOGD("    input 7 :epoll timer start");
    LOGD("    input 8 :epoll timer stop");
    LOGD("    input 9 :msg open");
    LOGD("    input a :msg close");
    LOGD("    input b :msg send");
    LOGD("    input q :quit.");
}

int main(int argc, char *argv[])
{
    char as8Buff[256]; 
    int isQuit = 0;
    int fd;
    
    LOG_OPEN("demo");
    printUsage();

    wm_epoll_start();
    do
    {
        fgets(as8Buff, sizeof(as8Buff), stdin);

        switch (as8Buff[0])
        {
            case '?':
                printUsage();
                break;
            case '1':
                sample_evt_start();
                break;
            case '2':
                sample_evt_stop();
                break;
            case '3':
                sample_sem_start();
                break;
            case '4':
                sample_sem_stop();
                break;
            case '5':
                sample_evt_rw_start();
                break;
            case '6':
                sample_evt_rw_stop();
                break;
            case '7':
                sample_epoll_timer_start();
                break;
            case '8':
                sample_epoll_timer_stop();
                break;
            case '9':
                sample_epoll_queue_open();
                break;
            case 'a':
                sample_epoll_queue_close();
                break;
            case 'b':
                sample_epoll_queue_send();
                break;
            case 'q':
                isQuit = 1;
                break;
            default:
                break;
        }

    } while (!isQuit);
 
    wm_epoll_stop();
    return 0;
}
