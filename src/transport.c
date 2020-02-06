
#include "buffer.h"
#include "iocomponent.h"
#include "tcp.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


static void* transport_event_loop(void *args);
static void* transport_timeout_loop(void *args);


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
    pthread_join(trans->event_loop_worker);
    pthread_join(trans->timeout_worker);

    transport_destroy(trans);
    return true;
}

static void* transport_event_loop(void *args)
{
    struct transport *trans;
    struct epoll_socket_event *socket_event;
    struct ioevent events[MAX_SOCKET_EVENTS] = {0};
    int timeout = 3;

    trans = (struct transport *)args;
    socket_event = &trans->sock_event;

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

            iocomponent_add_ref(ioc);

            bool rc = true;
            if (events[i].read_occurred)
            {
                // 读事件
                ioc->handle_read_event(ioc);
            }

            if (events[i].write_occurred)
            {
                // 写事件
                ioc->handle_wirte_event(ioc);
            }

            iocomponent_sub_ref(ioc);

            if (!rc)
            {
                transport_remove_component(trans, ioc);
            }
        }
    }

    return NULL;
}

{
static void* transport_timeout_loop(void *args)
    struct transport *trans = (struct transport *)args;
    struct iocomponent *my_del_head = NULL;
    struct iocomponent *my_del_tail = NULL;

    struct iocomponent *my_list = NULL;

    while (!trans->stop)
    {
        // 检测IOC是否超时
        pthread_mutex_lock(&trans->ioc_mutex);
        if (trans->ioc_list_changed)
        {
            /**
             * TODO: copy trans->ioc_list_head to my list
             */
            trans->ioc_list_changed = false;
        }

        // 加入到my_del
        if (trans->my_del_head != NULL && trans->my_del_tail != NULL)
        {
            if (my_del_tail == NULL)
            {
                my_del_head = trans->del_list_head;
            }
            else
            {
                my_del_tail->next = trans->del_list_head;
                trans->del_list_head->prev = my_del_tail;
            }
            my_del_tail = trans->del_list_tail;

            // 清空 my_del_list
            trans->my_del_head = trans->my_del_tail = NULL;
        }

        pthread_mutex_unlock(&trans->ioc_mutex);

        // 检查所有iocomponent
        while (my_list != NULL)
        {
            my_list->check_timeout(my_list, get_time());
            my_list = my_list->next;
        }

        // 删除无用的IOC
        struct iocomponent *tmp_list = my_del_head;
        int64_t now_time = get_time() - (int64_t)900000000; // 15min
        while (tmp_list != NULL)
        {
            if (iocomponent_get_ref(tmp_list) <= 0)
            {
                iocomponent_sub_ref(tmp_list);
            }

            if (iocomponent_get_ref(tmp_list) <= -10 || \
                tmp_list->last_use_time < now_time)
            {
                // 从链中删除
                if (tmp_list == my_del_head)
                {
                    my_del_head = tmp_list->next;
                }

                if (tmp_list == my_del_tail)
                {
                    my_del_tail = tmp_list->prev;
                }

                if (tmp_list->prev != NULL)
                {
                    tmp_list->prev->next = tmp_list->next;
                }
                if (tmp_list->next != NULL)
                {
                    tmp_list->next->prev = tmp_list->prev;
                }

                struct iocomponent *ioc = tmp_list;
                LOG(LOG_LEVEL_INFO, "DELIOC, %s, IOCount:%d, IOC:%p", \
                    isocket_get_address_str(ioc->socket), trans->ioc_count, ioc);

                iocomponent_del(ioc);
            }

            tmp_list = tmp_list->next;
        }

        usleep(500 * 1000); // 500ms检测一次
    }

    pthread_mutex_lock(&trans->ioc_mutex);
    if (my_del_head != NULL)
    {
        if (trans->del_list_tail == NULL)
        {
            trans->del_list_head = my_del_head;
        }
        else
        {
            trans->del_list_tail->next = my_del_head;
            my_del_head->prev = trans->del_list_tail;
        }
        trans->del_list_tail = my_deal_tail;
    }
    pthread_mutex_unlock(&trans->ioc_mutex);

    return NULL;
}

