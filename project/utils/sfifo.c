/*
* sfifo.c
* wangkaichao2@163.com 2018-09-21
*/
#include <stdlib.h>
#include <string.h>

#include "sfifo.h"
#include "wm_log.h"

sfifo_t *sfifo_init(uint8_t *buffer, uint32_t size)
{
    sfifo_t *fifo;
    fifo = malloc(sizeof(sfifo_t));
    if (!fifo) {
        LOGE("malloc err:%m\n");
        return NULL;
    }

    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;

    return fifo;
}

sfifo_t *sfifo_alloc(uint32_t size)
{
    uint8_t *buffer;
    sfifo_t *ret;

    buffer = malloc(size);
    if (!buffer) {
        LOGE("malloc err:%m\n");
        return NULL;
    }

    ret = sfifo_init(buffer, size);
    if (!ret) {
        free(buffer);
        return NULL;
    }

    return ret;
}

void sfifo_free(sfifo_t * fifo)
{
    free(fifo->buffer);
    free(fifo);
}

uint32_t sfifo_put(sfifo_t *fifo, const uint8_t *buffer, uint32_t len)
{
    uint32_t l;

    len = min(len, fifo->size - fifo->in + fifo->out);

    mb();

    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);
    memcpy(fifo->buffer, buffer + l, len - l);

    wmb();

    fifo->in += len;

    return len;
}

uint32_t sfifo_get(sfifo_t *fifo, uint8_t *buffer, uint32_t len)
{
    uint32_t l;

    len = min(len, fifo->in - fifo->out);

    rmb();

    l = min(len, fifo->size - (fifo->out & (fifo->size -1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);
    memcpy(buffer + l, fifo->buffer, len - l);

    mb();

    fifo->out += len;

    return len;
}

uint32_t sfifo_rest(const sfifo_t *fifo)
{
    return fifo->in - fifo->out;
}

