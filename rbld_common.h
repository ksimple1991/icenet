#ifndef RBLD_COMMON_H
#define RBLD_COMMON_H

#include "csv_util.h"
#include "ice_exp.h"

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define log_debug(msg, ...) \
    ice_log_debug(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)

#define log_info(msg, ...) \
    ice_log_info(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)

#define log_error(msg, ...) \
    ice_log_error(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)

#define log_fatal(msg, ...) \
    ice_log_fatal(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)

#define log_notice(msg, ...) \
    ice_log_notice(MODULE_CSTORE, MODULE_RBLD, msg, ##__VA_ARGS__)


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

typedef struct device_info
{
    unsigned int id;
    int frag;
    int period;
} device_info_t;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #define RBLD_COMMON_H */

