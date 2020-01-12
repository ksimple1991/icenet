#ifndef ICENET_CONNECTION_H
#define ICENET_CONNECTION_H

#include "iocomponent.h"
#include "isocket.h"
#include "packet_streamer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



struct packet_queue
{
    int size;
    void *head;
    void *tail;
};

struct connection
{
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

bool connection_post_packet()
{
    return true;
}




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ICENET_CONNECTION_H */
