#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#include "evt.h"
#include "wm_log.h"

typedef struct evt_t
{
    pthread_cond_t      cond;
    pthread_condattr_t  condattr;
    pthread_mutex_t     mutex;
    int                 state;
} evt_t;

typedef struct evt_rw_t
{
    pthread_cond_t      cond;
    pthread_condattr_t  condattr;
    pthread_mutex_t     mutex;
    int                 s32Rd;
    int                 s32Wr;
} evt_rw_t;


int evt_mtx_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeRelative)
{
    evt_t *pstEvt = NULL;
    
    CHK_ARG_RE(!pHandle, -1);
    pstEvt = (evt_t *)calloc(1, sizeof(evt_t));
    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_GT(pthread_mutex_init(&pstEvt->mutex, NULL), ERR_EXIT1);
    CHK_FUN_GT(pthread_mutex_lock(&pstEvt->mutex), ERR_EXIT2);
    
    if (s32IsShared || s32IsTimeRelative)
    {
        CHK_FUN_GT(pthread_condattr_init(&pstEvt->condattr), ERR_EXIT2); 
        if (s32IsShared)
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstEvt->condattr, PTHREAD_PROCESS_SHARED), ERR_EXIT3); }
        else
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstEvt->condattr, PTHREAD_PROCESS_PRIVATE), ERR_EXIT3);
        }

        if (s32IsTimeRelative)
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstEvt->condattr, CLOCK_MONOTONIC), ERR_EXIT3);
        }
        else
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstEvt->condattr, CLOCK_REALTIME), ERR_EXIT3);
        }

        CHK_FUN_GT(pthread_cond_init(&pstEvt->cond, &pstEvt->condattr), ERR_EXIT3);
    }

    *pHandle = (wm_handle_t)pstEvt;
    CHK_FUN_GT(pthread_mutex_unlock(&pstEvt->mutex), ERR_EXIT4);
    return 0;

ERR_EXIT4:
    pthread_cond_destroy(&pstEvt->cond);
ERR_EXIT3:
    pthread_condattr_destroy(&pstEvt->condattr);
ERR_EXIT2:
    pthread_mutex_destroy(&pstEvt->mutex);
ERR_EXIT1:
    free(pstEvt);
    return -1;
}

int evt_mtx_destroy(wm_handle_t handle)
{
    evt_t *pstEvt = (evt_t *)handle;

    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
    //CHK_FUN_RE(pthread_cond_broadcast(&pstEvt->cond), -1);
    CHK_FUN_RE(pthread_cond_destroy(&pstEvt->cond), -1);
    CHK_FUN_RE(pthread_condattr_destroy(&pstEvt->condattr), -1);
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    CHK_FUN_RE(pthread_mutex_destroy(&pstEvt->mutex), -1);
    free(pstEvt);
    return 0;
}

int evt_mtx_signal(wm_handle_t handle)
{
    evt_t *pstEvt = (evt_t *)handle;
    
    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
    if (pstEvt->state == 0)
    {
        CHK_FUN_RE(pthread_cond_signal(&pstEvt->cond), -1);
        pstEvt->state = 1;
    }
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    return 0;
}

int evt_mtx_wait(wm_handle_t handle, unsigned long ulMilsecond)
{
    evt_t *pstEvt = (evt_t *)handle;

    CHK_ARG_RE(!pstEvt, -1);

    if (ulMilsecond == (unsigned long)-1)
    {
        CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
        while (pstEvt->state != 1)
            pthread_cond_wait(&pstEvt->cond, &pstEvt->mutex);
    }
    else
    {
        clockid_t clock_id;
        struct timespec tp;

        CHK_FUN_RE(pthread_condattr_getclock(&pstEvt->condattr, &clock_id), -1);
        CHK_FUN_RE(clock_gettime(clock_id, &tp), -1);
        
        tp.tv_sec  += ulMilsecond / 1000;
        tp.tv_nsec += (ulMilsecond % 1000) * 1000000;
        if (tp.tv_nsec > 1000000000)
        {
            tp.tv_nsec -= 1000000000;
            tp.tv_sec  += 1;
        }

        CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
        while (pstEvt->state != 1)
        {
            int rc = pthread_cond_timedwait(&pstEvt->cond, &pstEvt->mutex, &tp);
            if (rc == ETIMEDOUT)
            {
                CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
                return -1;
            }
        }
    }

    pstEvt->state = 0;
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    return 0;
}

