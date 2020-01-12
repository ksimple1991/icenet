
#include "csv_packet_encode.h"
#include "iocomponent.h"
#include "tcp.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

void transport_add_component(struct transport *trans, \
    struct iocomponent *ioc, bool read_on, bool write_on);

void transport_remove_component(struct transport *trans, struct iocomponent *ioc);

void *start_func(void *arg)
{
    return NULL;
}

int transport_init(struct transport *trans)
{
    int result;

    result = init_pthread_lock(&trans->ioc_mutex);
    if (result != 0)
    {
        return result;
    }

    trans->stop = false;
    trans->ep_fd = -1;
    trans->threads_num = 0;
    trans->worker_tids = NULL;
    
    INIT_LIST_HEAD(&trans->ioc_list);
    trans->ioc_list_changed = false;
    return true;
}

bool transport_destroy(struct transport *trans)
{
    if (trans == NULL)
        return;

    struct iocomponent *ioc = NULL;
    struct iocomponent *next = NULL;
    list_for_each_entry_safe(ioc, next, &trans->ioc_list, list)
    {
        list_del(ioc);
        // 清理IO组件，释放输入输出空间
    }

    pthread_mutex_destroy(&trans->ioc_mutex);
    trans->ioc_list_changed = false;

    if (trans->ep_fd != -1)
    {
        close(trans->ep_fd);
        trans->ep_fd = -1;
    }

    if (trans->worker_tids != NULL)
    {
        free(trans->worker_tids);
        trans->worker_tids = NULL;
    }
}

bool transport_start(struct transport *trans)
{
    int result;
    pthread_t *tids;
    int threads_num = 5;

    signal(SIGPIPE, SIG_IGN);

    tids = malloc(sizeof(pthread_t) * threads_num);
    if (tids == NULL)
    {
        return false;
    }

    if ((result = create_work_threads(&threads_num, start_func, \
        (void *)trans, tids)) != 0)
    {
        free(tids);
        return false;
    }

    return true;
}

/**
 * 停止读写线程
 * 
 * @return true - 成功，false - 失败。
 */
bool transport_stop(struct transport *trans)
{
    trans->stop = true;
}

/**
 * 等待线程完全退出。
 *
 * @return true - 成功，false - 失败。
 */
bool transport_wait(struct transport *trans)
{
    pthread_t *ptid;
    pthread_t *ptid_end = trans->worker_tids + trans->threads_num;

    for (ptid = trans->worker_tids; ptid < ptid_end; ++ptid)
    {
        pthread_join(*trans->ptid, NULL);
    }
    transport_destroy(trans);
    return true;
}

void transport_event_loop(struct transport *trans, \
    struct epoll_socket_event *socket_event)
{
    struct ioevent events[MAX_SOCKET_EVENTS] = {0};
    int timeout = 3;
    int result;

    while (!trans->stop)
    {
        int cnt = epoll_get_events(socket_event, 10, events, MAX_SOCKET_EVENTS);
        if (cnt < 0)
        {
            continue;
        }

        for (int i = 0; i < cnt; i++)
        {
            struct iocomponent *ioc = events[i].ioc;
            if (ioc == NULL)
            {
                continue;
            }

            if (events[i].error_occurred)
            {
                transport_remove_component(trans, ioc);
                continue;
            }

            bool rc = true;

            if (events[i].read_occurred)
            {
                // 读事件
                ioc->
            }

            if (events[i].write_occurred)
            {
                // 写事件
            }

            if (!rc)
            {
                transport_remove_component(trans, ioc);
            }
        }
    }
}

void transport_timeout_loop(struct transport *trans)
{
    while (!trans->stop)
    {
        // 检测IOC是否超时

        // 删除无用的IOC

        usleep(500 * 1000); // 500ms检测一次 
    }
}

/**
 * 线程函数
 */
void transport_run(struct transport *trans)
{
    

    // 启动读写线程
    transport_event_loop(trans);

    // 启动超时检查线程
    transport_timeout_loop(trans);

}

void transport_add_component(struct transport *trans, \
    struct iocomponent *ioc, bool read_on, bool write_on)
{
    pthread_mutex_lock(&trans->ioc_mutex);
    if (ioc->inuse)
    {
        pthread_mutex_unlock(&trans->ioc_mutex);
        return;
    }

