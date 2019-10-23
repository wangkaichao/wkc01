#ifndef EVT_H
#define EVT_H

#include "wm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

int evt_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeRelative);
int evt_destroy(wm_handle_t handle);

int evt_mtx_signal(wm_handle_t handle);
int evt_mtx_wait(wm_handle_t handle, unsigned long ulMilsecond);

int evt_wr_broadcast(wm_handle_t handle);
int evt_rd_wait(wm_handle_t handle, unsigned long ulMilsecond);
#ifdef __cplusplus
}
#endif

#endif
