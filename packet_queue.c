#include "packet_queue.h"

void packet_set_header(struct packet *packet, struct packet_header *header)
{
    if (header != NULL)
    {
        memcpy(&packet->header, header, sizeof(struct packet_header));
    }
}

struct packet_header* packet_get_header(struct packet *packet)
{
    return &packet->header;
}

bool packet_queue_init(struct packet_queue *queue)
{
    queue->head = queue->tail = NULL;
    queue->size = 0;
    return true;
}

void packet_queue_destroy(struct packet_queue *queue)
{
    packet_queue_clear(queue);
}

struct packet* packet_queue_pop(struct packet_queue *queue)
{
    if (queue->head == NULL)
    {
        return NULL;
    }

    struct packet *packet = queue->head;

    queue->head = queue->head->next;
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }
    queue->size--;    

    return packet;
}

void packet_queue_push(struct packet_queue *queue, struct packet *packet)
{
    if (packet == NULL)
    {
        return;
    }

    packet->next = NULL;
    if (queue->tail == NULL)
    {
        queue->head = packet;
    }
    else
    {
        queue->tail->next = packet;
    }
    
    queue->tail = packet;
    queue->size++;
    return;
}

void packet_queue_clear(struct packet_queue *queue)
{
    if (queue->head == NULL)
    {
        return;
    }

    while (queue->head != NULL)
    {
        struct packet *packet = queue->head;
        queue->head = packet->next;

        if (packet->free != NULL)
        {
            packet->free(packet);
        }
    }

    queue->head = queue->tail = NULL;
    queue->size = 0;
}

void packet_queue_move_to(struct packet_queue *src, struct packet_queue *dst)
{
    if (src->head == NULL)
    {
        return;
    }

    if (dst->tail == NULL)
    {
        dst->head = src->head;
    }
    else
    {
        dst->tail->next = src->head;
    }

    dst->tail = src->tail;
    dst->size += src->size;
    src->head = src->tail = NULL;
    src->size = 0;
}

bool packet_queue_empty(struct packet_queue *queue)
{
    return (queue->size == 0);
}

struct packet* packet_queue_get_timeout_list(struct packet_queue *queue, int64_t now)
{
    struct packet *list = NULL;
    struct packet *tail = NULL;

    while (queue->head != NULL)
    {
        int64_t t = queue->head->expire_time;

        if (t == 0 ||  t >= now)
            break;

        if (tail == NULL)
        {
            list = queue->head;
        }
        else
        {
            tail->next = queue->head;
        }
        tail = queue->head;

        queue->head = queue->head->next;
        if (queue->head == NULL)
        {
            queue->tail = NULL;
        }
        queue->size--;
    }

    if (tail != NULL)
    {
        tail->next = NULL;
    }
    return list;
}

