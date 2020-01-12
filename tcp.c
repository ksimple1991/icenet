#include "transport.h"
#include "isocket.h"

struct iocomponent acceptor_ioc = 
{
    .list = NULL,
    .owner = NULL,
    .socket = NULL,
    .socket_event = NULL,
    .state = IOC_UNCONNECTED,
    .auto_reconn = false,
    .inuse = false,
    .last_use_time = 0,
    .buffer = NULL,
    .init = NULL,
    .close = NULL,
    .handle_write_event = NULL,
    .handle_read_event = NULL
};


void tcp_acceptor_init(struct iocomponent *ioc)
{

}

void tcp_acceptor_close(struct iocomponent *ioc)
{

}

bool tcp_acceptor_handle_write(struct iocomponent *ioc)
{
    /* do nothing */
    return true;
}

bool tcp_acceptor_handle_read(struct iocomponent *ioc)
{
    int fd;
    struct sockaddr_in addr;
    socklen_t socklen = sizeof(addr);

    fd = accept(ioc->socket->fd, &addr, &socklen, 0);
    if (fd < 0)
    {
        return false;
    }

    struct iocomponent *client;

    // 创建 tcp ioc 加入到 trans

    transport_add_iocomponent(ioc->trans, client, true, false);

    return true;
}

