#include "iocomponent.h"
#include "isocket.h"
#include "util.h"


struct iocomponent* iocomponent_new()
{
    struct iocomponent *ioc;

    ioc = (struct iocomponent *)malloc(sizeof(struct iocomponent));
    if (ioc != NULL)
    {
        memset(ioc, 0, sizeof(struct iocomponent));
    }

    return ioc;
}

void iocomponent_del(struct iocomponent *ioc)
{
    iocomponent_destroy(ioc);
    free(ioc);
}

bool iocomponent_init(struct iocomponent *ioc, struct transport *owner, \
    struct isocket *socket)
{
    INIT_LIST_HEAD(ioc->list);
    ioc->type = 0;

    ioc->owner = owner;
    ioc->socket = socket;

    socket->ioc = ioc;

    ioc->socket_event = NULL;
    ioc->auto_reconn = false;
    ioc->last_use_time = get_time();
    ioc->inuse = false;
    ioc->state = IOC_UNCONNECTED;
    ioc->refcount = 0;
    ioc->prev = ioc->next = NULL;
    return true;
}

bool iocomponent_destroy(struct iocomponent *ioc)
{
    if (ioc->socket)
    {
        isocket_del(ioc->socket);
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

bool iocomponent_is_connect_state(struct iocomponent *ioc)
{
    return (ioc->state == IOC_CONNECTED || ioc->state == IOC_CONNECTING);
}

int iocomponent_get_ref(struct iocomponent *ioc)
{
    return ioc->refcount;
}

void iocomponent_add_ref(struct iocomponent *ioc)
{
    ++ioc->refcount;
}

void iocomponent_sub_ref(struct iocomponent *ioc)
{
    --ioc->refcount;
}
