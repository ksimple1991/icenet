#ifndef ICENET_EPOLLSOCKETEVENT_H
#define ICENET_EPOLLSOCKETEVENT_H

#include "isocket.h"
#include "iocomponent.h"
#include "internal.h"
#include <stdbool.h>
#include <sys/epoll.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SOCKET_EVENTS 256

struct ioevnt
{
    bool read_occurred;
    bool write_occurred;
    bool error_occurred;
    struct iocomponent *ioc;
};

struct epoll_socket_event
{
    int ep_fd;
};

bool epoll_event_init(struct epoll_socket_event *ep_event);

bool epoll_event_destroy(struct epoll_socket_event *ep_event);

bool epoll_add_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket, bool enable_read, bool enable_write);

bool epoll_set_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket, bool enable_read, bool enable_write);

bool epoll_remove_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket);

int epoll_get_events(struct epoll_socket_event *ep_event, \
    int timeout, struct ioevent *ioevents, int cnt);


#ifdef __cplusplus
}
#endif

#endif /* ICENET_EPOLLSOCKETEVENT_H */
