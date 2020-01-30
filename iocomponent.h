#ifndef ICENET_IOCOMONENT_H
#define ICENET_IOCOMONENT_H

#include <stdbool.h>
#include <stdint.h>
#include "epollsocketevent.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ICENET_MAX_TIME (INT64_MAX)

struct epoll_socket_event;
struct transport;
struct isocket;

enum
{
    IOC_CONNECTING = 1,
    IOC_CONNECTED,
    IOC_CLOSED,
    IOC_UNCONNECTED
};

enum
{
    IOC_TYPE_ACCEPTOR = 1,
    IOC_TYPE_TCP,
    IOC_TYPE_UDP,
};

/**
 * IO对象的抽象，如 tcp、udp、unix domain等服务端、客户端
 * tcp_acceptor, tcp_ioc, udp_ioc
 */
struct iocomponent
{
    struct list_head *list;
    int type;
    struct transport *owner;
    struct isocket *socket;
    struct epoll_socket_event *socket_event;
    int state; // 连接状态
    bool is_server;
    bool auto_reconn; // 自动重连
    bool inuse;
    int64_t last_use_time;
    void *extra;

    bool (*init)(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server);
    void (*close)(struct iocomponent *ioc);
    bool (*handle_wirte_event)(struct iocomponent *ioc);
    bool (*handle_read_event)(struct iocomponent *ioc);
    void (*check_timeout)(struct iocomonent *ioc, int64_t now);
};

struct iocomponent* iocomponent_new();

void iocomponent_del(struct iocomponent *ioc);

bool iocomponent_init(struct iocomponent *ioc, struct transport *owner, \
    struct isocket *socket);

void iocomponent_enable_write(struct iocomponent *ioc, bool write_on);

#ifdef __cplusplus
}
#endif

#endif /* ICENET_IOCOMONENT_H */