int create_thread_worker(pthread_t *tid, void* (*run)(void *args), void *args)
{
    int result;
    pthread_attr_t attr;

    if ((result = pthread_attr_init(&attr)) != 0)
    {
        return result;
    }

    if ((result = pthread_attr_setdetachstate(&attr, \
        PTHREAD_CREATE_DETACHED)) != 0)
    {
        pthread_attr_destroy(&attr);
        return result;
    }

    if ((result = pthread_create(tid, &attr, run, args)) != 0)
    {
        pthread_attr_destroy(&attr);
        return result;
    }

    pthread_attr_destroy(&attr);
    return result;
}

/**
 * 线程函数
 */
bool transport_start(struct transport *trans)
{
    int result;

    // 启动读写线程
    result = create_thread_worker(&trans->event_loop_worker, \
        transport_event_loop, trans);
    if (result != 0)
    {
        LOG(LOG_LEVEL_ERROR, "create event loop worker failed, error: %d", errno);
        return false;
    }

    // 启动超时检查线程
    result = create_thread_worker(&trans->timeout_worker, \
        transport_timeout_loop, trans);
    if (result != 0)
    {
        LOG(LOG_LEVEL_ERROR, "create timeout worker failed, error: %d", errno);
        return false;
    }

    return true;
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

    // list_add_tail(&ioc->list, &trans->ioc_list);
    ioc->prev = trans->ioc_list_tail;
    ioc->next = NULL;
    if (trans->ioc_list_tail == NULL)
    {
        trans->ioc_list_head = ioc;
    }
    else
    {
        trans->ioc_list_tail->next = ioc;
    }
    trans->ioc_list_tail = ioc;

    ioc->inuse = true;
    trans->ioc_list_changed = true;
    trans->ioc_count++;
    pthread_mutex_unlock(&trans->ioc_mutex);

    struct isocket *socket = ioc->socket;
    ioc->socket_event = &trans->sock_event;
    epoll_add_event(&trans->sock_event, socket, read_on, write_on);
    LOG(LOG_LEVEL_INFO, "ADDIOC, SOCK: %d, %s, RON: %d, WON: %d, IOCOUNT: %d, IOC: %p", \
        isocket_get_handle(ioc->socket), isocket_get_address_str(ioc->socket), \
        read_on, write_on, trans->ioc_count, ioc);
}

void transport_remove_component(struct transport *trans, struct iocomponent *ioc)
{
    pthread_mutex_lock(&trans->ioc_mutex);
    ioc->close(ioc);
    if (ioc->auto_reconn)
    {
        pthread_mutex_unlock(&trans->ioc_mutex);
        return;
    }

    if (ioc->inuse == false)
    {
        pthread_mutex_unlock(&trans->ioc_mutex);
        return;
    }

    // list_del(&ioc->list);
    if (ioc == trans->ioc_list_head)
    {
        trans->ioc_list_head = ioc->next;
    }
    if (ioc == trans->ioc_list_tail)
    {
        trans->ioc_list_tail = ioc->prev;
    }

    if (ioc->prev != NULL)
    {
        ioc->prev->next = ioc->next;
    }
    if (ioc->next != NULL)
    {
        ioc->next->prev = ioc->prev;
    }

    ioc->prev = trans->del_list_tail;
    ioc->next = NULL;
    if (trans->del_list_tail == NULL)
    {
        trans->del_list_head = ioc;
    }
    else
    {
        trans->del_list_tail->next = ioc;
    }
    trans->del_list_tail = ioc;

    trans->ioc_count--;
    trans->ioc_list_changed = true;

    pthread_mutex_unlock(&trans->ioc_mutex);
}

static int parse_addr(char *src, char **args, int cnt)
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

