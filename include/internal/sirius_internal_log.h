#ifndef __SIRIUS_INTERNAL_LOG_H__
#define __SIRIUS_INTERNAL_LOG_H__

#include "../sirius_sys.h"

// 单次打印最大缓存
#define LOG_PRINT_BUFFER_SIZE   (4096)

static inline ssize_t sirius_write_fd(int fd, const void *buf, size_t count)
{
    ssize_t result;

    size_t bytes_written = 0;
    do {
        result = write(fd, &((const char *)buf)[bytes_written],
            count - bytes_written);
        if (result < 0) {
            return result;
        }
        bytes_written += result;
    } while (bytes_written < count);
    return bytes_written;
}

static inline ssize_t sirius_read_fd(int fd, void *buf, size_t count) {
    ssize_t result;

    size_t bytes_read = 0;
    do {
        result = read(fd, &((char *)buf)[bytes_read], count - bytes_read);
        if (result < 0) {
            return result;
        } else if (result == 0) {
            break;
        }
        bytes_read += result;
    } while (bytes_read < count);
    return bytes_read;
}

#endif  // __SIRIUS_INTERNAL_LOG_H__
