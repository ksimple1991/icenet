#ifndef ICENET_PACKET_QUEUE_H
#define ICENET_PACKET_QUEUE_H

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct packet_header
{
    uint32_t chid;
    int32_t pcode;
    int32_t datalen;
};

struct packet
{
    struct packet_header header;
    int64_t expire_time;
    void *channel;
    struct packet *next;
};

struct packet_queue
{
    int size;
    struct packet *head;
    struct packet *tail;
};

bool packet_queue_init(struct packet_queue *queue);

void packet_queue_destroy(struct packet_queue *queue);

struct packet* packet_queue_pop(struct packet_queue *queue);

bool packet_queue_push(struct packet_queue *queue, struct packet *packet);

void packet_queue_clear(struct packet_queue *queue);

struct packet* packet_queue_get_timeout_list(struct packet_queue *queue, int64_t now);

void packet_queue_move_to(struct packet_queue *src, struct packet_queue *dst);

bool packet_queue_empty(struct packet_queue *queue);


#ifdef __cplusplus
}
#endif

#endif /* ICENET_PACKET_QUEUE_H */
