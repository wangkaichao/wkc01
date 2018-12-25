/*
* clog.h
* wangkaichao2@163.com 2018-09-19
*/

#ifndef _CLOG_H
#define _CLOG_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define logDbg(fmt, ...) do{\
    printf("[D %s:%d]", __FUNCTION__, __LINE__);\
    printf(fmt, ##__VA_ARGS__);\
}while(0)

#define logErr(fmt, ...) do{\
    printf("[E %s:%d]", __FUNCTION__, __LINE__);\
    printf(fmt, ##__VA_ARGS__);\
}while(0)

//#define logDbg(fmt, ...) printf("[%s:%d][D]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
//#define logErr(fmt, ...) printf("[%s:%d][E]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGCAT_ENABLE           (0)
#define LOGCAT_ENABLE_TIME      (1)
#define LOGCAT_DISABLE          (2)
#define LOGCAT_DISABLE_TIME     (3)


#define log_dbg(fmt, ...) do{\
    logout(LOGCAT_DISABLE_TIME, (int8_t *)"[D %s:%d]", (int8_t *)__FUNCTION__, __LINE__);\
    logout(LOGCAT_DISABLE, (int8_t *)fmt, ##__VA_ARGS__);\
} while (0)

#define log_err(fmt, ...) do{\
    logout(LOGCAT_DISABLE_TIME, (int8_t *)"[E %s:%d]", (int8_t *)__FUNCTION__, __LINE__);\
    logout(LOGCAT_DISABLE, (int8_t *)fmt, ##__VA_ARGS__);\
} while (0)

size_t logout(int32_t cmd, const int8_t *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
