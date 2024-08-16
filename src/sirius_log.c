/********************************************************************************************
 * @name    sirius_log.c
 * 
 * @author  胡益华
 * 
 * @date    2024-07-30
 * 
 * @brief   日志信息打印
 ********************************************************************************************/

#include "internal/sirius_internal_log.h"

#include "sirius_errno.h"
#include "sirius_log.h"
#include "sirius_macro.h"
#include "sirius_sys.h"

#define I_LOG_FIFO_BUFFER_SIZE      (1024)                                  // 单次读取管道文件内容缓存大小
#define I_LOG_FIFO_PATH_SIZE        (256)                                   // 管道文件路径长度

#define I_LOG_CMD_THREAD_STOP       "echo " LOG_LOGLEVEL_CMD " 0 > "        // 防线程阻塞命令
#define I_LOG_CMD_THREAD_STOP_SIZE  (I_LOG_FIFO_PATH_SIZE << 1)             // 停止线程的系统调用命令长度

typedef enum {
    I_LOG_THREAD_STATE_INVALID      = 0,                                    // 无效状态
    I_LOG_THREAD_STATE_EXITING      = 1,                                    // 线程需要退出
    I_LOG_THREAD_STATE_EXITED       = 2,                                    // 线程已退出
    I_LOG_THREAD_STATE_RUNNING      = 3,                                    // 线程正在运行

    I_LOG_THREAD_STATE_MAX,
} i_log_thread_state_t;

typedef struct {
    pthread_t                       thread_id;                              // 管道文件读取线程标识符
    i_log_thread_state_t            th_state;                               // 线程状态
    char                            th_pipe_buffer[I_LOG_FIFO_BUFFER_SIZE]; // 单次读取管道文件缓存
    char                            th_pipe_path[I_LOG_FIFO_PATH_SIZE];     // 管道文件路径
    char
        th_stop_cmd[I_LOG_CMD_THREAD_STOP_SIZE];                            // 停止线程的系统调用命令
} i_log_thread_t;

typedef struct {
    bool                            is_init;                                // 是否初始化
    i_log_thread_t                  *p_g_thread;                            // 线程信息
    sirius_log_level_t              log_level;                              // 日志等级
    atomic_flag                     atomic_lock;                            // 原子锁
} i_log_t;

static i_log_t g_h = {0};

static char time_buffer[9] = {0};
static inline char* i_log_current_time()
{
    time_t      rawtime;
    struct tm   *p_time_info;

    time(&rawtime);
    p_time_info = localtime(&rawtime);
    // @note Writes to the cache are unlocked
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", p_time_info);

    return time_buffer;
}

