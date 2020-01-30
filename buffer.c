#include "buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


static bool packet_buffer_expand(struct packet_buffer *buf, int32_t need);

bool packet_buffer_init(struct packet_buffer *buf)
{
    buf->pstart = NULL;
    buf->pend = NULL;
    buf->pfree = NULL;
    buf->pdata = NULL;

    return true;
}

void packet_buffer_destroy(struct packet_buffer *buf)
{
    if (buf->pstart != NULL)
    {
        free(buf->pstart);
        buf->pstart = NULL;
        buf->pend = NULL;
        buf->pfree = NULL;
        buf->pdata = NULL;
    }
}

int32_t packet_buffer_size(struct packet_buffer *buf)
{
    return (buf->pend - buf->pstart);
}

char *packet_buffer_get_data(struct packet_buffer *buf)
{
    return (char *)buf->pdata;
}

int32_t packet_buffer_get_data_len(struct packet_buffer *buf)
{
    return (buf->pfree - buf->pdata);
}

char *packet_buffer_get_free(struct packet_buffer *buf)
{
    return (char *)buf->pfree;
}

int32_t packet_buffer_get_free_len(struct packet_buffer *buf)
{
    return (buf->pend - buf->pfree);
}

void packet_buffer_drain_data(struct packet_buffer *buf, int32_t len)
{
    buf->pdata += len;

    if (buf->pdata >= buf->pfree)
    {
        packet_buffer_clear(buf);
    }
}

void packet_buffer_pour_data(struct packet_buffer *buf, int32_t len)
{
    if (packet_buffer_ensure_free(buf, len))
    {
        buf->pfree += len;
    }
}

void packet_buffer_strip_data(struct packet_buffer *buf, int32_t len)
{
    assert(buf->pfree - buf->pdata >= len);
    buf->pfree -= len;
}

void packet_buffer_clear(struct packet_buffer *buf)
{
    buf->pdata = buf->pfree = buf->pstart;
}

bool packet_buffer_ensure_free(struct packet_buffer *buf, int32_t len)
{
    return packet_buffer_expand(buf, len);
}

/* 收缩 */
void packet_buffer_shrink(struct packet_buffer *buf)
{
    if (buf->pstart == NULL)
    {
        return;
    }

    if (buf->pend - buf->pstart <= MAX_BUFFER_SIZE ||
        buf->pfree - buf->pdata > MAX_BUFFER_SIZE)
    {
        return;
    }

    int32_t dlen = buf->pfree - buf->pdata;
    if (dlen < 0)
    {
        dlen = 0;
    }

    uint8_t *newbuf = (uint8_t *)malloc(MAX_BUFFER_SIZE);
    if (newbuf == NULL)
    {
        return;
    }

    if (dlen > 0)
    {
        memcpy(newbuf, buf->pdata, dlen);
    }
    free(buf->pstart);

    buf->pdata = buf->pstart = newbuf;
    buf->pfree = buf->pstart + dlen;
    buf->pend = buf->pstart + MAX_BUFFER_SIZE;

    return;
}

void packet_buffer_write_int8(struct packet_buffer *buf, uint8_t n)
{
    if (packet_buffer_expand(buf, 1))
    {
        packet_write_int8(buf->pfree, n);
        buf->pfree += 1;
    }
}

void packet_buffer_write_int16(struct packet_buffer *buf, int16_t n)
{
    if (packet_buffer_expand(buf, 2))
    {
        packet_write_int16(buf->pfree, n);
        buf->pfree += 2;
    }
}

void packet_buffer_write_int32(struct packet_buffer *buf, uint32_t n)
{
    if (packet_buffer_expand(buf, 4))
    {
        packet_write_int32(buf->pfree, n);
        buf->pfree += 4;
    }
}

void packet_buffer_write_int64(struct packet_buffer *buf, uint64_t n)
{
    if (packet_buffer_expand(buf, 8))
    {
        packet_write_int64(buf->pfree, n);
        buf->pfree += 8;
    }

}

void packet_buffer_write_bytes(struct packet_buffer *buf, const void *src, int32_t len)
{
    if (packet_buffer_expand(buf, len))
    {
        memcpy(buf->pfree, src, len);
        buf->pfree += len;
    }
}

void packet_buffer_write_string(struct packet_buffer *buf, const char *str)
{
    int32_t len = (str ? strlen(str) : 0);
    if (len > 0)
    {
        ++len; // 添加'\0'
    }

    if (packet_buffer_expand(buf, len + sizeof(uint32_t)))
    {
        packet_buffer_write_int32(buf, len);

        if (len > 0)
        {
            memcpy(buf->pfree, str, len);
            buf->pfree += len;
        }
    }
}

void packet_buffer_write_array32(struct packet_buffer *buf, uint32_t array[], int32_t size)
{
    packet_buffer_write_int32(buf, size);

    for (int32_t i = 0; i < size; ++i)
    {
        packet_buffer_write_int32(buf, array[i]);
    }
}

