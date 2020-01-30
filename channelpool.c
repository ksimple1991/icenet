#include "channelpool.h"
#include "share_func.h"
#include "util.h"
#include <stdatomic.h>

#define CHANNEL_CLUSTER_SIZE 25

static atomic_int_least32_t global_channel_id;

bool channelpool_init(struct channelpool *pool)
{
    return true;
}

void channelpool_destroy(struct channelpool *pool)
{

}

struct channel* channelpool_alloc_channel(struct channelpool *pool)
{
    struct channel *channel;

    pthread_mutex_lock(&pool->mutex);
    if (pool->free_list_head == NULL)
    {
        struct channel *channel_cluster = (struct channel *) \
            malloc(sizeof(struct channel) * CHANNEL_CLUSTER_SIZE);
        if (channel_cluster == NULL)
        {
            pthread_mutex_unlock(&pool->mutex);
            return NULL;
        }
        memset(channel_cluster, 0, sizeof(struct channel) * CHANNEL_CLUSTER_SIZE);

        pool->free_list_head = pool->free_list_tail = &channel_cluster[1];
        for (int i = 2; i < CHANNEL_CLUSTER_SIZE; i++)
        {
            pool->free_list_tail->next = &channel_cluster[i];
            channel_cluster[i].prev = pool->free_list_tail;
            pool->free_list_tail = pool->free_list_tail->next;
        }
        pool->free_list_head->prev = NULL;
        pool->free_list_tail->next = NULL;

        channel = &channel_cluster[0];
    }
    else
    {
        channel = pool->free_list_head;
        pool->free_list_head = pool->free_list_head->next;
        if (pool->free_list_head != NULL)
        {
            pool->free_list_head->prev = NULL;
        }
        else
        {
            pool->free_list_tail = NULL;
        }
    }
    
    /* 把channel放到use_list中 */
    channel->prev = pool->use_list_tail;
    channel->next = NULL;
    channel->expire_time = ICENET_MAX_TIME;
    pool->use_list_tail = channel;

    uint32_t id = atomic_fetch_add(&global_channel_id, 1);
    id &= 0x0FFFFFFF;
    if (id == 0)
    {
        id = 1;
        atomic_exchange(&global_channel_id, 1);
    }

    channel->id = id;
    channel->handler = NULL;
    channel->args = NULL;

    /**
     * TODO: insert use map
     */
    if (pool->max_use_count < (int)pool->map_size)
    {
        pool->max_use_count = pool->map_size;
    }

    pthread_mutex_unlock(&pool->mutex);

    return channel;
}

bool channelpool_free_channel(struct channelpool *pool, struct channel *channel)
{

}

bool channnelpool_appendp_channel(struct channelpool *pool, struct channel *channel)
{

}

struct channel* channelpool_get_timeout_list(struct channelpool *pool)
{

}

int channelpool_get_use_list_count(struct channelpool *pool)
{

}

bool channelpool_append_free_list(struct channelpool *pool, struct channel *add_list)
{

}