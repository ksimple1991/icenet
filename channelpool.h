#ifndef ICENET_CHANNEL_POOL_H
#define ICENET_CHANNEL_POOL_H

#include "connection.h"
#include "list.h"
#include "util.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct channel
{
    uint32_t id;
    void *args;
    struct ipacket_handler *handler;
    int64_t expire_time;
    struct channel *prev;
    struct channel *next;
};

struct channelpool
{
    pthread_mutex_t mutex;
    struct list_head cluster_list;

    // TODO: hash map
    void *use_map;
    int use_map_size;

    struct channel *free_list_head;
    struct channel *free_list_tail;
    struct channel *use_list_head;
    struct channel *use_list_tail;
    int max_use_count;
};

bool channelpool_init(struct channelpool *pool);

void channelpool_destroy(struct channelpool *pool);

struct channel* channelpool_alloc_channel(struct channelpool *pool);

bool channelpool_free_channel(struct channelpool *pool, struct channel *channel);

struct channel* channelpool_get_timeout_list(struct channelpool *pool);

int channelpool_get_use_list_count(struct channelpool *pool);


#ifdef __cplusplus
}
#endif

#endif /* ICENET_CHANNEL_POOL_H */
