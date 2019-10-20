#ifndef WM_LOG_H
#define WM_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOGD    printf
#define LOGI    printf
#define LOGW    printf
#define LOGE    printf

#define CHK_ARG(val)                            \
do {                                            \
    if ((val)) {                                \
        LOGE("%s %d: Invalid Parameter", __func__, __LINE__); \
    }                                           \
} while (0)

#define CHK_ARG_RV(val)                         \
do {                                            \
    if ((val)) {                                \
        LOGE("%s %d: Invalid Parameter", __func__, __LINE__); \
        return;                                 \
    }                                           \
} while (0)

#define CHK_ARG_RE(val, err)                    \
do {                                            \
    if ((val)) {                                \
        LOGE("%s %d: Invalid Parameter", __func__, __LINE__); \
        return err;                             \
    }                                           \
} while (0)

#define CHK_FUN(func, ret)                      \
do {                                            \
    ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s %d: %s failed %d(%#x)", __func__, __LINE__, #func, ret, ret); \
    }                                           \
} while (0)

#define CHK_FUN_RV(func)                        \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s %d: %s failed %d(%#x)", __func__, __LINE__, #func, ret, ret); \
        return;                                 \
    }                                           \
} while (0)

#define CHK_FUN_RE(func, err)                   \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s %d: %s failed %d(%#x)", __func__, __LINE__, #func, ret, ret); \
        return err;                             \
    }                                           \
} while (0)

#define CHK_FUN_GT(func, here)                  \
do {                                            \
    int ret = func;                             \
    if (ret != 0) {                             \
        LOGE("%s %d: %s failed %d(%#x)", __func__, __LINE__, #func, ret, ret); \
        goto here;                              \
    }                                           \
} while (0)

#define CHK_FUN_EQ(func, ret, err)              \
do {                                            \
    ret = func;                                 \
    if (ret != err) {                           \
        LOGE("%s %d: %s failed", __func__, __LINE__, #func); \
    }                                           \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
