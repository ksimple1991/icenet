#include "iocomponent.h"
#include "isocket.h"
#include "util.h"
#include "transport.h"

bool tcp_acceptor_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server);
void tcp_acceptor_close(struct iocomponent *ioc);
bool tcp_acceptor_handle_read(struct iocomponent *ioc);
void tcp_acceptor_check_timeout(struct iocomonent *ioc, int64_t now);

bool tcp_component_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server);
void tcp_component_close(struct iocomponent *ioc);
bool tcp_component_handle_write(struct iocomponent *ioc);
bool tcp_component_handle_read(struct iocomponent *ioc);
void tcp_component_check_timeout(struct iocomponent *ioc, int64_t now);

// struct iocomponent acceptor_component = 
// {
//     .list = NULL,
//     .owner = NULL,
//     .socket = NULL,
//     .socket_event = NULL,
//     .state = IOC_UNCONNECTED,
//     .auto_reconn = false,
//     .inuse = false,
//     .last_use_time = 0,
//     .buffer = NULL,
//     .init = tcp_acceptor_init,
//     .close = tcp_acceptor_close,
//     .handle_write_event = NULL,
//     .handle_read_event = tcp_acceptor_handle_read,
//     .check_timeout = tcp_acceptor_check_timeout
// };

bool tcp_acceptor_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server)
{
    bool rc;

    rc = iocomponent_init(ioc, owner, socket);
    if (rc == true)
    {
        isocket_set_blocking(socket, false);
        rc = isocket_server_listen(socket);
    }

    ioc->type = IOC_TYPE_ACCEPTOR;
    ioc->is_server = is_server;

    ioc->init = tcp_acceptor_init;
    ioc->close = tcp_acceptor_close;
    ioc->handle_write_event = NULL;
    ioc->handle_read_event = tcp_acceptor_handle_read;
    ioc->check_timeout = tcp_acceptor_check_timeout;

    return rc;
}

void tcp_acceptor_close(struct iocomponent *ioc)
{

}

bool tcp_acceptor_handle_read(struct iocomponent *ioc)
{
    struct isocket *socket = isocket_server_accept(ioc->socket);
    if (socket == NULL)
    {
        return false;
    }

    struct iocomponent *tcp_component \
        = (struct iocomponent *)malloc(sizeof(struct iocomponent));
    if (tcp_component == NULL)
    {
        isocket_del(socket);
        return false;
    }

    // 创建 tcp ioc 加入到 trans
    transport_add_component(ioc->owner, tcp_component, true, false);
    return true;
}

void tcp_acceptor_check_timeout(struct iocomponent *ioc, int64_t now)
{
    return false;
}

struct tcp_component_data
{
    int64_t start_connect_time;
};

bool tcp_component_init(struct iocomponent *ioc, struct transport *owner, \
        struct socket *socket, bool is_server)
{
    struct isocket *socket = ioc->socket;

    isocket_set_blocking(socket, false);
    isocket_set_reuse_address(socket, true);
    isocket_set_keep_alive(socket, true);
    isocket_set_int_option(socket, SO_SNDBUF, 640000);
    isocket_set_int_option(socket, SO_RCVBUF, 640000);

    if (is_server)
    {
        if (!isocket_connect(socket) && ioc->auto_reconn == false)
        {
            return false;
        }
    }
    else
    {
        ioc->state = IOC_CONNECTED;
    }
    
    ioc->is_server = is_server;

    return true;
}

void tcp_component_close(struct iocomponent *ioc)
{

}

bool tcp_component_handle_read(struct iocomponent *ioc)
{
    ioc->last_use_time = get_time();

    bool rc = false;
    if (ioc->state == IOC_CONNECTED)
    {
        // read
    }

    return rc;
}

bool tcp_component_handle_write(struct iocomponent *ioc)
{
    ioc->last_use_time = get_time();
    bool rc = true;

    if (ioc->state == IOC_CONNECTED)
    {
        // rc = write_data
    }
    else if (ioc->state == IOC_CONNECTING)
    {
        int error = isocket_get_so_error(ioc->socket);
        if (error == 0)
        {
            iocomponent_enable_write(ioc, true);
            ioc->state = IOC_CONNECTED;
        }
        else
        {
            if (ioc->socket_event)
            {
                epoll_remove_event(ioc->socket_event, ioc->socket);
            }
            isocket_close(ioc->socket);
            ioc->state = IOC_CLOSED;
        }
    }
    return true;
}

void tcp_component_check_timeout(struct iocomponent *ioc, int64_t now)
{
    if (ioc->state == IOC_CONNECTING)
    {
        struct tcp_component_data *data = (struct tcp_component_data *)ioc->extra;

        // 连接超时2秒
        if (data->start_connect_time > 0 && \
            data->start_connect_time < (now - (int64_t)2000000))
        {
            ioc->state = IOC_CLOSED;
            LOG(LOG_LEVEL_ERROR, "connect to %s timeout", isocket_get_address_str(ioc->socket));
            isocket_shutdown(ioc->socket);
        }
    }
    else if (ioc->state == IOC_CONNECTED && ioc->is_server == true \
        && ioc->auto_reconn == false)
    {
        int64_t idle = now - ioc->last_use_time;
        
        // 空闲15分钟断开
        if (idle > (int64_t)900000000)
        {
            ioc->state = IOC_CLOSED;
            LOG(LOG_LEVEL_ERROR, "%s idle %d seconds, has been disconnected", \
                isocket_get_address_str(ioc->socket), (int)(idle / 1000000));
            isocket_shutdown(ioc->socket);
        }
    }

    // connection check timeout
}


