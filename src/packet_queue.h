#ifndef ICENET_PACKET_QUEUE_H
#define ICENET_PACKET_QUEUE_H

#include "internal.h"

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
    struct channel *channel;
    struct packet *next;
    void (*free)(struct packet *packet);
};

struct packet_queue
{
    int size;
    struct packet *head;
    struct packet *tail;
};

void packet_set_header(struct packet *packet, struct packet_header *header);

struct packet_header* packet_get_header(struct packet *packet);

uint32_t packet_get_channel_id(struct packet *packet);


bool packet_queue_init(struct packet_queue *queue);

void packet_queue_destroy(struct packet_queue *queue);

struct packet* packet_queue_pop(struct packet_queue *queue);

void packet_queue_push(struct packet_queue *queue, struct packet *packet);

void packet_queue_clear(struct packet_queue *queue);

struct packet* packet_queue_get_timeout_list(struct packet_queue *queue, int64_t now);

void packet_queue_move_to(struct packet_queue *src, struct packet_queue *dst);

bool packet_queue_empty(struct packet_queue *queue);

#ifdef __cplusplus
}
#endif

#endif /* ICENET_PACKET_QUEUE_H */
