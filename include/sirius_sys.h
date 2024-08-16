/********************************************************************************************
 * @name    sirius_internal_sys.h
 * 
 * @author  胡益华
 * 
 * @date    2024-07-30
 * 
 * @brief   标准库、系统库头文件
********************************************************************************************/

#ifndef __SIRIUS_INTERNAL_SYS_H__
#define __SIRIUS_INTERNAL_SYS_H__

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/syscall.h>

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#if (__GNUC__ >= 5) || \
    (__GNUC__ == 4 && __GNUC_MINOR__ >= 9) || \
    (__GNUC__ == 4 && __GNUC_MINOR__ == 9 && __GNUC_PATCHLEVEL__ >= 0)
#include <stdatomic.h>
#else
#error "The GCC version should be 4.9.0 or higher"
#endif  // GUNC version
#endif  // GUNC macro definitions

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>     // 处理函数接收可变参数
#include <limits.h>     // 针对整形类型的极值定义
#include <float.h>
#include <errno.h>
        // __linux__
#else
#error "The current operating system is not supported"
#endif

#endif  // __SIRIUS_INTERNAL_SYS_H__