int evt_rw_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeRelative)
{
    evt_rw_t *pstEvt = NULL;
    
    CHK_ARG_RE(!pHandle, -1);
    pstEvt = (evt_rw_t *)calloc(1, sizeof(evt_rw_t));
    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_GT(pthread_mutex_init(&pstEvt->mutex, NULL), ERR_EXIT1);
    CHK_FUN_GT(pthread_mutex_lock(&pstEvt->mutex), ERR_EXIT2);
    
    if (s32IsShared || s32IsTimeRelative)
    {
        CHK_FUN_GT(pthread_condattr_init(&pstEvt->condattr), ERR_EXIT2); 
        if (s32IsShared)
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstEvt->condattr, PTHREAD_PROCESS_SHARED), ERR_EXIT3);
        }
        else
        {
            CHK_FUN_GT(pthread_condattr_setpshared(&pstEvt->condattr, PTHREAD_PROCESS_PRIVATE), ERR_EXIT3);
        }

        if (s32IsTimeRelative)
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstEvt->condattr, CLOCK_MONOTONIC), ERR_EXIT3);
        }
        else
        {
            CHK_FUN_GT(pthread_condattr_setclock(&pstEvt->condattr, CLOCK_REALTIME), ERR_EXIT3);
        }

        CHK_FUN_GT(pthread_cond_init(&pstEvt->cond, &pstEvt->condattr), ERR_EXIT3);
    }

    *pHandle = (wm_handle_t)pstEvt;
    CHK_FUN_GT(pthread_mutex_unlock(&pstEvt->mutex), ERR_EXIT4);
    return 0;

ERR_EXIT4:
    pthread_cond_destroy(&pstEvt->cond);
ERR_EXIT3:
    pthread_condattr_destroy(&pstEvt->condattr);
ERR_EXIT2:
    pthread_mutex_destroy(&pstEvt->mutex);
ERR_EXIT1:
    free(pstEvt);
    return -1;
}

int evt_rw_destroy(wm_handle_t handle)
{
    evt_rw_t *pstEvt = (evt_rw_t *)handle;

    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
    //CHK_FUN_RE(pthread_cond_broadcast(&pstEvt->cond), -1);
    CHK_FUN_RE(pthread_cond_destroy(&pstEvt->cond), -1);
    CHK_FUN_RE(pthread_condattr_destroy(&pstEvt->condattr), -1);
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    CHK_FUN_RE(pthread_mutex_destroy(&pstEvt->mutex), -1);
    free(pstEvt);
    return 0;
}

int evt_wr_broadcast(wm_handle_t handle)
{
    evt_rw_t *pstEvt = (evt_rw_t *)handle;
    
    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);

    pstEvt->s32Wr = 1;

    if (pstEvt->s32Rd > 0)
    {
        CHK_FUN_RE(pthread_cond_broadcast(&pstEvt->cond), -1);
    }
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    return 0;
}

int evt_rd_reset(wm_handle_t handle)
{
    evt_rw_t *pstEvt = (evt_rw_t *)handle;
    
    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
    pstEvt->s32Wr = 0;
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    return 0;
}

int evt_rd_wait(wm_handle_t handle, unsigned long ulMilsecond)
{
    int rc = 0;
    evt_rw_t *pstEvt = (evt_rw_t *)handle;

    CHK_ARG_RE(!pstEvt, -1);
    CHK_FUN_RE(pthread_mutex_lock(&pstEvt->mutex), -1);
    if (ulMilsecond == 0)
    {
        if (pstEvt->s32Wr != 1)
        {
            rc = -1;
        }
        else
        {
            pstEvt->s32Wr = 0;
            rc = 0;
        }
    }
    else if (ulMilsecond == (unsigned long)-1)
    {
        do {
            ++pstEvt->s32Rd;
            rc = pthread_cond_wait(&pstEvt->cond, &pstEvt->mutex);
            --pstEvt->s32Rd;
        } while (rc != 0);
    }
    else
    {
        clockid_t clock_id;
        struct timespec tp;

        CHK_FUN_RE(pthread_condattr_getclock(&pstEvt->condattr, &clock_id), -1);
        CHK_FUN_RE(clock_gettime(clock_id, &tp), -1);
        
        tp.tv_sec  += ulMilsecond / 1000;
        tp.tv_nsec += (ulMilsecond % 1000) * 1000000;
        if (tp.tv_nsec > 1000000000)
        {
            tp.tv_nsec -= 1000000000;
            tp.tv_sec  += 1;
        }

        do {

            ++pstEvt->s32Rd;
            rc = pthread_cond_timedwait(&pstEvt->cond, &pstEvt->mutex, &tp);
            --pstEvt->s32Rd;
        } while (rc != 0 && rc != ETIMEDOUT);
    }
    CHK_FUN_RE(pthread_mutex_unlock(&pstEvt->mutex), -1);
    return rc;
}

