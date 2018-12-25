/*
* log.cpp
* wangkaichao2@163.com 2018-09-29
*/
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log.hpp"
#include "clog.h"

/*
* @param[in] type 初始化log类型
*   @arg 0 只读方式, 不创建共享内存，logcat使用
*   @arg 1 读写方式，创建共享内存
*/
clslog::clslog(int32_t type):m_type(type), m_shm(nullptr), m_rpos(max_size + 1)
{
    int fd = shm_open(shm_name, O_RDWR|O_CREAT, 0755);
    if (fd < 0) {
        logErr("shm_open err!\n");
        return ;
    }
    
    off_t total_len = sizeof(shm_st);
    if (ftruncate(fd, total_len) < 0) {
        logErr("ftruncate err!\n");
        return ;
    }


    m_shm = (shm_st *)mmap(nullptr, total_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!m_shm) {
        logErr("mmap err!\n");
        return ;
    }

    if (m_type == 0) {
        if (m_shm->match != magic) {
            sleep(1);
        }
    }

    if (m_type == 1) {
        m_shm->head = m_shm->tail = 0;
        m_shm->size = 0;
        m_shm->match = magic;
        logDbg("log shm open\n");
        return ;
    }
}

clslog::~clslog()
{
    if (!m_shm)
        return ;
    
    off_t total_len = sizeof(shm_st);
    munmap(m_shm, total_len);

    /*if (m_type == 1) {
        shm_unlink(shm_name);
        logDbg("log shm unlink.\n");
    }*/
}

clslog& clslog::instance(int32_t type)
{
    static clslog ins(type);

    return ins;
}

size_t clslog::write(const int8_t *pbuf, size_t len)
{
    if (!pbuf)
        return 0;
    
    std::lock_guard<std::mutex> lk(m_mut);
    
    for (size_t i = 0; i < len; i++) {
        size_t index = (m_shm->tail + i) % max_size;
        m_shm->buf[index] = pbuf[i];
    }

    m_shm->tail = (m_shm->tail + len) % max_size;
    m_shm->size += len;
    if (m_shm->size > max_size) {
        m_shm->size = max_size;
        m_shm->head = m_shm->tail + 1;
    }

    return len;
}

size_t clslog::read(int8_t *pbuf, size_t len)
{
    if (!pbuf)
        return 0;

    std::lock_guard<std::mutex> lk(m_mut);
    if (m_shm->size == 0)
        return 0;

    if (m_rpos > max_size) {
        m_rpos = m_shm->head;
    }

    size_t i;

    for (i = 0; i < len; i++) {
        size_t index = (m_rpos + i) % max_size;
        if (index == m_shm->tail)
            break;
        pbuf[i] = m_shm->buf[index];
    }

    m_rpos += i;
    m_rpos %= max_size;

    return i;
}

size_t logout(int32_t cmd, const int8_t *fmt, ...)
{
    va_list ap;
    int8_t buf[1024];
    int l;

    if (!fmt)
        return 0;

    l = 0;

CONTINUE:
    switch (cmd) {
        case LOGCAT_ENABLE:
            va_start(ap, fmt);
            l += vsnprintf((char *)buf + l, sizeof(buf) - l, (char *)fmt, ap);
            va_end(ap);
            l = clslog::instance(1).write(buf, l);
            break;
        case LOGCAT_ENABLE_TIME: // 添加[xxxx]时间戳
		{
			struct timespec t;
			//clock_gettime(CLOCK_BOOTTIME, &t);
			clock_gettime(CLOCK_MONOTONIC_RAW, &t);
            l += snprintf((char *)buf + l, sizeof(buf) - l, "[%10ld.%06ld]", (long)t.tv_sec, t.tv_nsec % 1000);
			cmd = LOGCAT_ENABLE;
            goto CONTINUE;
		}
            break;
        case LOGCAT_DISABLE:
            va_start(ap, fmt);
            l += vsnprintf((char *)buf + l, sizeof(buf) - l, (char *)fmt, ap);
            va_end(ap);

            buf[sizeof(buf) - 1] = 0;
            l = fputs((char *)buf, stdout);
            break;
        case LOGCAT_DISABLE_TIME:
		{
			struct timespec t;
			//clock_gettime(CLOCK_BOOTTIME, &t);
			clock_gettime(CLOCK_MONOTONIC_RAW, &t);
            l += snprintf((char *)buf + l, sizeof(buf) - l, "[%10ld.%06ld]", (long)t.tv_sec, t.tv_nsec % 1000);
			cmd = LOGCAT_DISABLE;
            goto CONTINUE;
		}
        default:
            break;
    }

    return l;
}

