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

bool transport_wait(struct transport *trans);

void transport_run(struct transport *trans);

bool transport_stop(struct transport *trans);

/**
 * 起一个监听端口
 * 
 * @spec: 格式 [udp|tcp]:ip:port
 * 
 * @return IO组件的指针
 */
struct iocomponent* transport_listen(struct transport *trans, \
    const char *spec);

/**
 * 创建一个Connection，连接到指定地址，并加入到Socket的监听事件中
 * 
 * @spec: 格式 [udp|tcp]:ip:port
 * 
 * @return 返回connection对象指针
 */
struct connection* transport_connect(struct transport *trans, \
    const char *spec, bool auto_reconn);

void transport_disconnect(struct transport *trans, struct connection *connection);

void transport_add_component(struct transport *trans, \
    struct iocomponent *ioc, bool read_on, bool write_on);

void transport_event_loop(struct transport *trans, \
    struct epoll_socket_event *socket_event);

void transport_timeout_loop(struct transport *trans);


#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_H */