uint8_t packet_buffer_read_int8(struct packet_buffer *buf)
{
    return (*buf->pdata++);
}

uint16_t packet_buffer_read_int16(struct packet_buffer *buf)
{
    int16_t n = buf->pdata[0];
    n <<= 8;
    n |= buf->pdata[1];
    buf->pdata += 2;
    assert(buf->pfree >= buf->pdata);
    return n;
}

uint32_t packet_buffer_read_int32(struct packet_buffer *buf)
{
    uint32_t n = buf->pdata[0];
    n <<= 8;
    n |= buf->pdata[1];
    n <<= 8;
    n |= buf->pdata[2];
    n <<= 8;
    n |= buf->pdata[3];

    buf->pdata += 4;
    assert(buf->pfree >= buf->pdata);
    return n;
}

uint64_t packet_buffer_read_int64(struct packet_buffer *buf)
{
    uint64_t n = buf->pdata[0];
    n <<= 8;
    n |= buf->pdata[1];
    n <<= 8;
    n |= buf->pdata[2];
    n <<= 8;
    n |= buf->pdata[3];
    n <<= 8;
    n |= buf->pdata[4];
    n <<= 8;
    n |= buf->pdata[5];
    n <<= 8;
    n |= buf->pdata[6];
    n <<= 8;
    n |= buf->pdata[7];

    buf->pdata += 8;
    assert(buf->pfree >= buf->pdata);
    return n;
}

bool packet_buffer_read_bytes(struct packet_buffer *buf, void *dst, int32_t len)
{
    if (buf->pdata + len > buf->pfree)
    {
        return false;
    }

    memcpy(dst, buf->pdata, len);
    buf->pdata += len;
    assert(buf->pfree >= buf->pdata);
    return true;
}

bool packet_buffer_read_string(struct packet_buffer *buf, char **str, int32_t len)
{
    if (buf->pdata + sizeof(uint32_t) > buf->pfree)
    {
        return false;
    }

    int32_t strlen = packet_buffer_read_int32(buf);
    if (buf->pfree - buf->pdata < strlen)
    {
        strlen = buf->pfree - buf->pdata;
    }

    if (*str == NULL && strlen > 0)
    {
        *str = (char *)malloc(strlen);
        if (*str == NULL)
        {
            /* 删除string */
            buf->pdata += strlen;
            assert(buf->pfree >= buf->pdata);
            return false;
        }
        len = strlen;
    }

    if (len > strlen)
    {
        len = strlen;
    }

    if (len > 0)
    {
        memcpy(*str, buf->pdata, len);
        (*str)[len - 1] = '\0';
    }

    buf->pdata += strlen;
    assert(buf->pfree >= buf->pdata);
    return true;
}

bool packet_buffer_read_array32(struct packet_buffer *buf, uint32_t **array_out, int32_t *array_size)
{
    int32_t size = packet_buffer_read_int32(buf);

    uint32_t *out = (uint32_t *)malloc(size * sizeof(uint32_t));
    if (out == NULL)
    {
        return false;
    }
    memset(out, 0, size * sizeof(uint32_t));

    for (int32_t i = 0; i < size; ++i)
    {
        out[i] = packet_buffer_read_int32(buf);
    }

    *array_out = out;
    *array_size = size;
    return true;
}

static bool packet_buffer_expand(struct packet_buffer *buf, int32_t need)
{
    if (buf->pstart == NULL)
    {
        int32_t len = 256;
        while (len < need)
        {
            len <<= 1;
        }

        unsigned char *pdata = (unsigned char *)malloc(len);
        if (pdata == NULL)
        {
            return false;
        }

        buf->pfree = buf->pdata = buf->pstart = pdata;
        buf->pend = buf->pstart + len;

        return true;
    }

    /* 空间不够 */
    if (packet_buffer_get_free_len(buf) < need)
    {
        int32_t flen = packet_buffer_get_free_len(buf) + (buf->pdata - buf->pstart);
        int32_t dlen = buf->pfree - buf->pdata;

        if (flen < need || flen * 4 < dlen)
        {
            int32_t bufsize = (buf->pend - buf->pstart) * 2;

            while (bufsize - dlen < need)
            {
                bufsize <<= 1;
            }

            uint8_t *newbuf = (uint8_t *)malloc(bufsize);
            if (newbuf == NULL)
            {
                printf("[ERROR][%s:%d]expand data buffer failed, length: %d", __FUNCTION__, __LINE__, bufsize);
                return false;
            }

            if (dlen > 0)
            {
                memcpy(newbuf, buf->pdata, dlen);
            }

            free(buf->pstart);

            buf->pdata = buf->pstart = newbuf;
            buf->pfree = buf->pstart + dlen;
            buf->pend = buf->pstart + bufsize;
        }
        else
        {
            memmove(buf->pstart, buf->pdata, dlen);
            buf->pfree = buf->pstart + dlen;
            buf->pdata = buf->pstart;
        }
    }

    return true;
}

