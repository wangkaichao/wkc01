#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "evt.h"

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
        evt_signal(hHandle);
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
        rc = evt_wait(hHandle, 1000);
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

static void printUsage(void)
{
    printf("Usage:\n");
    printf("    input ? :print usage\n");
    printf("    input 1 :evt start\n");
    printf("    input 2 :evt stop\n");
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
                break;
            case '4':
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
