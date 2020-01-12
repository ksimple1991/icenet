
#include "csv_packet_encode.h"
#include "ice_list_exp.h"
#include "share_func.h"
#include "epollsocketevent.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


// typedef struct connection
// {
//     struct list_head list;
//     struct rbld_event_base *base;
//     int fd;
//     char address[16];
//     int status;
//     csv_packet_buffer_t input;
//     csv_packet_buffer_t output;
// } connection_t;

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

void *start_func(void *arg)
{
    return NULL;
}

int transport_init(struct transport *trans)
{
    int result;
    pthread_t *tids;
    int threads_num = 5;

    tids = malloc(sizeof(pthread_t) * threads_num);
    if (tids == NULL)
    {
        return ENOMEM;
    }

    result = init_pthread_lock(&trans->ioc_mutex);
    if (result != 0)
    {
        free(tids);
        return result;
    }

    if ((result = create_work_threads(&threads_num, start_func, \
        (void *)trans, tids)) != 0)
    {
        free(tids);
        pthread_mutex_destroy(&trans->ioc_mutex);
        return result;
    }

    trans->stop = false;
    trans->ep_fd = -1;
    trans->threads_num = threads_num;
    trans->worker_tids = tids;
    
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

void transport_run(struct transport *trans)
{
    signal(SIGPIPE, SIG_IGN);

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

