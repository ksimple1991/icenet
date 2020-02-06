#include "connection.h"
#include "util.h"

bool connection_init(struct connection *conn, struct isocket *socket, \
    struct packet_streamer *streamer)
{
    conn->isserver = false;
    conn->default_packet_handler = NULL;
    conn->ioc = NULL;
    conn->socket = socket;
    conn->streamer = streamer;
    conn->queue_timeout = 500;
    conn->queue_limit = 50;
    conn->queue_total_size = 0;

    if (channelpool_init(&conn->pool) == false)
    {
        return false;
    }

    if (init_pthread_lock(&conn->output_mutex) != 0)
    {
        channelpool_destroy(&conn->pool);
        return false;
    }

    if (pthread_cond_init(&conn->output_cond, NULL) != 0)
    {
        pthread_mutex_destroy(&conn->output_mutex);
        channelpool_destroy(&conn->pool);
        return false;
    }

    if (packet_queue_init(&conn->input_queue) == false)
    {
        pthread_cond_destroy(&conn->output_cond);
        pthread_mutex_destroy(&conn->output_mutex);
        channelpool_destroy(&conn->pool);
        return false;
    }

    if (packet_queue_init(&conn->output_queue) ==false)
    {
        pthread_cond_destroy(&conn->output_cond);
        pthread_mutex_destroy(&conn->output_mutex);
        channelpool_destroy(&conn->pool);
        return false;
    }

    return true;
}

void connection_destroy(struct connection *conn)
{
    if (conn != NULL)
    {
        connection_disconnect(conn);
        conn->socket = NULL;
        conn->ioc = NULL;

        packet_queue_destroy(&conn->input_queue);
        packet_queue_destroy(&conn->output_queue);
        packet_queue_destroy(&conn->my_queue);

        pthread_mutex_destroy(&conn->output_mutex);
        pthread_cond_destroy(&conn->output_cond);

        channelpool_destroy(&conn->pool);
    }
}

void connection_set_server(struct connection *conn, bool is_server)
{
    conn->isserver = is_server;
}

void connection_set_iocomponent(struct connection *conn, struct iocomponent *ioc)
{
    conn->ioc = ioc;
}

void connection_set_default_packet_handler(struct connection *conn, struct ipacket_handler *ph)
{
    conn->default_packet_handler = ph;
}

void connection_set_queue_limit(struct connection *conn, int limit)
{
    conn->queue_limit = limit;
}

void connection_set_iocomponent(struct connection *conn, struct iocomponent *ioc)
{
    conn->ioc = ioc;
}

struct iocomponent* connection_get_iocomponetn(struct connection *conn)
{
    return conn->ioc;
}

int connection_get_input_queue_length(struct connection *conn)
{
    return conn->input_queue.size;
}

int connection_get_output_queue_length(struct connection *conn)
{
    return conn->output_queue.size;
}

bool connection_is_connect_state(struct connection *conn)
{
    if (conn->ioc != NULL)
    {
        return iocomponent_is_connect_state(conn->ioc);
    }

    return false;
}

/**
 * 连接断开，将发送队列中的packet全部超时
 */
void connection_disconnect(struct connection *conn)
{
    pthread_mutex_lock(&conn->output_mutex);
    packet_queue_move_to(&conn->output_queue, &conn->my_queue);
    pthread_mutex_unlock(&conn->output_mutex);
    connection_check_timeout(conn, ICENET_MAX_TIME);
}

