#ifndef SHARE_FUNC_H
#define SHARE_FUNC_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool dio_file_exists(const char *file);

ssize_t dio_safe_write(int fd, const void *buf, size_t count);

ssize_t dio_safe_read(int fd, char *buf, size_t count);

int init_pthread_lock(pthread_mutex_t *lock);

int create_work_threads(int *count, \
    void *(*start_func)(void *), void *arg, pthread_t *tids);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SHARE_FUNC_H */
