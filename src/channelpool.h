#ifndef ICENET_CHANNEL_POOL_H
#define ICENET_CHANNEL_POOL_H

#include "internal.h"
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

    struct list_head cluster;
};

struct channel_cluster
{
    struct channel *channel_list;
    struct channel_cluster *next;
};

struct channelpool
{
    pthread_mutex_t mutex;
    struct channel *cluster_list;
    struct list_head cluster;

    // TODO: hash map
    struct channel *use_map;
    int map_size;
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

bool channelpool_append_channel(struct channelpool *pool, struct channel *channel);

bool channelpool_append_free_list(struct channelpool *pool, struct channel *add_list);

struct channel* channelpool_offer_channel(struct channelpool *pool, uint32_t id);

struct channel* channelpool_get_timeout_list(struct channelpool *pool, int64_t now);

int channelpool_get_use_list_count(struct channelpool *pool);

void channelpool_set_expire_time(struct channelpool *pool, struct channel *channel, int64_t now);

#ifdef __cplusplus
}
#endif

#endif /* ICENET_CHANNEL_POOL_H */
