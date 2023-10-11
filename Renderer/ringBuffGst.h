#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <gst/gst.h>

#define BUFF_SIZE 30

struct ringBuff {
    GstBuffer* data[BUFF_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t len;
    bool(*push)(struct ringBuff*, GstBuffer*);
    GstBuffer*(*pop)(struct ringBuff*);
};

static inline bool isEmpty(struct ringBuff *ring) {
    return (ring->len == 0);
}

static inline bool isFull(struct ringBuff *ring) {
    return (ring->len == BUFF_SIZE);
}

static inline bool push(struct ringBuff *ring, GstBuffer* data) {
    bool ret = false;
    if (!isFull(ring)) {
        ring->data[ring->tail % BUFF_SIZE] = data;
        ring->tail++;
        ring->len++;
        ret = true;
    }
    else
        fprintf(stderr, "[ringBufferGst::%s]: Ring buffer is full!\n", __func__);
    return ret;
}

static inline GstBuffer* pop(struct ringBuff *ring) {
    void* ret = NULL;
    if (!isEmpty(ring)) {
        ret = ring->data[ring->head % BUFF_SIZE];
        ring->data[ring->head % BUFF_SIZE] = NULL;
        ring->head++;
        ring->len--;
    }
    else
        fprintf(stderr, "[ringBufferGst::%s]: Ring buffer is empty!\n", __func__);
    return ret;
}

static struct ringBuff* initRingBuff() {
    int i;
    struct ringBuff* ret = (struct ringBuff*)malloc(sizeof(struct ringBuff));
    if (!ret)
        return NULL;
    ret->head = 0;
    ret->tail = 0;
    ret->len = 0;
    for (i = 0; i < BUFF_SIZE; i++)
        ret->data[i] = NULL;
    ret->push = push;
    ret->pop = pop;
    return ret;
}
