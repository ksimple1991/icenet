#ifndef ICENET_CONNECTION_H
#define ICENET_CONNECTION_H

#include "iocomponent.h"
#include "isocket.h"
#include "packet_queue.h"
#include "packet_streamer.h"

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
    int queue_total_size;
    int queue_limit;
    int queue_timeout;
};

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

    // init queue

    return true;
}







bool connection_post_packet()
{
    return true;
}




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ICENET_CONNECTION_H */
