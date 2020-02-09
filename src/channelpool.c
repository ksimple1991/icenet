#include "channelpool.h"
#include "share_func.h"
#include "util.h"
#include <stdatomic.h>

#define CHANNEL_CLUSTER_SIZE 25

static atomic_int_least32_t global_channel_id;

struct channel *channel_get_next(struct channel *channel)
{
    return channel->next;
}

bool channelpool_init(struct channelpool *pool)
{
    pool->max_use_count = 0;
    pool->use_list_head = NULL;
    pool->use_list_tail = NULL;
    pool->free_list_head = NULL;
    pool->free_list_tail = NULL;
    // pool->cluster_list = NULL;

    INIT_LIST_HEAD(&pool->cluster);

    if (init_pthread_lock(&pool->mutex) != 0)
    {
        return false;
    }

    return true;
}

void channelpool_destroy(struct channelpool *pool)
{
    pthread_mutex_destroy(&pool->mutex);

    // if (pool->cluster_list == NULL)
    // {
    //     return;
    // }

    // while (pool->cluster_list != NULL)
    // {
    //     struct channel *channel = pool->cluster_list;
    //     pool->cluster_list = pool->cluster_list->next;
        
    //     /**
    //      * TODO: free channel
    //      */
    //     free(channel);
    // }

    struct channel *channel;
    struct channel *next;

    list_for_each_entry_safe(channel, next, &pool->cluster, cluster)
    {
        /**
         * TODO: destroy channel
         */
        free(channel);
    }


}

struct channel* channelpool_alloc_channel(struct channelpool *pool)
{
    struct channel *channel;

    pthread_mutex_lock(&pool->mutex);
    if (pool->free_list_head == NULL)
    {
        assert(CHANNEL_CLUSTER_SIZE > 2);
        struct channel *channel_cluster = (struct channel *) \
            malloc(sizeof(struct channel) * CHANNEL_CLUSTER_SIZE);
        if (channel_cluster == NULL)
        {
            pthread_mutex_unlock(&pool->mutex);
            return NULL;
        }
        memset(channel_cluster, 0, sizeof(struct channel) * CHANNEL_CLUSTER_SIZE);

        /**
         * TODO: init channel
         */
        list_add_tail(&channel_cluster->cluster, &pool->cluster);

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
    pthread_mutex_lock(&pool->mutex);

    // 查找use_map，并删除
struct channel *loop = pool->use_map;
    while (loop != NULL)
    {
        if (loop->id == channel->id)
        {
            break;
        }
        loop = loop->next;
    }

    if (loop == NULL)
    {
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }

    // TODO: 从use_map中移除
    assert(false);


    // 加入到use_list
    if (channel == pool->use_list_head)
    {
        pool->use_list_head = channel->next;
    }
    if (channel == pool->use_list_tail)
    {
        pool->use_list_tail = channel->prev;
    }

    if (channel->prev != NULL)
    {
        channel->prev->next = channel->next;
    }
    if (channel->next != NULL)
    {
        channel->next->prev = channel->prev;
    }

    // 加入到free_list
    channel->prev = pool->free_list_tail;
    channel->next = NULL;
    if (pool->free_list_tail == NULL)
    {
        pool->free_list_head = channel;
    }
    else
    {
        pool->free_list_tail->next = channel;
    }
    pool->free_list_tail = channel;
    channel->id = 0;
    channel->handler = NULL;
    channel->args = NULL;

    pthread_mutex_unlock(&pool->mutex);
    return true;
}

bool channelpool_append_channel(struct channelpool *pool, struct channel *channel)
{
    return true;
}

struct channel *channelpool_offer_channel(struct channelpool *pool, uint32_t id)
{
    return NULL;
}

struct channel* channelpool_get_timeout_list(struct channelpool *pool, int64_t now)
{
    struct channel *list = NULL;

    pthread_mutex_lock(&pool->mutex);
    if (pool->use_list_head == NULL)
    {
        pthread_mutex_unlock(&pool->mutex);
        return list;
    }
    
    struct channel *channel = pool->use_list_head;
    while (channel != NULL)
    {
        if (channel->expire_time >= now)
        {
            break;
        }

        /**
         * TODO: erase use_map
         */
        channel = channel->next;
    }

    if (channel != pool->use_list_head)
    {
        list = pool->use_list_head;
        if (channel == NULL)
        {
            pool->use_list_head = pool->use_list_tail = NULL;
        }
        else
        {
            if (channel->prev != NULL)
            {
                channel->prev->next = NULL;
            }
            channel->prev = NULL;
            pool->use_list_head = channel;
        }
        
    }
    pthread_mutex_unlock(&pool->mutex);
    return list;
}

int channelpool_get_use_list_count(struct channelpool *pool)
{
    return pool->use_map_size;
}

bool channelpool_append_free_list(struct channelpool *pool, struct channel *add_list)
{
    if (add_list == NULL)
    {
        return true;
    }

    pthread_mutex_lock(&pool->mutex);
    struct channel *tail = add_list;
    while (tail->next != NULL)
    {
        tail->id = 0;
        tail->handler = NULL;
        tail->args = NULL;
        tail = tail->next;
    }

    tail->id = 0;
    tail->handler = NULL;
    tail->args = NULL;

    add_list->prev = pool->free_list_tail;
    if (pool->free_list_tail == NULL)
    {
        pool->free_list_head = add_list;
    }
    else
    {
        pool->free_list_tail->next = add_list;
    }
    pool->free_list_tail = add_list;

    pthread_mutex_unlock(&pool->mutex);
    return true;
}

void channelpool_set_expire_time(struct channelpool *pool, struct channel *channel, int64_t now)
{
    pthread_mutex_lock(&pool->mutex);
    if (channel != NULL)
    {
        channel->expire_time = now;
    }
    pthread_mutex_unlock(&pool->mutex);
}
