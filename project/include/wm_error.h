#ifndef WM_ERROR_H
#define WM_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_NO_ERROR                             0
#define ERR_ERROR                               -1

#define ERR_THREAD_QUEUE_NOT_INITIALIZED        -1110

#define ERR_MSG_RESPONSE_QUEUE_NOT_SET          -1300 // 0xAEC
#define ERR_MSG_SIGNALSIZE_NOT_SET              -1301 // 0xAEB
#define ERR_MSG_COPY_MSG_ALLOC_FAILED           -1302 // 0xAEA

// All error codes have negative values.
#define ERR_MSG_QUEUE_CLOSE_FAILED              -1410 // 0xA7E
#define ERR_MSG_QUEUE_CLOSE_NO_HANDLE           -1411 // 0xA7D
#define ERR_MSG_QUEUE_GETATTR_FAILED            -1412 // 0xA7C
#define ERR_MSG_QUEUE_GETATTR_NO_HANDLE         -1413 // 0xA7B
#define ERR_MSG_QUEUE_GETNR_FAILED              -1414 // 0xA7A
#define ERR_MSG_QUEUE_GETNR_NO_HANDLE           -1415 // 0xA79
#define ERR_MSG_QUEUE_HANDLE_UNDEFINED          -1416 // 0xA78
#define ERR_MSG_QUEUE_NAME_UNKNOWN              -1417 // 0xA77
#define ERR_MSG_QUEUE_OPEN_FAILED               -1418 // 0xA76
#define ERR_MSG_QUEUE_RECEIVE_FAILED            -1419 // 0xA75
#define ERR_MSG_QUEUE_RECEIVE_NO_HANDLE         -1420 // 0xA74
#define ERR_MSG_QUEUE_SEND_FAILED               -1421 // 0xA73
#define ERR_MSG_QUEUE_SEND_NO_HANDLE            -1422 // 0xA72
#define ERR_MSG_QUEUE_SEND_NO_TARGET_HANDLE     -1423 // 0xA71
#define ERR_MSG_QUEUE_UNLINK_FAILED             -1424 // 0xA70
#define ERR_MSG_QUEUE_SELECT_FAILED             -1425 // 0xA6F
#define ERR_MSG_QUEUE_PTR_IS_NULL               -1426 // 0xA6E
#define ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL       -1427 // 0xA6D
#define ERR_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE  -1428 // 0xA6C

#define ERR_OBS_BLOCKING_QUEUE_NOT_ALLOWED      -1520 // 0xA10
#define ERR_OBS_SIGNAL_SIZE_NOT_SET             -1521 // 0xA0F

#ifdef __cplusplus
}
#endif

#endif

