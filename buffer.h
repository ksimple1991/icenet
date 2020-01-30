#ifndef ICENET_DATA_BUFFER_H
#define ICENET_DATA_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define MAX_BUFFER_SIZE 2048


/**
 *              读取位置       写入位置
 *                 |             |
 * buffer: |-------++++++++++++++-------|
 *        pstart  pdata        pfree   pend
 */

typedef struct packet_buffer
{
    unsigned char *pstart; // buffer起始地址
    unsigned char *pend; // buffer结束地址
    unsigned char *pfree; // buffer第一个空闲地址
    unsigned char *pdata; // buffer第一个数据地址
} packet_buffer_t;

bool packet_buffer_init(struct packet_buffer *buf);

void packet_buffer_destroy(struct packet_buffer *buf);

int32_t packet_buffer_size(struct packet_buffer *buf);

char *packet_buffer_get_data(struct packet_buffer *buf);

int32_t packet_buffer_get_data_len(struct packet_buffer *buf);

char *packet_buffer_get_free(struct packet_buffer *buf);

int32_t packet_buffer_get_free_len(struct packet_buffer *buf);

/**
 * 从 @buf 中数据头部开始，丢弃 @len 字节数据
 */
void packet_buffer_drain_data(struct packet_buffer *buf, int32_t len);

/**
 * 向 @buf 中写入 @len 字节空数据
 */
void packet_buffer_pour_data(struct packet_buffer *buf, int32_t len);

/**
 * 从 @buf 中数据尾部开始，去掉 @len 字节的数据
 */
void packet_buffer_strip_data(struct packet_buffer *buf, int32_t len);

/**
 * 清空 @buf 中有效数据
 */
void packet_buffer_clear(struct packet_buffer *buf);

/**
 * 确保有 @len 的剩余空间
 */
bool packet_buffer_ensure_free(struct packet_buffer *buf, int32_t len);

/**
 * 收缩 @buf 大小
 * NOTE: 写入数据后 @buf 大小会扩充，如果需要减少 @buf 大小可以使用该接口
 */
void packet_buffer_shrink(struct packet_buffer *buf);

/**
 * write系列接口用于向packet_buffer中写入特定类型数据
 */
void packet_buffer_write_int8(struct packet_buffer *buf, uint8_t n);

void packet_buffer_write_int16(struct packet_buffer *buf, int16_t n);

void packet_buffer_write_int32_t(struct packet_buffer *buf, uint32_t n);

void packet_buffer_write_int64(struct packet_buffer *buf, uint64_t n);

void packet_buffer_write_bytes(struct packet_buffer *buf, const void *src, int32_t len);

void packet_buffer_write_string(struct packet_buffer *buf, const char *str);

void packet_buffer_write_array32(struct packet_buffer *buf, uint32_t array[], int32_t size);

/**
 * read接口用于从packet_buffer中读取特定类型数据
 */
uint8_t packet_buffer_read_int8(struct packet_buffer *buf);

uint16_t packet_buffer_read_int16(struct packet_buffer *buf);

uint32_t packet_buffer_read_int32_t(struct packet_buffer *buf);

uint64_t packet_buffer_read_int64(struct packet_buffer *buf);

bool packet_buffer_read_bytes(struct packet_buffer *buf, void *dst, int32_t len);

bool packet_buffer_read_string(struct packet_buffer *buf, char **str, int32_t len);

bool packet_buffer_read_array32(struct packet_buffer *buf, uint32_t **array_out, int32_t *array_size);

/**
 * packet_write_ 接口用于向指定位置写入数据
 */

static inline void packet_write_int8(uint8_t *buf, uint8_t n)
{
    buf[0] = (uint8_t)n;
}

static inline void packet_write_int16(uint8_t *buf, uint16_t n)
{
    buf[1] = (uint8_t)n;
    n >>= 8;
    buf[0] = (uint8_t)n;
}

static inline void packet_write_int32_t(uint8_t *buf, uint32_t n)
{
    buf[3] = (uint8_t)n;
    n >>= 8;
    buf[2] = (uint8_t)n;
    n >>= 8;
    buf[1] = (uint8_t)n;
    n >>= 8;
    buf[0] = (uint8_t)n;
}

static inline void packet_write_int64(uint8_t *buf, uint64_t n)
{
    buf[7] = (uint8_t)n;
    n >>= 8;
    buf[6] = (uint8_t)n;
    n >>= 8;
    buf[5] = (uint8_t)n;
    n >>= 8;
    buf[4] = (uint8_t)n;
    n >>= 8;
    buf[3] = (uint8_t)n;
    n >>= 8;
    buf[2] = (uint8_t)n;
    n >>= 8;
    buf[1] = (uint8_t)n;
    n >>= 8;
    buf[0] = (uint8_t)n;
}

static inline uint8_t packet_read_int8(uint8_t *buf)
{
    return buf[0];
}

static inline int16_t packet_read_int16(uint8_t *buf)
{
    int16_t num;

    num = buf[0];
    num <<= 8;
    num |= buf[1];
    return num;
}

static inline uint32_t packet_read_int32_t(uint8_t *buf)
{
    uint32_t num;

    num = buf[0];
    num <<= 8;
    num |= buf[1];
    num <<= 8;
    num |= buf[2];
    num <<= 8;
    num |= buf[3];

    return num;
}

static inline uint64_t packet_read_int64(uint8_t *buf)
{
    uint64_t num;

    num = buf[0];
    num <<= 8;
    num |= buf[1];
    num <<= 8;
    num |= buf[2];
    num <<= 8;
    num |= buf[3];
    num <<= 8;
    num |= buf[4];
    num <<= 8;
    num |= buf[5];
    num <<= 8;
    num |= buf[6];
    num <<= 8;
    num |= buf[7];

    return num;
}

#ifdef __cplusplus
} 
#endif // __cplusplus

#endif /* ICENET_DATA_BUFFER_H */
