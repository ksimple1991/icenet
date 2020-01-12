#include "epollsocketevent.h"


bool epoll_event_init(struct epoll_socket_event *ep_event)
{
    ep_event->ep_fd = epoll_create(MAX_SOCKET_EVENTS);

    return ep_event->ep_fd >= 0 ? true : false;
}

bool epoll_event_destroy(struct epoll_socket_event *ep_event)
{
    if (ep_event->ep_fd != -1)
    {
        close(ep_event->ep_fd);
        ep_event->ep_fd = -1;
    }

    return true;
}

bool epoll_add_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket, bool enable_read, bool enable_write)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = socket;
    ev.events = 0;

    if (enable_read)
    {
        ev.events |= EPOLLIN;
    }

    if (enable_write)
    {
        ev.events |= EPOLLOUT;
    }

    int result;
    result = epoll_ctl(ep_event->ep_fd, EPOLL_CTL_MOD, socket->fd, &ev);
    if (result == 0)
    {
        return true;
    }

    return false;
}

bool epoll_set_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket, bool enable_read, bool enable_write)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = socket;
    ev.events = 0;

    return true;
}

bool epoll_remove_event(struct epoll_socket_event *ep_event, \
    struct isocket *socket)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = socket;
    ev.events = 0;

    int result;
    result = epoll_ctl(ep_event->ep_fd, EPOLL_CTL_DEL, socket->fd, &ev);
    if (result == 0)
    {
        return true;
    }

    return true;
}

int epoll_get_events(struct epoll_socket_event *ep_event, \
    int timeout, struct ioevent *ioevents, int cnt)
{
    struct epoll_event events[MAX_SOCKET_EVENTS];

    if (cnt > MAX_SOCKET_EVENTS)
    {
        cnt = MAX_SOCKET_EVENTS;
    }

    int result = epoll_wait(ep_event->ep_fd, events, cnt, timeout);
    if (result > 0)
    {
        memset(ioevents, 0, sizeof(struct ioevent) * result);
    }

    for (int i = 0; i < result; i++)
    {
        ioevents[i].ioc = events[i].data.ptr;

        if (events[i].events & (EPOLLERR | EPOLLHUP))
        {
            ioevents[i].error_occurred = true;
        }

        if (events[i].events & EPOLLIN)
        {
            ioevents[i].read_occurred = true;
        }

        if (events[i].events & EPOLLOUT)
        {
            ioevents[i].write_occurred = true;
        }
    }

    return result;
}
