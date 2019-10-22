#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#include "wm_sem.h"
#include "wm_log.h"

typedef struct wm_sem_t
{
    pthread_cond_t      cond;
    pthread_condattr_t  condattr;
    pthread_mutex_t     mutex;
    int                 cnt;
} wm_sem_t;

int wm_sem_create(wm_handle_t *pHandle, int s32Num, int s32IsShared, int s32IsTimeRelative)
{
    wm_sem_t *pstSem = NULL;
    
    CHK_ARG_RE(!pHandle, -1);
    pstSem = (wm_sem_t *)calloc(1, sizeof(wm_sem_t));
    CHK_ARG_RE(!pstSem, -1);
    CHK_FUN_GT(pthread_mutex_init(&pstSem->mutex, NULL), ERR_EXIT1);
    CHK_FUN_GT(pthread_mutex_lock(&pstSem->mutex), ERR_EXIT2);
    
    if (s32IsShared || s32IsTimeRelative)
    {
        CHK_FUN_GT(pthread_condattr_init(&pstSem->condattr), ERR_EXIT2); 
        if (s32IsShared)
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstSem->condattr, PTHREAD_PROCESS_SHARED), ERR_EXIT3);
        }
        else
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstSem->condattr, PTHREAD_PROCESS_PRIVATE), ERR_EXIT3);
        }

        if (s32IsTimeRelative)
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstSem->condattr, CLOCK_MONOTONIC), ERR_EXIT3);
        }
        else
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstSem->condattr, CLOCK_REALTIME), ERR_EXIT3);
        }

        CHK_FUN_GT(pthread_cond_init(&pstSem->cond, &pstSem->condattr), ERR_EXIT3);
    }

    pstSem->cnt = s32Num;
    *pHandle = (wm_handle_t)pstSem;
    CHK_FUN_GT(pthread_mutex_unlock(&pstSem->mutex), ERR_EXIT4);
    return 0;

ERR_EXIT4:
    pthread_cond_destroy(&pstSem->cond);
ERR_EXIT3:
    pthread_condattr_destroy(&pstSem->condattr);
ERR_EXIT2:
    pthread_mutex_destroy(&pstSem->mutex);
ERR_EXIT1:
    free(pstSem);
    return -1;

}

int wm_sem_destroy(wm_handle_t handle)
{
    wm_sem_t *pstSem = (wm_sem_t *)handle;

    CHK_ARG_RE(!pstSem, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
    //CHK_FUN_RE(pthread_cond_broadcast(&pstSem->cond), -1);
    CHK_FUN_RE(pthread_cond_destroy(&pstSem->cond), -1);
    CHK_FUN_RE(pthread_condattr_destroy(&pstSem->condattr), -1);
    CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
    CHK_FUN_RE(pthread_mutex_destroy(&pstSem->mutex), -1);
    free(pstSem);
    return 0;
}

int wm_sem_post(wm_handle_t handle)
{
    wm_sem_t *pstSem = (wm_sem_t *)handle;
    
    CHK_ARG_RE(!pstSem, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
    pstSem->cnt++;
    CHK_ARG_RE(pstSem->cnt < 0, -1);
    CHK_FUN_RE(pthread_cond_signal(&pstSem->cond), -1);
    CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
    return 0;
}

int wm_sem_wait(wm_handle_t handle, unsigned long ulMilsecond)
{
    wm_sem_t *pstSem = (wm_sem_t *)handle;

    CHK_ARG_RE(!pstSem, -1);

    if (ulMilsecond == 0)
    {
        CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
        if (pstSem->cnt <= 0)
        {
            CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
            return -1;
        }
    }
    else if (ulMilsecond == (unsigned long)-1)
    {
        CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
        while (pstSem->cnt <= 0)
            pthread_cond_wait(&pstSem->cond, &pstSem->mutex);
    }
    else
    {
        clockid_t clock_id;
        struct timespec tp;

        CHK_FUN_RE(pthread_condattr_getclock(&pstSem->condattr, &clock_id), -1);
        CHK_FUN_RE(clock_gettime(clock_id, &tp), -1);
        
        tp.tv_sec  += ulMilsecond / 1000;
        tp.tv_nsec += (ulMilsecond % 1000) * 1000000;
        if (tp.tv_nsec > 1000000000)
        {
            tp.tv_nsec -= 1000000000;
            tp.tv_sec  += 1;
        }

        CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
        while (pstSem->cnt <= 0)
        {
            int rc = pthread_cond_timedwait(&pstSem->cond, &pstSem->mutex, &tp);
            if (rc == ETIMEDOUT)
            {
                CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
                return -1;
            }
        }
    }

    pstSem->cnt--;
    CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
    return 0;
}

int wm_sem_getvalue(wm_handle_t handle, int *ps32Val)
{
    wm_sem_t *pstSem = (wm_sem_t *)handle;
    
    CHK_ARG_RE(!pstSem || !ps32Val, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstSem->mutex), -1);
    *ps32Val = pstSem->cnt;
    CHK_FUN_RE(pthread_mutex_unlock(&pstSem->mutex), -1);
    return 0;
}

