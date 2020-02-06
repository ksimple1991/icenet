#ifndef ICENET_ISOCKET_H
#define ICENET_ISOCKET_H

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iocomponent;

typedef struct isocket
{
    int fd;
    // struct sockaddr address;
    struct sockaddr_in address;
    struct iocomponent *ioc;
} isocket_t;

struct isocket* isocket_new();

void isocket_del(struct isocket *socket);

bool isocket_setup(struct isocket *socket, int sockfd, struct sockaddr *host_address);

bool isocket_set_address(struct isocket *socket, const char *address, const int port);

const char *isocket_get_address_str(struct isocket *socket);

void isocket_close(struct isocketet *socket);

void isocket_shutdown(struct isocket *socket);

bool isocket_create_udp(struct isocket *socket);

int isocket_get_handle(struct isocket *socket);

uint64_t isocket_get_peer_id(struct isocket *socket);

int isocket_get_so_error(struct isocket *socket);

int isocket_write(struct isocket *socket, const void *data, int len);

int isocket_read(struct isocket *socket, void *data, int len);

int isocket_last_error();

bool isocket_connect(struct isocket *socket);

bool isocket_set_blocking(struct isocket *socket, bool blocking_enabled);

bool isocket_set_reuse_port(struct isocket *socket, bool on);

bool isocket_set_reuse_address(struct isocket *socket, bool on);

bool isocket_set_keep_alive(struct isocket *socket, bool on);

bool isocket_set_tcp_no_delay(struct isocket *socket, bool nodelay);

bool isocket_set_int_option(struct isocket *socket, int option, int value);

struct isocket *isocket_server_accept(struct isocket *server);

bool isocket_server_listen(struct isocket *socket);

#ifdef __cplusplus
}
#endif

#endif /* ICENET_ISOCKET_H */
