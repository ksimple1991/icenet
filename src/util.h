#ifndef ICENET_UTIL_H
#define ICENET_UTIL_H

#include "share_func.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

#define LOG(level, _fmt_, args...) printf("[%s:%d] " _fmt_ "\n", __FUNCTION__, __LINE__, ##args)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ICENET_MAX_TIME (INT64_MAX)

typedef union user_data
{
    void *ptr;
    int32_t i32;
    int64_t i64;
    uint32_t u32;
    uint64_t u64;
} user_data_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ICENET_UTIL_H */

