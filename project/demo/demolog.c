#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "clog.h"

static pthread_t th_handle[20]; 

static void *thread_fun(void *param)
{
    long id = (long)param;
    pthread_detach(pthread_self());
    
    while (1) {
        log_dbg("~~~~~~~~~~~~~~id:%u\n", id);
        usleep(100*1000);
    }

    return NULL;
}

int main()
{
    for (long i = 0; i < 20; i++) {
        pthread_create(&th_handle[i], NULL, thread_fun, (void *)i);
    }

    while (1)
        sleep(100);

    return 0;
}
