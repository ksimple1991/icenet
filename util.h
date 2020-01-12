#ifndef ICENET_UTIL_H
#define ICENET_UTIL_H

#include "share_func.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// #define log_notice(msg, ...) \
//     ice_log_notice(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

#define LOG(level, _fmt_, args...) printf("[%s:%ld] " _fmt_, __FUNCTION__, __LINE__, ##args)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

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

