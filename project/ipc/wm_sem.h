#ifndef WM_SEM_H
#define WM_SEM_H

#include "wm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

int wm_sem_create(wm_handle_t *pHandle, int s32Num, int s32IsShared, int s32IsTimeRelative);
int wm_sem_destroy(wm_handle_t handle);
int wm_sem_post(wm_handle_t handle);
int wm_sem_broadcast(wm_handle_t handle, int s32Num);
int wm_sem_wait(wm_handle_t handle, unsigned long ulMilsecond);
int wm_sem_getvalue(wm_handle_t handle, int *ps32Val);

#ifdef __cplusplus
}
#endif

#endif
