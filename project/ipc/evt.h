#ifndef EVT_H
#define EVT_H

#include "wm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

int evt_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeAbs);
int evt_destroy(wm_handle_t handle);
int evt_signal(wm_handle_t handle);
int evt_broadcast(wm_handle_t handle);
int evt_wait(wm_handle_t handle, unsigned long ulTimeout);

#ifdef __cplusplus
}
#endif

#endif
