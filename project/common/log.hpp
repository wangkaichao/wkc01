/*
* wangkaichao2@163.com 2018-09-16
*/
#ifndef _LOG_H
#define _LOG_H

#include <stdint.h>
#include <mutex>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdarg.h>
#include <stdio.h>

class clslog {
public:

    const char *shm_name = "log_shm";
    const static size_t magic = 0x55995599;
    const static size_t max_size = 1 << 22;
    
    typedef struct {
        size_t match;
        size_t head;
        size_t tail;
        size_t size;
        int8_t buf[max_size];
    } shm_st;

    clslog() = delete;
    
    ~clslog();
    
    clslog(const clslog &) = delete;
    
    clslog& operator=(const clslog &) = delete;
    
    static clslog& instance(int32_t);

    size_t write(const int8_t *, size_t);
    
    size_t read(int8_t *, size_t);

private:

    clslog(int32_t);

    mutable std::mutex m_mut;
    
    int32_t m_type;
    
    shm_st *m_shm;

    size_t m_rpos; // used by logcat
};

#endif
