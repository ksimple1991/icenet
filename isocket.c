
#include "isocket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ISOCKET_INVLID_FD -1

bool isock_init(struct isocket *socket)
{
    socket->fd = -1;
    return true;
}

void isock_destroy(struct isocket *socket)
{
    if (isock->fd != -1)
    {
        close(isock->fd);
        isock->fd = -1;
    }
}

bool isock_set_address(struct isocket *sock, const char *address, const int port)
{
    sock->address.sin_family = AF_INET;
    sock->address.sin_port = htons((uint16_t)port);

    if (address == NULL || address[0] == '\0')
    {
        sock->address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        const char *p = address;
        char c;
        bool is_ip_address = true;

        while ((c = (*p++)) != '\0')
        {
            if ((c != '.') && (!((c >= '0') && (c <= '9'))))
            {
                is_ip_address = false;
                break;
            }
        }

        if (is_ip_address)
        {
            sock->address.sin_addr.s_addr = inet_addr(address);
        }
        else
        {
            /**
             * TODO: 解析域名
             * 
             */
            return false;
        }
    }
    
    return true;
}

bool isock_check_socket(struct isocket *sock)
{
    if (sock->fd == ISOCKET_INVLID_FD)
    {
        sock->fd = socket(AF_INET, SOCKET_STREAM, 0);
        if (sock->fd == -1)
        {
            return false;
        }
    }

    return true;
}

bool isock_connect(struct isocket *sock)
{
    if (!isock_check_socket(sock))
    {
        return false;
    }

    return (0 == connect(sock->fd, (struct sockaddr *)&sock->addres, sizeof(sock->address)));
}

void isock_close(struct isocket *sock)
{
    if (sock->fd != ISOCKET_INVLID_FD)
    {
        close(sock->fd);
        sock->fd = ISOCKET_INVLID_FD;
    }
}

void isock_shutdown(struct isocket *sock)
{
    if (sock->fd != ISOCKET_INVLID_FD)
    {
        shutdown(sock->fd, SHUT_WR);
    }
}

bool isock_create_udp(struct isocket *sock)
{
    sock->fd = socket(AF_INET, SOCK_DGRAM, 0);

    return (sock->fd != -1);
}

bool isock_setup(struct isocket *sock, int sockfd, struct sockaddr *host_address)
{

}

int isock_get_fd(struct isocket *sock)
{
    return sock->fd;
}

int isock_write(struct isocket *sock, const void *data, int len)
{
    if (sock->fd == ISOCKET_INVLID_FD)
        return -1;

    int res;
    do
    {
        res = write(sock->fd, data, len);
        if (res > 0)
        {
            // COUNT_DATA_WRITE
        }
    } while (res < 0 && errno == EINTR);
    
    return res;
}

int isock_read(struct isocket *sock, void *data, int len)
{
    if (sock->fd == ISOCKET_INVLID_FD)
        return -1;

    int res;
    do
    {
        res = read(sock->fd, data, len);
        if (res > 0)
        {
            // COUNT_DATA_READ
        }
    } while (res < 0 && errno == EINTR);
    
    return res;
}

int isock_last_error()
{
    return errno;
}

int isock_get_error(struct isocket *sock)
{
    if (sock->fd == ISOCKET_INVLID_FD)
        return EINVAL;

    int last_error = errno;
    int socket_error;
    socklen_t soErrorLen = sizeof(socket_error);
    if (getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, \
        (void *)(&socket_error), &soErrorLen) != 0)
    {
        return lastError;
    }
    if (soErrorLen != sizeof(socket_error))
        return EINVAL;

    return socket_error;
}

int isock_get_so_error(struct isocket *sock)
{
    if (sock->fd == ISOCKET_INVLID_FD)
        return EINVAL;

    int last_error = isock_last_error();

    int so_error = 0;
    socklen_t so_error_len = sizeof(so_error);
    if (getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, \
        (void *)(&so_error), &so_error_len) != 0)
        return last_error;

    if (so_error_len != sizeof(so_error))
        return EINVAL;

    return so_error;
}

bool isock_set_int_option(struct isocket *sock, int option, int value)
{
    bool ret_val = false;

    if (isock_check_socket(sock))
    {
        rc = (setsockopt(sock->fd, SOL_SOCKET, option,
                         (const void *)(&value), sizeof(value)) == 0);
    }

    return ret_val;
}

bool isock_set_time_option(struct isocket *sock, int option, int milliseconds)
{
    bool rc = false;
    if (isock_check_socket(sock))
    {
        struct timeval timeout;
        timeout.tv_sec = (int)(milliseconds / 1000);
        timeout.tv_usec = (milliseconds % 1000) * 1000000;
        rc = (setsockopt(sock->fd, SOL_SOCKET, option, \
            (const void *)(&timeout), sizeof(timeout)) == 0);
    }
    return rc;
}

bool isock_set_blocking(struct isocket *sock, bool blocking_enabled)
{
    bool rc = false;

    if (isock_check_socket(sock))
    {
        int flags = fcntl(sock->fd, F_GETFL, NULL);
        if (flags >= 0)
        {
            if (blocking_enabled)
            {
                flags &= ~O_NONBLOCK; // clear nonblocking
            }
            else
            {
                flags |= O_NONBLOCK;  // set nonblocking
            }

            if (fcntl(sock->fd, F_SETFL, flags) >= 0)
            {
                rc = true;
            }
        }
    }

    return rc;
}

bool isock_set_reuse_port(struct isocket *sock, bool on)
{
    return isock_set_int_option(sock, SO_REUSEPORT, on ? 1 : 0);
}

bool isock_set_reuse_address(struct isocket *sock, bool on)
{
    return isock_set_int_option(sock, SO_REUSEADDR, on ? 1 : 0);
}

bool isock_set_keep_alive(struct isocket *sock, bool on)
{
    return isock_set_int_option(sock, SO_KEEPALIVE, on ? 1 : 0);
}

bool isock_set_tcp_no_delay(struct isocket *sock, bool nodelay)
{
    bool rc = false;
    int optval = nodelay ? 1 : 0;
    if (isock_check_socket(sock))
    {
        rc = (setsockopt(sock->fd, IPPROTO_TCP, TCP_NODELAY, \
            (const void *)(&optval), sizeof(optval)) == 0);
    }
    return rc;
}

uint64_t isock_get_peer_id(struct isocket *sock)
{
    if (sock->fd == ISOCKET_INVLID_FD)
        return 0;
    
    struct sockaddr_in peer;
    socklen_t length = sizeof(peer);
    if (getpeername(sock->fd, (struct sockaddr *)&peer, &length))
    {
        return ntohl(peer.sin_addr.s_addr);
    }

    return 0;
}

int isock_net_ip4_addr(const char *src, unsigned int *dst)
{
    struct in_addr addr = {0};
    int res = inet_pton(AF_INET, src, &addr);
    if (res == 1)
    {
        *dst = ntohl(addr.s_addr);
        return 0;
    }

    return -1;
}

int isock_net_ip4_name(unsigned int ip, char *cp, unsigned int size)
{
    snprintf(cp, size, "%u.%u.%u.%u",
            (ip >> 24) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF,
            ip & 0xFF);
    
    return 0;
}

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

// void iocomponent_close(struct iocomponent *ioc)
// {

// }

