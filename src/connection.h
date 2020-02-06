#ifndef ICENET_CONNECTION_H
#define ICENET_CONNECTION_H

#include "channelpool.h"
#include "iocomponent.h"
#include "isocket.h"
#include "packet_queue.h"
#include "packet_streamer.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define READ_WRITE_SIZE 8192

#ifndef UNUSED
#define UNUSED(v) ((void)(v))

struct ipacket_handler
{
    void (*destroy)(struct ipacket_handler *packet_handler);
    int (*handle_packet)(struct packet *packet, void *args);
};

struct connection
{
    bool isserver;
    struct ipacket_handler *default_packet_handler;
    struct iocomponent *ioc;
    struct isocket *socket;
    struct packet_streamer *streamer;

    struct packet_queue input_queue;
    struct pakcet_queue output_queue;
    struct packet_queue my_queue;
    pthread_mutex_t output_mutex;
    pthread_cond_t output_cond;
    int queue_total_size;
    int queue_limit;
    int queue_timeout;

    struct channelpool pool;
};

bool connection_init(struct connection *conn, struct isocket *socket, \
    struct packet_streamer *streamer);

void connection_set_server(struct connection *conn, bool is_server);

void connection_set_iocomponent(struct connection *conn, struct iocomponent *ioc);

void connection_set_default_packet_handler(struct connection *conn, struct ipacket_handler *ph);

/**
 * 设置队列最大长度，0 - 不限制
 */
void connection_set_queue_limit(struct connection *conn, int limit);

void connection_set_iocomponent(struct connection *conn, struct iocomponent *ioc);

struct iocomponent* connection_get_iocomponetn(struct connection *conn);

int connection_get_input_queue_length(struct connection *conn);

int connection_get_output_queue_length(struct connection *conn);

bool connection_is_connect_state(struct connection *conn);

void connection_disconnect(struct connection *conn);

bool connection_post_packet(struct connection *conn, struct packet *packet);

bool connection_handle_packet(struct connection *conn, struct packet_buffer *input);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ICENET_CONNECTION_H */
