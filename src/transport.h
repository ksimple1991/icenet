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
    pthread_t timeout_worker;
    pthread_t event_loop_worker;
    struct epoll_socket_event sock_event;

    bool ioc_list_changed;
    int ioc_count;
    struct list_head ioc_list;
    struct iocomponent *ioc_list_head;
    struct iocomponent *ioc_list_tail;
    pthread_mutex_t ioc_mutex;

    struct iocomponent *del_list_head;
    struct iocomponent *del_list_tail;
} transport_t;

int transport_init(struct transport *trans);

bool transport_destroy(struct transport *trans);

bool transport_wait(struct transport *trans);

bool transport_start(struct transport *trans);

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

void transport_remove_component(struct transport *trans, struct iocomponent *ioc);

#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_H */