#define I_LOG_PRINT(color, stream, type, fmt, ...) \
    do { \
        while (atomic_flag_test_and_set(&(g_h.atomic_lock))); \
        fprintf(stream, \
        color "[%s " #type " %s (%s|%d)] " \
        fmt, i_log_current_time(), \
        SIRIUS_FILE, __FUNCTION__, __LINE__, \
        ##__VA_ARGS__); \
        fprintf(stream, LOG_NONE); \
        atomic_flag_clear(&(g_h.atomic_lock)); \
    } while (0)

#define I_LOG_INFO(fmt, ...) \
    I_LOG_PRINT(LOG_GREEN, \
    stdout, info, fmt, ##__VA_ARGS__)
#define I_LOG_WARN(fmt, ...) \
    I_LOG_PRINT(LOG_YELLOW, \
    stderr, warn, fmt, ##__VA_ARGS__)
#define I_LOG_ERROR(fmt, ...) \
    I_LOG_PRINT(LOG_RED, \
    stderr, error, fmt, ##__VA_ARGS__)

static inline int i_log_fifo_remove(const char *p_pipe)
{
    int ret = SIRIUS_ERR;

    if (likely(0 == access(p_pipe, W_OK))) {
        ret = remove(p_pipe);
        if (ret) {
            I_LOG_ERROR("Failed to delete file [%s]\n", p_pipe);
            perror(NULL);
            return ret;
        }
    }

    return SIRIUS_OK;
}

#define I_LOG_UNSUPPORTED_CMD(unsupported_cmd) \
    I_LOG_WARN("Unsupported command: \n[ %s ]\n\n", \
    unsupported_cmd); \
    return SIRIUS_ERR_INVALID_PARAMETER;

static int i_log_pipe_command_loglevel(char **pp_cmd)
{
    if (pp_cmd[0]) {
        if (likely(SIRIUS_LOG_LEVEL_0 <= atoi(pp_cmd[0])) &&
            (SIRIUS_LOG_LEVEL_MAX > atoi(pp_cmd[0]))) {
                g_h.log_level = atoi(pp_cmd[0]);
        } else {
            I_LOG_UNSUPPORTED_CMD(pp_cmd[0]);
        }
    } else {
        I_LOG_WARN("Incomplete command\n");
    }

    return SIRIUS_OK;
}

static int i_log_pipe_command_deal(char **pp_cmd)
{
    int ret = SIRIUS_OK;

    if (likely(pp_cmd[0])) {
        if (!(strcmp(LOG_LOGLEVEL_CMD, pp_cmd[0]))) {
            ret = i_log_pipe_command_loglevel(pp_cmd + 1);
            if (ret) {
                return ret;
            }
        } else {
            I_LOG_UNSUPPORTED_CMD(pp_cmd[0]);
        }
    }

    return SIRIUS_OK;
}

// 单次最长命令数量
#define I_LOG_FIFO_CMD_MAX (32)

static int i_log_pipe_command_parse(char *p_buffer)
{
    unsigned int    i                           = 0;
    char            *p_tmp                      = NULL;
    char            *p_cmd[I_LOG_FIFO_CMD_MAX]  = {NULL};

    // 末尾换行符替换为空
    p_buffer[strlen(p_buffer) - 1] = '\0';

    p_cmd[i] = strtok_r(p_buffer, " ", &p_tmp);
    while (p_cmd[i]) {
        i++;

        if (I_LOG_FIFO_CMD_MAX == i) {
            if (strtok_r(NULL, " ", &p_tmp)) {
                I_LOG_ERROR("The number of commands exceeds the cache\n");
                return SIRIUS_ERR_CACHE_OVERFLOW;
            }
            break;
        }

        p_cmd[i] = strtok_r(NULL, " ", &p_tmp);
    }

    i_log_pipe_command_deal(p_cmd);

    return SIRIUS_OK;
}

static int i_log_pipe_thread(void *args)
{
    int         fifo_fd = -1;
    ssize_t     bytes   = 0;
    const char  *p_pipe = (const char *)args;

    g_h.p_g_thread->th_state = I_LOG_THREAD_STATE_RUNNING;

    while (true) {
        fifo_fd = open(p_pipe, O_RDONLY);
        if (-1 == fifo_fd) {
            I_LOG_ERROR("open failed\n");
            perror(NULL);
            g_h.p_g_thread->th_state = I_LOG_THREAD_STATE_EXITED;
            return SIRIUS_ERR;
        }

        memset(g_h.p_g_thread->th_pipe_buffer, 0, I_LOG_FIFO_BUFFER_SIZE);
        bytes = read(fifo_fd, g_h.p_g_thread->th_pipe_buffer,
            I_LOG_FIFO_BUFFER_SIZE);
        if (bytes <= 0) {
            if (I_LOG_THREAD_STATE_EXITING == g_h.p_g_thread->th_state) {
                goto label_thread_exit;
            }
            goto label_file_close;
        }

        if (I_LOG_THREAD_STATE_EXITING == g_h.p_g_thread->th_state) {
            goto label_thread_exit;
        }

        g_h.p_g_thread->th_pipe_buffer[I_LOG_FIFO_BUFFER_SIZE - 1] = '\0';
        (void)i_log_pipe_command_parse(g_h.p_g_thread->th_pipe_buffer);

label_file_close:
        close(fifo_fd);
        fifo_fd = -1;
    }

label_thread_exit:
    close(fifo_fd);
    g_h.p_g_thread->th_state = I_LOG_THREAD_STATE_EXITED;

    return SIRIUS_OK;
}

static void i_log_pipe_destory()
{
    int ret = SIRIUS_ERR;

    if (I_LOG_THREAD_STATE_EXITED != g_h.p_g_thread->th_state) {
        g_h.p_g_thread->th_state = I_LOG_THREAD_STATE_EXITING;

#define I_LOG_THREAD_EXIT_WAIT(count) \
    for (unsigned int i = 0; i < count; i++) { \
        usleep(50 * 1000); \
        if (I_LOG_THREAD_STATE_EXITED == \
            g_h.p_g_thread->th_state) { \
            goto label_thread_join; \
        } \
    }

        I_LOG_THREAD_EXIT_WAIT(20);

        memset(g_h.p_g_thread->th_stop_cmd, 0, I_LOG_CMD_THREAD_STOP_SIZE);
        strncpy(g_h.p_g_thread->th_stop_cmd, I_LOG_CMD_THREAD_STOP,
            I_LOG_CMD_THREAD_STOP_SIZE - 1);

        size_t stop_cmd_len = strlen(g_h.p_g_thread->th_stop_cmd);
        size_t copy_len = I_LOG_CMD_THREAD_STOP_SIZE - stop_cmd_len;
        if (0 < copy_len) {
            memmove(g_h.p_g_thread->th_stop_cmd + stop_cmd_len,
                g_h.p_g_thread->th_pipe_path, copy_len);
        }

        if (unlikely('\0' !=
            g_h.p_g_thread->th_stop_cmd[I_LOG_CMD_THREAD_STOP_SIZE - 1])) {
            g_h.p_g_thread->th_stop_cmd[I_LOG_CMD_THREAD_STOP_SIZE - 1] =
                '\0';
            goto label_thread_cancel;
        }
        system(g_h.p_g_thread->th_stop_cmd);
        I_LOG_THREAD_EXIT_WAIT(10);

label_thread_cancel:
        I_LOG_WARN("Thread [%lu] will be forcibly terminated\n",
            g_h.p_g_thread->thread_id);
        pthread_cancel(g_h.p_g_thread->thread_id);
        usleep(500 * 1000);
    }

label_thread_join:
    ret = pthread_join(g_h.p_g_thread->thread_id, NULL);
    if (ret) {
        I_LOG_ERROR("pthread_join failed with [%d]\n", ret);
    }

    i_log_fifo_remove(g_h.p_g_thread->th_pipe_path);

#ifdef I_LOG_THREAD_EXIT_WAIT
#undef I_LOG_THREAD_EXIT_WAIT
#endif
}

static int i_log_pipe_create(const char *p_pipe)
{
    int ret = SIRIUS_ERR;

    mode_t fifo_mode =
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    ret = mkfifo(p_pipe, fifo_mode);
    if (EEXIST == errno) {
        I_LOG_WARN("The file [%s] already exists and will be deleted\n",
            p_pipe);

        if (i_log_fifo_remove(p_pipe)) {
            return SIRIUS_ERR;
        } else {
            ret = mkfifo(p_pipe, fifo_mode);
            goto label_mkfifo_check;
        }
    }

label_mkfifo_check:
    if (ret) {
        I_LOG_ERROR("mkfifo failed with [%d]\n", ret);
        perror(NULL);
        return ret;
    }

    ret = pthread_create(&(g_h.p_g_thread->thread_id), NULL,
        (void *)i_log_pipe_thread, (void *)p_pipe);
    if (ret) {
        I_LOG_ERROR("pthread_create failed with [%d]\n", ret);
        perror(NULL);
        goto label_pipe_delete;
    }

    return SIRIUS_OK;

label_pipe_delete:
    i_log_fifo_remove(p_pipe);

    return ret;
}

static inline void i_log_parameter_init(const sirius_log_create_t *p_handle)
{
    atomic_flag_clear(&(g_h.atomic_lock));
    g_h.log_level   = p_handle->default_log_level;
    g_h.is_init     = true;
}

int sirius_log_print(sirius_log_level_t log_level, const char *p_color,
                        const char *p_module, const char *p_fmt, ...)
{
    if (unlikely(!(g_h.is_init))) {
        I_LOG_WARN("The log module is not initialized\n");
        return 0;
    }

    if (g_h.log_level < log_level) {
        return 0;
    }

    char    buf[LOG_PRINT_BUFFER_SIZE];
    int     n = 0;
    time_t  rawtime;
    struct tm   *p_time_info;

    va_list args;
    va_start(args, p_fmt);

#define I_LOG_SNPRINTF(fd, type) \
    time(&rawtime); \
    p_time_info = localtime(&rawtime); \
    n = snprintf(buf, sizeof(buf), \
    "%s [%02d:%02d:%02d " #type " %s %lu] ", \
        p_color, \
        p_time_info->tm_hour, p_time_info->tm_min, p_time_info->tm_sec, \
        p_module, syscall(__NR_gettid)); \
    n += vsnprintf(buf + n, sizeof(buf) - n, p_fmt, args); \
    while (atomic_flag_test_and_set(&(g_h.atomic_lock))); \
    sirius_write_fd(fd, buf, strlen(buf)); \
    atomic_flag_clear(&(g_h.atomic_lock));

    switch (log_level) {
        case SIRIUS_LOG_LEVEL_0:
            return 0;
        case SIRIUS_LOG_LEVEL_DEFAULT:
            I_LOG_SNPRINTF(STDOUT_FILENO, prnt);
            break;
        case SIRIUS_LOG_LEVEL_ERROR:
            I_LOG_SNPRINTF(STDERR_FILENO, erro);
            break;
        case SIRIUS_LOG_LEVEL_WARN:
            I_LOG_SNPRINTF(STDERR_FILENO, warn);
            break;
        case SIRIUS_LOG_LEVEL_INFO:
            I_LOG_SNPRINTF(STDOUT_FILENO, info);
            break;
        case SIRIUS_LOG_LEVEL_DEBUG_1:
            I_LOG_SNPRINTF(STDOUT_FILENO, dbg1);
            break;
        case SIRIUS_LOG_LEVEL_DEBUG_2:
            I_LOG_SNPRINTF(STDOUT_FILENO, dbg2);
            break;
        case SIRIUS_LOG_LEVEL_DEBUG_3:
            I_LOG_SNPRINTF(STDOUT_FILENO, dbg3);
            break;
        default:
            I_LOG_ERROR("Unsupported log level: %d\n", log_level);
            return 0;
    }

    va_end(args);

#undef I_LOG_SNPRINTF
    return n;
}

int sirius_log_deinit()
{
    if (unlikely(!(g_h.is_init))) {
        I_LOG_WARN("The log module is not initialized\n");
        return SIRIUS_ERR_NOT_INIT;
    }

    i_log_pipe_destory();

    if (g_h.p_g_thread) {
        free(g_h.p_g_thread);
        g_h.p_g_thread = NULL;
    }

    memset(&g_h, 0, sizeof(i_log_t));

    I_LOG_INFO("The deinitialization of the log module is complete\n");
    return SIRIUS_OK;
}

int sirius_log_init(const sirius_log_create_t *p_handle)
{
    int ret = SIRIUS_ERR;

    if (unlikely(g_h.is_init)) {
        I_LOG_WARN("The log module is initialized repeatedly\n");
        return SIRIUS_ERR_INIT_REPEATED;
    }

    if (unlikely(!(p_handle))) {
        I_LOG_ERROR("Null pointer\n");
        return SIRIUS_ERR_NULL_POINTER;
    }

    g_h.p_g_thread = (i_log_thread_t *)calloc(1, sizeof(i_log_thread_t));
    if (!(g_h.p_g_thread)) {
        I_LOG_ERROR("calloc failed\n");
        return SIRIUS_ERR_MEMORY_ALLOC;
    }

    strncpy(g_h.p_g_thread->th_pipe_path, p_handle->p_pipe_path,
        I_LOG_FIFO_PATH_SIZE - 1);
    ret = i_log_pipe_create(g_h.p_g_thread->th_pipe_path);
    if (ret) {
        I_LOG_ERROR("i_log_pipe_create failed with [%d]\n", ret);
        return ret;
    }

    i_log_parameter_init(p_handle);

    I_LOG_INFO("The initialization of the log module is complete\n");
    return SIRIUS_OK;
}
