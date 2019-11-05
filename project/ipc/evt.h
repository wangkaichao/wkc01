#ifndef EVT_H
#define EVT_H

#include "wm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

int evt_mtx_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeRelative);
int evt_mtx_destroy(wm_handle_t handle);
int evt_mtx_signal(wm_handle_t handle);
int evt_mtx_wait(wm_handle_t handle, unsigned long ulMilsecond);

int evt_rw_create(wm_handle_t *pHandle, int s32IsShared, int s32IsTimeRelative);
int evt_rw_destroy(wm_handle_t handle);
int evt_wr_broadcast(wm_handle_t handle);

/**
 * @brief When first call evt_rd_wait() and ulMilsecond is zero. Before it,
 * call evt_rd_reset() make sure get the new signal.
 * @Note When first call evt_rd_wait() and ulMilsecond is zero, will return -1.
 * After then if get the new signal, the second call evt_rd_wait(), will return 0.
 * @param handle
 *
 * @return 0:successful, -1:failed.
 */
int evt_rd_reset(wm_handle_t handle);
int evt_rd_wait(wm_handle_t handle, unsigned long ulMilsecond);
#ifdef __cplusplus
}
#endif

#endif
