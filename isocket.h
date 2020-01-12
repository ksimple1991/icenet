#ifndef ISOCKET_H
#define ISOCKET_H

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>


#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * IO对象的抽象，如 tcp、udp、unix domain等服务端、客户端
 * tcp_acceptor, tcp_ioc, udp_ioc
 */
struct iocomponent
{
    struct list_head *list;
    struct transport *owner;
    struct isocket *socket;
    struct epoll_socket_event *socket_event;
    int state; // 连接状态
    bool auto_reconn; // 自动重连
    bool inuse;
    int64_t last_use_time;
    char *buffer;

    void (*init)(struct iocomponent *ioc);
    void (*close)(struct iocomponent *ioc);
    bool (*handle_wirte_event)(struct iocomponent *ioc);
    bool (*handle_read_event)(struct iocomponent *ioc);
};

typedef struct isocket
{
    int fd;
    // struct sockaddr address;
    struct sockaddr_in address;
    struct iocomponent *ioc;
} isocket_t;


#ifdef __cplusplus
}
#endif

#endif /* ISOCKET_H */
