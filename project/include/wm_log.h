#ifndef WM_LOG_H
#define WM_LOG_H

#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

// /etc/syslog.conf
// user.debug       /dev/console
// user.info        /dev/console
// user.warning     /var/log/user.log
// user.err         /var/log/user.log

#if defined __arm__
//#pragma message("===arm===")
//#error "fuck"
#define LOG_OPEN(n) openlog(n, LOG_CONS | LOG_PID, LOG_USER)
#else
//#pragma message("===pc===")
#define LOG_OPEN(n) openlog(n, LOG_PERROR | LOG_PID, LOG_USER)
#endif

#ifndef TAG
#define TAG __FILE__
#endif

#define LOGA(fmt, args...)   syslog(LOG_ALERT,   "A/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGC(fmt, args...)   syslog(LOG_CRIT,    "C/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGE(fmt, args...)   syslog(LOG_ERR,     "E/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGW(fmt, args...)   syslog(LOG_WARNING, "W/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGN(fmt, args...)   syslog(LOG_NOTICE,  "N/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGI(fmt, args...)   syslog(LOG_INFO,    "I/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)
#define LOGD(fmt, args...)   syslog(LOG_DEBUG,   "D/%s %s %d: " fmt, TAG, __func__, __LINE__, ##args)

#define CHK_ARG(val)                            \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter.");             \
    }                                           \
} while (0)

#define CHK_ARG_RV(val)                         \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter.");             \
        return;                                 \
    }                                           \
} while (0)

#define CHK_ARG_RE(val, err)                    \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter.");             \
        return err;                             \
    }                                           \
} while (0)

#define CHK_ARG_GT(val, here)                   \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter.");             \
        goto here;                              \
    }                                           \
} while (0)

#define CHK_FUN(func, ret)                      \
do {                                            \
    ret = func;                                 \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x).", #func, ret, ret); \
    }                                           \
} while (0)

#define CHK_FUN_RV(func)                        \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x).", #func, ret, ret); \
        return;                                 \
    }                                           \
} while (0)

#define CHK_FUN_RE(func, err)                   \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x).", #func, ret, ret); \
        return err;                             \
    }                                           \
} while (0)

#define CHK_FUN_GT(func, here)                  \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x).", #func, ret, ret); \
        goto here;                              \
    }                                           \
} while (0)

#define CHK_FUN_EQ(func, ret, err)              \
do {                                            \
    ret = func;                                 \
    if (ret != err) {                           \
        LOGE("%s failed.", #func);              \
    }                                           \
} while (0)

#define CHK_ARG_M(val)                          \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter. %m");          \
    }                                           \
} while (0)

/*------------------------------------------------------------*/
#define CHK_ARG_RV_M(val)                       \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter. %m");          \
        return;                                 \
    }                                           \
} while (0)

#define CHK_ARG_RE_M(val, err)                  \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter. %m");          \
        return err;                             \
    }                                           \
} while (0)

#define CHK_ARG_GT_M(val, here)                 \
do {                                            \
    if ((val)) {                                \
        LOGE("Invalid Parameter. %m");          \
        goto here;                              \
    }                                           \
} while (0)

#define CHK_FUN_M(func, ret)                    \
do {                                            \
    ret = func;                                 \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x). %m", #func, ret, ret); \
    }                                           \
} while (0)

#define CHK_FUN_RV_M(func)                      \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x). %m", #func, ret, ret); \
        return;                                 \
    }                                           \
} while (0)

#define CHK_FUN_RE_M(func, err)                 \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x). %m", #func, ret, ret); \
        return err;                             \
    }                                           \
} while (0)

#define CHK_FUN_GT_M(func, here)                \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s failed %d(%#x). %m", #func, ret, ret); \
        goto here;                              \
    }                                           \
} while (0)

#define CHK_FUN_EQ_M(func, ret, err)            \
do {                                            \
    ret = func;                                 \
    if (ret != err) {                           \
        LOGE("%s failed. %m", #func);           \
    }                                           \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
