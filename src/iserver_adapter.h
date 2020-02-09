#ifndef ICENET_ISERVER_ADAPTER_H
#define ICENET_ISERVER_ADAPTER_H

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct iserver_adapter
{
    bool batch_push_packet;
    int (*handle_packet)(struct connection *conn, struct packet *packet);
    bool (*handle_batch_packet)(struct connection *conn, struct packet_queue *queue);
};

void iserver_adapter_set_batch(struct iserver_adapter *adapter, bool value)
{
    adapter->batch_push_packet = value;
}

#ifdef __cplusplus
}
#endif

#endif /* ICENET_ISERVER_ADAPTER_H */
