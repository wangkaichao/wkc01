#ifndef WM_TYPE_H
#define WM_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WM_FALSE = 0,
    WM_TRUE,
} WM_BOOL;

#define unsigned long       wm_id_t
#define void *              wm_handle_t

#define MAX(x, y)   (x) > (y) ? (x) : (y)
#define MIN(x, y)   (x) < (y) ? (x) : (y)

#ifdef __cplusplus
}
#endif

#endif

