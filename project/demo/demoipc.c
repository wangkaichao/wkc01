#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "evt.h"
#include "wm_sem.h"

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

    printf("%s enter....\n", __func__);

    while (gs32Run)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        printf("%s~~~:%ld-%lu\n", __func__, ts.tv_sec, ts.tv_nsec);

        ts.tv_sec  -= 60; 
        ts.tv_nsec = 0;
        int ret = clock_settime(CLOCK_REALTIME, &ts);
        if (ret != 0)
        {
            perror("set time error");
            exit(EXIT_FAILURE);
        }

        usleep(1000 * 1000);
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void* thread_P(void* pArg)
{
    //pthread_detach(pthread_self());

    printf("%s enter....\n", __func__);

    while (gs32RunP)
    {
        evt_mtx_signal(hHandle);
        usleep(3000 * 1000);
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void* thread_V(void* pArg)
{
    int rc = -1;

    //pthread_detach(pthread_self());

    printf("%s enter....\n", __func__);

    while (gs32RunV)
    {
        rc = evt_mtx_wait(hHandle, 1000);
        printf("%s %d~~~~~~~ret:%d\n", __func__, __LINE__, rc);
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void sample_evt_start(void)
{
    evt_create(&hHandle, 0, 1);

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
    evt_destroy(hHandle);
}

//---------------------------------------------------------------------

static int gs32EvtW = 0, gs32EvtR1 = 0, gs32EvtR2 = 0;
static pthread_t evt_w, evt_r1, evt_r2;
static wm_handle_t evt_rw_handle;

static void *thread_evt_w(void *pArg)
{
    int cnt = 10;
    printf("%s enter....\n", __func__);

    while (gs32EvtW && cnt > 0)
    {
        evt_wr_broadcast(evt_rw_handle);
        printf("%s ~~~~~~~~~~~~~~~~~~~[%d]\n", __func__, cnt);
        cnt--;
        usleep(500*1000);
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void *thread_evt_r1(void *pArg)
{
    int rc = -1;

    printf("%s enter....\n", __func__);
    while (gs32EvtR1)
    {
        rc = evt_rd_wait(evt_rw_handle, 0);
        if (rc == 0)
        {
            printf("%s\n", __func__);
        }
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void *thread_evt_r2(void *pArg)
{
    int rc = -1;
    printf("%s enter....\n", __func__);

    while (gs32EvtR2)
    {
        rc = evt_rd_wait(evt_rw_handle, -1);
        if (rc == 0)
        {
            printf("%s\n", __func__);
        }
   }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void sample_evt_rw_start(void)
{
    if (evt_rw_handle)
        return;

    evt_create(&evt_rw_handle, 0, 1);

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
    evt_destroy(evt_rw_handle);
    evt_rw_handle = 0;
}

//---------------------------------------------------------------------

static int gs32SemP = 0, gs32SemV1 = 0, gs32SemV2 = 0;
static pthread_t sem_th_p, sem_th_v1, sem_th_v2;
static wm_handle_t sem_handle;

static void *thread_sem_p(void *pArg)
{
    int cnt = 10;
    printf("%s enter....\n", __func__);

    while (gs32SemP && cnt > 0)
    {
        //wm_sem_post(sem_handle);
        wm_sem_broadcast(sem_handle, 2);
        printf("%s ~~~~~~~~~~~~~~~~~~~[%d]\n", __func__, cnt);
        cnt--;
        usleep(1000*1000);
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void *thread_sem_v1(void *pArg)
{
    int rc = -1;
    int cnt = 0;
    printf("%s enter....\n", __func__);

    while (gs32SemV1)
    {
        rc = wm_sem_wait(sem_handle, 0);
        if (rc == 0)
        {
            wm_sem_getvalue(sem_handle, &cnt);
            printf("%s %d\n", __func__, cnt);
        }
    }

    printf("%s quit....\n", __func__);
    return NULL;
}

static void *thread_sem_v2(void *pArg)
{
    int rc = -1;
    int cnt = 0;
    printf("%s enter....\n", __func__);

    while (gs32SemV2)
    {
        rc = wm_sem_wait(sem_handle, 500);
        if (rc == 0)
        {
            wm_sem_getvalue(sem_handle, &cnt);
            printf("%s %d\n", __func__, cnt);
        }
   }

    printf("%s quit....\n", __func__);
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


static void printUsage(void)
{
    printf("Usage:\n");
    printf("    input ? :print usage\n");
    printf("    input 1 :evt start\n");
    printf("    input 2 :evt stop\n");
    printf("    input 3 :sem start\n");
    printf("    input 4 :sem stop\n");
    printf("    input 5 :evt rw start\n");
    printf("    input 6 :evt rw stop\n");
    printf("    input q :quit.\n");
}

int main(int argc, char *argv[])
{
    char as8Buff[256]; 
    int isQuit = 0;

    printUsage();

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
            case 'q':
                isQuit = 1;
                break;
            default:
                break;
        }

    } while (!isQuit);
 
    return 0;
}
