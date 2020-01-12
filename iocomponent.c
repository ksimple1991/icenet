#include "iocomponent.h"
#include "isocket.h"

bool iocomponent_init(struct iocomponent *ioc, struct transport *owner, \
    struct isocket *socket)
{
    ioc->owner = owner;
    ioc->socket = socket;
    
    socket->ioc = ioc;

    ioc->socket_event = NULL;
    ioc->auto_reconn = false;
    ioc->last_use_time = time(NULL);
    ioc->inuse = false;
    ioc->state = IOC_UNCONNECTED;
    INIT_LIST_HEAD(ioc->list);

    return true;
}

bool iocomponent_destroy(struct iocomponent *ioc)
{
    if (ioc->socket)
    {
        isock_destroy(ioc->socket);
        ioc->socket = NULL;
    }
}

void iocomponent_enable_write(struct iocomponent *ioc, bool write_on)
{
    if (ioc->socket_event != NULL)
    {
        epoll_set_event(ioc->socket_event, ioc->socket, true, write_on);
    }
}