    list_add_tail(&ioc->list, &trans->ioc_list);

    ioc->inuse = true;
    trans->ioc_list_changed = true;
    trans->ioc_count++;
    pthread_mutex_unlock(&trans->ioc_mutex);

    struct isocket *socket = ioc->socket;
    ioc->socket_event = &trans->sock_event;
    epoll_add_event(&trans->sock_event, socket, read_on, write_on);

}

void transport_remove_component(struct transport *trans, struct iocomponent *ioc)
{
    pthread_mutex_lock(&trans->ioc_mutex);

    /**
     * TODO: 释放ioc
     */
    // iocomponent_destroy(ioc);

    if (ioc->auto_reconn)
    {
        return;
    }

    if (ioc->inuse == false)
    {
        return;
    }

    list_del(&ioc->list);

    pthread_mutex_unlock(&trans->ioc_mutex);

    trans->ioc_count--;
    trans->ioc_list_changed = true;
}

static bool parse_addr(char *src, char **args, int cnt)
{
    int index = 0;
    char *prev = src;

    while (*src)
    {
        if (*src = ':')
        {
            *src = '\0';
            args[index++] = prev;
        
            if (inex > cnt)
            {
                return index;
            }

            prev = src + 1;
        }

        src++;
    }

    args[index++] = prev;
    return index;
}

struct connection* transport_listen(struct transport *trans, const char *spec)
{
    char tmp[1024];
    char *args[32] = {NULL};
    strncpy(tmp, spec, 1024);
    tmp[1023] = '\0';

    if (parse_addr(tmp, args, 32) != 3)
    {
        return NULL;
    }

    if (strcasecmp(args[0], "tcp") == 0)
    {
        char *host = args[1];
        int port = atoi(args[2]);

        struct isocket *socket = isocket_new();
        if (socket == NULL)
        {
            return NULL;
        }

        if (!isocket_set_address(socket, host, port))
        {
            isocket_del(socket);
            return NULL;
        }

        struct iocomponent *acceptor = iocomponent_new();
        if (acceptor == NULL)
        {
            isocket_del(socket);
            return NULL;
        }

        if (!tcp_acceptor_init(acceptor, trans, socket, true))
        {
            iocomponent_del(acceptor);
            return NULL;
        }

        transport_add_component(acceptor, true, false);
        return acceptor;
    }
    else if (strcasecmp(args[0], "udp") == 0)
    {
        /* do nothing */
    }

    return NULL;
}

struct connection* transport_connect(struct transport *trans, \
    const char *spec, bool auto_reconn)
{
    char tmp[1024];
    char *args[32] = {NULL};
    strncpy(tmp, spec, 1024);
    tmp[1023] = '\0';

    if (parse_addr(tmp, args, 32) != 3)
    {
        return NULL;
    }

    if (strcasecmp(args[0], "tcp") == 0)
    {
        char *host = args[1];
        int port = atoi(args[2]);

        struct isocket *socket = isocket_new();
        if (socket == NULL)
        {
            return NULL;
        }

        if (!isocket_set_address(socket, host, port))
        {
            isocket_del(socket);
            return NULL;
        }

        struct iocomponent *component = iocomponent_new();
        if (component == NULL)
        {
            isocket_del(socket);
            return NULL;
        }

        component->auto_reconn = true;
        if (!tcp_component_init(component, trans, socket, false))
        {
            iocomponent_del(component);
            LOG(LOG_LEVEL_ERROR, "init fail, host: %s, port: %d", host, port);
            return NULL;
        }

        transport_add_component(trans, component, true, true);

        return component;
    }
    else if (strcasecmp(args[0], "udp"))
    {
        /* do nothing */
    }

    return NULL;
}

bool transport_disconnect(struct transport *trans, struct connection *connection)
{
    struct iocomponent *ioc;

    if (connection == NULL)
    {
        return false;
    }

    ioc = connection->ioc;
    ioc->auto_reconn = false;

    if (ioc->socket)
    {
        isocket_shutdown(ioc->socket);
    }

    return true;
}
