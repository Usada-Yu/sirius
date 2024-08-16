/********************************************************************************************
 * @name    sirius_log.h
 * 
 * @author  胡益华
 * 
 * @date    2024-07-30
 * 
 * @brief   日志信息打印
 * 
 * @details
 * (1)  初始化后创建管道文件 log_pipe
 * 
 * (2)  控制日志打印等级方式： echo loglevel [lv] > log_pipe。
 *      lv参考 sirius_log_level_t
********************************************************************************************/

#ifndef __SIRIUS_LOG_H__
#define __SIRIUS_LOG_H__

// 自定义模块名
// CFLAGS += -DLOG_MODULE_NAME='"$(LOG_MODULE_NAME)"'
#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME             "unknown"
#endif  // LOG_MODULE_NAME

// 日志颜色
#define LOG_NONE                    "\033[m"
#define LOG_RED                     "\033[0;32;31m"
#define LOG_GREEN                   "\033[0;32;32m"
#define LOG_BLUE                    "\033[0;32;34m"
#define LOG_DARY_GRAY               "\033[1;30m"
#define LOG_CYAN                    "\033[0;36m"
#define LOG_PURPLE                  "\033[0;35m"
#define LOG_BROWN                   "\033[0;33m"
#define LOG_YELLOW                  "\033[1;33m"
#define LOG_WHITE                   "\033[1;37m"

#define LOG_LOGLEVEL_CMD            "loglevel"              // 调整日志等级管道输入命令

#ifndef STDIN_FILENO
// 标准输入文件描述符
#define STDIN_FILENO    (0)
#endif

#ifndef STDOUT_FILENO
// 标准输出文件描述符
#define STDOUT_FILENO   (1)
#endif

#ifndef STDERR_FILENO
// 标准错误输出文件描述符
#define STDERR_FILENO   (2)
#endif

// 日志打印等级枚举
typedef enum {
    SIRIUS_LOG_LEVEL_0              = 0,                    // 关闭日志打印

    SIRIUS_LOG_LEVEL_DEFAULT        = 1,                    // 默认等级

    SIRIUS_LOG_LEVEL_ERROR          = 2,                    // 错误打印
    SIRIUS_LOG_LEVEL_WARN           = 3,                    // 警告打印
    SIRIUS_LOG_LEVEL_INFO           = 4,                    // 通用信息打印

    SIRIUS_LOG_LEVEL_DEBUG_1        = 5,                    // debug等级为1
    SIRIUS_LOG_LEVEL_DEBUG_2        = 6,                    // debug等级为2
    SIRIUS_LOG_LEVEL_DEBUG_3        = 7,                    // debug等级为3

    SIRIUS_LOG_LEVEL_MAX,
} sirius_log_level_t;

// log模块句柄
typedef struct {
    sirius_log_level_t              default_log_level;      // 默认日志打印等级
    char                            *p_pipe_path;           // 管道文件路径
} sirius_log_create_t;

/********************************************************************************************
 * @brief   日志信息打印
 * 
 * @param   log_level：打印等级
 * @param   p_color：打印色彩
 * @param   p_module：模块名
 * @param   p_fmt：打印信息
 * 
 * @return  成功：SIRIUS_OK
 *          失败：error code
********************************************************************************************/
int sirius_log_print(sirius_log_level_t log_level, const char *p_color,
                        const char *p_module, const char *p_fmt, ...);

/********************************************************************************************
 * @brief   日志模块反初始化
 * 
 * @return  成功：SIRIUS_OK
 *          失败：error code
********************************************************************************************/
int sirius_log_deinit();

/********************************************************************************************
 * @brief   日志模块初始化
 * 
 * @param   p_handle：日志模块初始化信息，模块内进行深拷贝，调用后可直接释放
 * 
 * @note    (1) 同进程内仅可初始化一次
 *          (2) 反初始化后可再次初始化
 * 
 * @return  成功：字符串长度
 *          失败：0
********************************************************************************************/
int sirius_log_init(const sirius_log_create_t *p_handle);

#ifndef SIRIUS_LOG_WRITE
#define SIRIUS_LOG_WRITE(lv, color, format, ...) \
    do { \
        sirius_log_print(lv, color, \
            LOG_MODULE_NAME, \
            format, ##__VA_ARGS__); \
    } while (0)
#endif

#ifndef SIRIUS_PRINTF
#define SIRIUS_PRINTF(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_DEFAULT, LOG_NONE, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_PRINTF

#ifndef SIRIUS_INFO
#define SIRIUS_INFO(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_INFO, LOG_GREEN, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_INFO

#ifndef SIRIUS_WARN
#define SIRIUS_WARN(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_WARN, LOG_YELLOW, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_WARN

#ifndef SIRIUS_ERROR
#define SIRIUS_ERROR(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_ERROR, LOG_RED, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_ERROR

#ifndef SIRIUS_DEBUG_1
#define SIRIUS_DEBUG_1(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_DEBUG_1, LOG_NONE, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_DEBUG_1

#ifndef SIRIUS_DEBUG_2
#define SIRIUS_DEBUG_2(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_DEBUG_2, LOG_NONE, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_DEBUG_2

#ifndef SIRIUS_DEBUG_3
#define SIRIUS_DEBUG_3(format, ...) \
    SIRIUS_LOG_WRITE(SIRIUS_LOG_LEVEL_DEBUG_3, LOG_NONE, \
        format, ##__VA_ARGS__)
#endif  // SIRIUS_DEBUG_3

#endif  // __SIRIUS_LOG_H__
