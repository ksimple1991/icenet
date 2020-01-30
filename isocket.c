
#include "isocket.h"
#include "util.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ISOCKET_INVLID_FD -1

static bool isocket_check_socket(struct isocket *socket);

struct isocket* isocket_new()
{
    struct isocket *socket;
    socket = (struct isocket *)malloc(sizeof(struct iosocket));
    if (socket != NULL)
    {
        socket->fd = -1;
        memset(socket->address, 0, sizeof(socket->address));
        socket->ioc = NULL;
    }

    return socket;
}

void isocket_del(struct isocket *socket)
{
    isocket_close(socket);
    free(socket);
}

bool isocket_set_address(struct isocket *socket, const char *address, const int port)
{
    socket->address.sin_family = AF_INET;
    socket->address.sin_port = htons((uint16_t)port);

    if (address == NULL || address[0] == '\0')
    {
        socket->address.sin_addr.s_addr = htonl(INADDR_ANY);
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
            socket->address.sin_addr.s_addr = inet_addr(address);
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

/**
 * TODO: 临时凑数，非线程安全
 * add by kangjia, 2020/01/12
 */
const char *isocket_get_address_str(struct isocket *socket)
{
    static char address[16];

    memset(address, 0, sizeof(address));
    if (inet_ntop(AF_INET, socket->address.sin_addr, \
        address, sizeof(address)) == 0)
    {
        return address;
    }

    return NULL;
}

static bool isocket_check_socket(struct isocket *socket)
{
    if (socket->fd == ISOCKET_INVLID_FD)
    {
        socket->fd = socket(AF_INET, SOCKET_STREAM, 0);
        if (socket->fd == -1)
        {
            return false;
        }
    }

    return true;
}

bool isocket_connect(struct isocket *socket)
{
    if (!isocket_check_socket(socket))
    {
        return false;
    }

    return (0 == connect(socket->fd, (struct sockaddr *)&socket->addres, sizeof(socket->address)));
}

void isocket_close(struct isocketet *socket)
{
    if (socket->fd != ISOCKET_INVLID_FD)
    {
        close(socket->fd);
        socket->fd = ISOCKET_INVLID_FD;
    }
}

void isocket_shutdown(struct isocket *socket)
{
    if (socket->fd != ISOCKET_INVLID_FD)
    {
        shutdown(socket->fd, SHUT_WR);
    }
}

bool isocket_create_udp(struct isocket *socket)
{
    socket->fd = socket(AF_INET, SOCK_DGRAM, 0);

    return (socket->fd != -1);
}

bool isocket_setup(struct isocket *socket, int sockfd, struct sockaddr *host_address)
{
    isocket_close(socket);
    socket->fd = sockfd;
    memcpy(&socket->address, host_address, sizeof(socket->address));
}

int isocket_get_fd(struct isocket *socket)
{
    return socket->fd;
}

int isocket_write(struct isocket *socket, const void *data, int len)
{
    if (socket->fd == ISOCKET_INVLID_FD)
        return -1;

    int res;
    do
    {
        res = write(socket->fd, data, len);
        if (res > 0)
        {
            // COUNT_DATA_WRITE
        }
    } while (res < 0 && errno == EINTR);
    
    return res;
}

int isocket_read(struct isocket *socket, void *data, int len)
{
    if (socket->fd == ISOCKET_INVLID_FD)
        return -1;

    int res;
    do
    {
        res = read(socket->fd, data, len);
        if (res > 0)
        {
            // COUNT_DATA_READ
        }
    } while (res < 0 && errno == EINTR);
    
    return res;
}

int isocket_last_error()
{
    return errno;
}

int isocket_get_so_error(struct isocket *socket)
{
    if (socket->fd == ISOCKET_INVLID_FD)
        return EINVAL;

    int last_error = isocket_last_error();

    int socket_error = 0;
    socklen_t so_error_len = sizeof(socket_error);
    if (getsockopt(socket->fd, SOL_SOCKET, SO_ERROR, \
        (void *)(&socket_error), &so_error_len) != 0)
        return last_error;

    if (so_error_len != sizeof(socket_error))
        return EINVAL;

    return socket_error;
}

bool isocket_set_int_option(struct isocket *socket, int option, int value)
{
    bool ret_val = false;

    if (isocket_check_socket(socket))
    {
        rc = (setsockopt(socket->fd, SOL_SOCKET, option,
                         (const void *)(&value), sizeof(value)) == 0);
    }

    return ret_val;
}

bool isocket_set_time_option(struct isocket *socket, int option, int milliseconds)
{
    bool rc = false;
    if (isocket_check_socket(socket))
    {
        struct timeval timeout;
        timeout.tv_sec = (int)(milliseconds / 1000);
        timeout.tv_usec = (milliseconds % 1000) * 1000000;
        rc = (setsockopt(socket->fd, SOL_SOCKET, option, \
            (const void *)(&timeout), sizeof(timeout)) == 0);
    }
    return rc;
}

bool isocket_set_blocking(struct isocket *socket, bool blocking_enabled)
{
    bool rc = false;

    if (isocket_check_socket(socket))
    {
        int flags = fcntl(socket->fd, F_GETFL, NULL);
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

            if (fcntl(socket->fd, F_SETFL, flags) >= 0)
            {
                rc = true;
            }
        }
    }

    return rc;
}

bool isocket_set_reuse_port(struct isocket *socket, bool on)
{
    return isocket_set_int_option(socket, SO_REUSEPORT, on ? 1 : 0);
}

bool isocket_set_reuse_address(struct isocket *socket, bool on)
{
    return isocket_set_int_option(socket, SO_REUSEADDR, on ? 1 : 0);
}

bool isocket_set_keep_alive(struct isocket *socket, bool on)
{
    return isocket_set_int_option(socket, SO_KEEPALIVE, on ? 1 : 0);
}

bool isocket_set_tcp_no_delay(struct isocket *socket, bool nodelay)
{
    bool rc = false;
    int optval = nodelay ? 1 : 0;
    if (isocket_check_socket(socket))
    {
        rc = (setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, \
            (const void *)(&optval), sizeof(optval)) == 0);
    }
    return rc;
}