bool connection_post_packet(struct connection *conn, struct packet *packet, bool noblocking)
{
    if (conn->ioc == NULL)
    {
        return false;
    }

    if (!connection_is_connect_state(conn->ioc))
    {
        if (conn->ioc->auto_reconn == false)
        {
            return false;
        }
        else if (conn->output_queue.size > 10)
        {
            return false;
        }
        else
        {
            /**
             * TODO: tcpcomponent 初始化
             */
        }
    }

    pthread_mutex_lock(&conn->output_mutex);
    conn->queue_total_size = conn->output_queue.size + \
        channelpool_get_use_list_count(&conn->pool) + conn->my_queue.size;
    if (!conn->isserver && conn->queue_limit > 0 \
        && conn->queue_total_size >= conn->queue_limit && noblocking)
    {
        pthread_mutex_unlock(&conn->output_mutex);
        return false;
    }
    pthread_mutex_unlock(&conn->output_mutex);

    struct channel *channel = NULL;
    packet->expire_time = conn->queue_timeout;
    if (conn->streamer->exist_packet_header)
    {
        /**
         * 存在包头，则设置channel
         */
    }

    pthread_mutex_lock(&conn->output_mutex);
    packet_queue_push(&conn->output_queue, packet);
    if (conn->ioc != NULL && conn->output_queue.size == 1)
    {
        iocomponent_enable_write(conn->ioc, true);
    }
    pthread_mutex_unlock(&conn->output_mutex);

    if (!conn->isserver && conn->queue_limit > 0)
    {
        pthread_mutex_lock(&conn->output_mutex);
        conn->queue_total_size = conn->output_queue.size + \
            channelpool_get_use_list_count(&conn->pool) + conn->my_queue.size;
        if (conn->queue_total_size > conn->queue_limit && noblocking == false)
        {
            bool *stop = NULL;
            if (conn->ioc != NULL && conn->ioc->owner != NULL)
            {

            }

            while (conn->queue_total_size > conn->queue_limit && stop && *stop == false)
            {
                struct timespec abstime;
                abstime.tv_sec = 1;
                abstime.tv_nsec = 0;
                if (pthread_cond_timedwait(&conn->output_cond, &conn->output_mutex, &abstime) != 0)
                {
                    if (connection_is_connect_state(conn->ioc) == false)
                    {
                        break;
                    }
                    conn->queue_total_size = conn->output_queue.size + \
                        channelpool_get_use_list_count(&conn->pool) + conn->my_queue.size;
                }
            }
        }
        pthread_mutex_unlock(&conn->output_mutex);
    }

    if (conn->isserver && conn->ioc != NULL)
    {
        iocomponent_sub_ref(conn->ioc);

    }

    return true;
}

bool connection_handle_packet(struct connection *conn, struct packet_buffer *input)
{
    struct packet *packet;
    int ret_code;
    void *args;
    struct channel *channel;
    struct ipacket_handler *packet_handler = NULL;

    if (conn->streamer->exist_packet_header && !conn->isserver)
    {

    }

    packet = conn->streamer->decode(input, header);
    if (packet == NULL)
    {
        
    }
    else
    {
        packet_set_header(packet, header);
        if (conn->isserver && )
        {
            return true;
        }
    }
    

    if (conn->isserver)
    {
        if (conn->ioc != NULL)
        {
            iocomponent_add_ref(conn->ioc);
        }
        ret_code = ;
    }
    else
    {
        if (packet_handler == NULL)
        {
            packet_handler = conn->default_packet_handler;
        }
        assert(packet_handler != NULL);

        ret_code = packet_handler->handle_packet(packet, args);
        channel->args = NULL;

        if (channel != NULL)
        {
            channelpool_append_channel(&conn->pool, channel);
        }
    }

    return true;
}

bool connection_check_timeout(struct connection *conn, int64_t now)
{
    struct channel *list = channelpool_get_timeout_list(&conn->pool, now);
    struct channel *channel;
    struct ipacket_handler *packer_handler;
    if (list != NULL)
    {
        /**
         * client endpoint, 给每个channel发送一个超时packet, 服务端把channel回收
         */
        if (!conn->isserver)
        {
            channel = list;
            while (channel != NULL)
            {
                packer_handler = channel->handler;
                if (packer_handler == NULL)
                {
                    packer_handler = conn->default_packet_handler;
                }

                if (packer_handler != NULL)
                {
                    packer_handler->handle_packet(, channel->args);
                    channel->args = NULL;
                }

                channel = channel->next;
            }
        }

        channelpool_append_free_list(&conn->pool, list);
    }

    pthread_mutex_lock(&conn->output_mutex);
    struct packet *packet_list = packet_queue_get_timeout_list(&conn->output_queue, now);
    pthread_mutex_unlock(&conn->output_mutex);

    while (packet_list != NULL)
    {
        struct packet *packet = packet_list;
        packet_list = packet_list->next;
        channel = packet->channel;


        if (channel != NULL)
        {

        }
    }

    if (!conn->isserver && conn->queue_limit > 0 \
        && conn->queue_total_size > conn->queue_limit)
    {
        pthread_mutex_lock(&conn->output_mutex);
        conn->queue_total_size = conn->output_queue.size + \
            channelpool_get_use_list_count(&conn->pool) + conn->my_queue.size;
        if (conn->queue_total_size <= conn->queue_limit)
        {
            pthread_cond_broadcast(&conn->output_cond);
        }
        pthread_mutex_unlock(&conn->output_mutex);
    }

    return true;
}

