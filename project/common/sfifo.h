/*
* sfifo.h
* wangkaichao2@163.com 2018-09-21
* only two thread: one write, one read can safe
*/
// memory barrier
#ifndef _SFIFO_H
#define _SFIFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__arm__)
#define mb() __asm__ __volatile__("isb":::"memory")
#define rmb() __asm__ __volatile__("dsb":::"memory")
#define wmb() __asm__ __volatile__("dmb":::"memory")
#elif defined(__x86_64)
#define mb() __asm__ __volatile__("mfence":::"memory")
#define rmb() __asm__ __volatile__("lfence":::"memory")
#define wmb() __asm__ __volatile__("sfence":::"memory")
#else
#define mb() __asm__ __volatile__("":::"memory")
#define rmb() __asm__ __volatile__("":::"memory")
#define wmb() __asm__ __volatile__("":::"memory")
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t in;
    uint32_t out;
} sfifo_t;

sfifo_t *sfifo_init(uint8_t *buffer, uint32_t size);
sfifo_t *sfifo_alloc(uint32_t size);
void sfifo_free(sfifo_t * fifo);
uint32_t sfifo_put(sfifo_t *fifo, const uint8_t *buffer, uint32_t len);
uint32_t sfifo_get(sfifo_t *fifo, uint8_t *buffer, uint32_t len);
uint32_t sfifo_rest(const sfifo_t *fifo);

#ifdef __cplusplus
}
#endif

#endif