uint64_t isocket_get_peer_id(struct isocket *socket)
{
    if (socket->fd == ISOCKET_INVLID_FD)
        return 0;
    
    struct sockaddr_in peer;
    socklen_t length = sizeof(peer);
    if (getpeername(socket->fd, (struct sockaddr *)&peer, &length))
    {
        return ntohl(peer.sin_addr.s_addr);
    }

    return 0;
}

bool isocket_server_listen(struct isocket *socket)
{
    if (!isocket_check_socket(socket))
        return false;

    isocket_set_reuse_address(socket, true);
    isocket_set_int_option(socket, SO_SNDBUF, 640000);
    isocket_set_int_option(socket, SO_RCVBUF, 640000);

    if (bind(socket->fd, (struct sockaddr *)&socket->address, \
        sizeof(socket->address)) < 0)
    {
        return false;
    }

    #define SERVER_SOCKET_BACKLOG 256
    if (listen(socket->fd, SERVER_SOCKET_BACKLOG) < 0)
    {
        return false;
    }

    return true;
}

struct isocket *isocket_server_accept(struct isocket *server)
{
    struct sockaddr_in addr;
    int len = sizeof(addr);
    struct isocket *handle_socket = NULL;

    int fd = accept(server->fd, (struct sockaddr *)&addr, (socklen_t *)&len);
    if (fd >= 0)
    {
        handle_socket = isocket_new();
        if (handle_socket != NULL)
        {
            isocket_setup(handle_socket, fd, &addr);
        }
    }
    else
    {
        if (errno != EAGAIN)
        {
            LOG(LOG_LEVEL_ERROR, "%s(%d)", strerror(errno), errno);
        }
    }

    return handle_socket;
}

int isocket_net_ip4_addr(const char *src, unsigned int *dst)
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

int isocket_net_ip4_name(unsigned int ip, char *cp, unsigned int size)
{
    snprintf(cp, size, "%u.%u.%u.%u",
            (ip >> 24) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF,
            ip & 0xFF);
    
    return 0;
}

