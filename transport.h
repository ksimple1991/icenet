#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "list.h"
#include "epollsocketevent.h"
#include "share_func.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct transport
{
    bool stop;
    int threads_num;
    pthread_t *worker_tids;
    struct epoll_socket_event sock_event;
    bool ioc_list_changed;
    int ioc_count;
    struct list_head ioc_list;
    pthread_mutex_t ioc_mutex;
} transport_t;

int transport_init(struct transport *trans);

bool transport_destroy(struct transport *trans);

bool transport_stop(struct transport *trans);

bool transport_wait(struct transport *trans);

void transport_event_loop(struct transport *trans, \
    struct epoll_socket_event *socket_event);

void transport_timeout_loop(struct transport *trans);

void transport_run(struct transport *trans);

void transport_add_component(struct transport *trans, \
    struct iocomponent *ioc, bool read_on, bool write_on);


#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_H */
