#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "wm_log.h"

#ifdef TAG
#undef TAG
#define TAG "demolog"
#endif

static pthread_t th_handle[20]; 

static void *thread_fun(void *param)
{
    long id = (long)param;
    pthread_detach(pthread_self());
    
    while (1) {
        LOGD("~~~~~~~~~~~~~~id:%ld\n", id);
        usleep(100*1000);
    }

    return NULL;
}

int main()
{
    LOG_OPEN("demo");

    for (long i = 0; i < 20; i++) {
        pthread_create(&th_handle[i], NULL, thread_fun, (void *)i);
    }

    while (1)
        sleep(100);

    return 0;
}
