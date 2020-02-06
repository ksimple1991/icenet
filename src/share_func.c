
#include "share_func.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#define RET_ERR(error) (-(error))

bool dio_file_exists(const char *file)
{
    return access(file, F_OK) == 0 ? true : false;
}

ssize_t dio_safe_write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    ssize_t remain;
    ssize_t written;
    const char *pos;

    n = write(fd, buf, count);
    if (n < 0)
    {
        if (errno != EINTR)
        {
            return -1;
        }
    }
    else if (n == (ssize_t)count)
    {
        return count;
    }

    pos = ((char *)buf + n);
    remain = count - n;
    while (remain > 0)
    {
        n = write(fd, pos, remain);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            written = count - remain;
            return written > 0 ? written : -1;
        }

        pos += n;
        remain -= n;
    }

    return count;
}

ssize_t dio_safe_read(int fd, char *buf, size_t count)
{
    ssize_t n;
    ssize_t remain;
    ssize_t read_bytes;
    char *pos;

    n = read(fd, buf, count);
    if (n < 0)
    {
        if (errno != EINTR)
        {
            return -1;
        }
    }
    else
    {
        if (n == 0 || n == (ssize_t)count)
        {
            return n;
        }
    }

    pos = buf + n;
    remain = count - n;
    while (remain > 0)
    {
        n = read(fd, pos, remain);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            read_bytes = count - remain;
            return read_bytes > 0 ? read_bytes : -1;
        }
        else if (n == 0)
        {
            break;
        }

        pos += n;
        remain -= n;
    }

    return count - remain;
}

int init_pthread_lock(pthread_mutex_t *lock)
{
    pthread_mutexattr_init mat;
    int result;

    result = pthread_mutexattr_init(&mat);
    if (result != 0)
    {
        return result;
    }

    result = pthread_mutexattr_settype(&mat, PTHREAD_MUTEX_ERRORCHECK);
    if (result != 0)
    {
        pthread_mutexattr_destroy(&mat);
        return result;
    }

    result = pthread_mutex_init(lock, &mat);
    if (result != 0)
    {
        pthread_mutexattr_destroy(&mat);
        return result;
    }

    result = pthread_mutexattr_destroy(&mat);
    if (result != 0)
    {
        return result;
    }

    return 0;
}

int init_pthread_attr(pthread_attr_t *attr)
{
    int result;

    if ((result = pthread_attr_init(attr)) != 0)
    {
        return result;
    }

    if ((result = pthread_attr_setdetachstate(attr, \
        PTHREAD_CREATE_DETACHED)) != 0)
    {
        pthread_attr_destroy(attr);
        return result;
    }

    return 0;
}

int create_work_threads(int *count, \
    void *(*start_func)(void *), void *arg, pthread_t *tids)
{
    int result;
    pthread_attr_t attr;
    pthread_t *ptid;
    pthread_t *ptid_end;

    if ((result = init_pthread_attr(&attr)) != 0)
    {
        return result;
    }

    ptid = tids;
    ptid_end = tids + *count;

    for (ptid = tids; ptid < ptid_end; ++ptid)
    {
        if ((result = pthread_create(ptid, &attr, start_func, arg)) != 0)
        {
            *count = ptid - tids;
            break;
        }
    }

    pthread_attr_destroy(&attr);
    return result;
}

int64_t get_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((int64_t)t.tv_sec * (int64_t)1000000 + (int64_t)t.tv_usec);
}
