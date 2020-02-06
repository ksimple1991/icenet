#ifndef ICENET_TCP_H
#define ICENET_TCP_H

#include "iocomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_component_data
{
    int64_t start_connect_time;
};

bool tcp_acceptor_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server);

bool tcp_component_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server);

#ifdef __cplusplus
}
#endif

#endif /* ICENET_TCP_H */
