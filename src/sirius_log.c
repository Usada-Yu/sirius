#include "sirius_errno.h"
#include "sirius_log.h"
#include "sirius_attributes.h"

#include "./internal/sirius_internal_log.h"
#include "./internal/sirius_internal_file.h"

/**
 * cache size for reading the contents
 * of a pipeline file at a time
 */
#define I_LOG_FIFO_BUR_SIZE     (1024)

/* the file path length of the pipeline file */
#define I_LOG_FIFO_PATH_SIZE    (256)

/**
 * the thread reading the pipeline file is blocked,
 * and the specific blocking location is located in
 * the read function.
 * 
 * the pipeline file will be opened and closed repeatedly,
 * and if you don't, the thread reading the pipeline file
 * will be extremely cpu intensive.
 * 
 * because in addition to opening the pipeline file,
 * you can set it to block, once data is written to the
 * pipeline file, it will become unblocked, in this case,
 * you can only close the pipeline file and open it again.
 * 
 * another way is to add appropriate sleep to the thread,
 * but I don't use that here, because I don't think it's
 * necessary to keep doing ineffective loops
 * 
 * so, when I need to quit the thread reading the pipeline
 * file, the appropriate command needs to be sent to allow
 * the blocking read function to continue and terminate the
 * thread
 */

/* thread termination command */
#define I_LOG_CMD_THD_EXIT      "echo " LOG_CMD_LV " 0 > "

/* the length of the thread termination command */
#define I_LOG_CMD_THD_EXIT_SIZE (I_LOG_FIFO_PATH_SIZE << 1)

typedef enum {
    /* invalid state */
    I_LOG_THD_STATE_INVALID = 0,

    /* thread exiting */
    I_LOG_THD_STATE_EXITING,
    /* thread exited */
    I_LOG_THD_STATE_EXITED,
    /* thread is running */
    I_LOG_THD_STATE_RUNNING,

    I_LOG_THD_STATE_MAX,
} i_log_thd_state_t;

typedef struct {
    /* thread id */
    pthread_t id;

    /* thread state */
    i_log_thd_state_t state;

    /* buffer of the single-read pipe */
    char pipe_buf[I_LOG_FIFO_BUR_SIZE];

    /* path of the pipe */
    char pipe_path[I_LOG_FIFO_PATH_SIZE];

    /* buffer of the system call to stop the thread */
    char exit_cmd[I_LOG_CMD_THD_EXIT_SIZE];
} i_log_thd_t;

typedef struct {
    /* module init flag */
    bool is_init;

    /* thread config */
    i_log_thd_t *p_thd;

    /* log level */
    sirius_log_lv_t log_lv;

#ifndef __STDC_NO_ATOMICS__
    /* atomic lock */
    atomic_flag atomic_lock;
#endif
} i_log_t;

static i_log_t g_h = {0};

#ifndef __STDC_NO_ATOMICS__
#define i_log_atomic_set() \
while (atomic_flag_test_and_set(&(g_h.atomic_lock)))
#define i_log_atomic_clear() \
atomic_flag_clear(&(g_h.atomic_lock))
#else
#define i_log_atomic_set() \
do {} while (0)
#define i_log_atomic_clear() \
do {} while (0)
#endif // __STDC_NO_ATOMICS__

static char tm_buf[9] = {0};
static inline char*
i_log_current_time()
{
    time_t raw_tm;
    time(&raw_tm);
    struct tm *p_tm_info = localtime(&raw_tm);

    /* Note that writes to the cache are unlocked */
    strftime(
        tm_buf, sizeof(tm_buf), "%H:%M:%S", p_tm_info);
    return tm_buf;
}

#define I_LOG_PRNT(color, stream, type, fmt, ...) \
    do { \
        i_log_atomic_set(); \
        fprintf(stream, \
            color "[%s " #type " %s (%s|%d)] " \
            fmt, i_log_current_time(), \
            SIRIUS_FILE, __FUNCTION__, __LINE__, \
            ##__VA_ARGS__); \
        fprintf(stream, LOG_NONE); \
        i_log_atomic_clear(); \
    } while (0)

#define I_LOG_INFO(fmt, ...) \
    I_LOG_PRNT(LOG_GREEN, \
        stdout, info, fmt, ##__VA_ARGS__)
#define I_LOG_WARN(fmt, ...) \
    I_LOG_PRNT(LOG_YELLOW, \
        stderr, warn, fmt, ##__VA_ARGS__)
#define I_LOG_ERROR(fmt, ...) \
    I_LOG_PRNT(LOG_RED, \
        stderr, error, fmt, ##__VA_ARGS__)

static inline int
i_log_fifo_rm(const char *p_pipe)
{
    int ret;
    internal_file_access_and_remove(ret, p_pipe);
    return ret;
}

#define I_LOG_CMD_ERROR(cmd) \
    I_LOG_WARN("invalid cmd: \n[ %s ]\n", \
        cmd); \
    return SIRIUS_ERR_INVALID_PARAMETER;

static int
i_log_pipe_cmd_log_lv(char **pp_cmd)
{
    if (pp_cmd[0]) {
        if ((SIRIUS_LOG_LV_0 <= atoi(pp_cmd[0])) &&
            (SIRIUS_LOG_LV_MAX > atoi(pp_cmd[0]))) {
            g_h.log_lv = atoi(pp_cmd[0]);
        } else {
            I_LOG_CMD_ERROR(pp_cmd[0]);
        }
    } else {
        I_LOG_WARN("incomplete command\n");
    }

    return SIRIUS_OK;
}

static int
i_log_pipe_cmd_deal(char **pp_cmd)
{
    int ret;

    if (pp_cmd[0]) {
        if (!(strcmp(LOG_CMD_LV, pp_cmd[0]))) {
            ret = i_log_pipe_cmd_log_lv(pp_cmd + 1);
            if (ret) {
                return ret;
            }
        } else {
            I_LOG_CMD_ERROR(pp_cmd[0]);
        }
    }

    return SIRIUS_OK;
}

/* 单次间隔命令数量 */
#define I_LOG_FIFO_CMD_MAX (32)

static void
i_log_pipe_cmd_parse(char *p_buf)
{
    /* replace the '\n' at the end with the '\0' */
    p_buf[strlen(p_buf) - 1] = '\0';

    unsigned int i = 0;
    char *p_tmp = NULL;
    char *p_cmd[I_LOG_FIFO_CMD_MAX] = {NULL};
    p_cmd[i] = strtok_r(p_buf, " ", &p_tmp);
    while (p_cmd[i]) {
        i++;

        if (I_LOG_FIFO_CMD_MAX == i) {
            if (strtok_r(NULL, " ", &p_tmp)) {
                I_LOG_WARN(
                    "the number of commands exceeded\n");
                return;
            }
            break;
        }

        p_cmd[i] = strtok_r(NULL, " ", &p_tmp);
    }

    (void)i_log_pipe_cmd_deal(p_cmd);
}

static int
i_log_pipe_thd(void *args)
{
    int fd;
    const char *p_pipe = (const char *)args;
    i_log_thd_t *p_th = g_h.p_thd;

    p_th->state = I_LOG_THD_STATE_RUNNING;
    while (true) {
        fd = open(p_pipe, O_RDONLY);
        if (-1 == fd) {
            I_LOG_ERROR("open [%s]\n", p_pipe);
            perror(NULL);
            p_th->state = I_LOG_THD_STATE_EXITED;
            return errno;
        }

        memset(p_th->pipe_buf, 0, I_LOG_FIFO_BUR_SIZE);
        ssize_t bytes = read(fd, p_th->pipe_buf,
            I_LOG_FIFO_BUR_SIZE);
        if (bytes <= 0) {
            if (I_LOG_THD_STATE_EXITING == p_th->state) {
                goto label_thd_exit;
            }
            goto label_file_close;
        }

        if (I_LOG_THD_STATE_EXITING == p_th->state) {
            goto label_thd_exit;
        }

        p_th->pipe_buf[I_LOG_FIFO_BUR_SIZE - 1] = '\0';
        i_log_pipe_cmd_parse(p_th->pipe_buf);

label_file_close:
        close(fd);
    }

label_thd_exit:
    close(fd);
    p_th->state = I_LOG_THD_STATE_EXITED;

    return SIRIUS_OK;
}

static void
i_log_pipe_destory()
{
    i_log_thd_t *p_th = g_h.p_thd;

    if (I_LOG_THD_STATE_EXITED != p_th->state) {
        p_th->state = I_LOG_THD_STATE_EXITING;

#define THD_JDG(count) \
    for (unsigned int i = 0; i < count; i++) { \
        usleep(50 * 1000); \
        if (I_LOG_THD_STATE_EXITED == \
            p_th->state) { \
            goto label_thread_join; \
        } \
    }

        THD_JDG(20);

        memset(p_th->exit_cmd, 0, I_LOG_CMD_THD_EXIT_SIZE);
        strncpy(p_th->exit_cmd, I_LOG_CMD_THD_EXIT,
            I_LOG_CMD_THD_EXIT_SIZE - 1);

        size_t stop_cmd_len = strlen(p_th->exit_cmd);
        size_t copy_len = I_LOG_CMD_THD_EXIT_SIZE - stop_cmd_len;
        if (0 < copy_len) {
            memmove(p_th->exit_cmd + stop_cmd_len,
                p_th->pipe_path, copy_len);
        }

        if ('\0' !=
            p_th->exit_cmd[I_LOG_CMD_THD_EXIT_SIZE - 1]) {
            p_th->exit_cmd[I_LOG_CMD_THD_EXIT_SIZE - 1] =
                '\0';
            goto label_thd_cancel;
        }
        system(p_th->exit_cmd);
        THD_JDG(10);

label_thd_cancel:
        I_LOG_WARN("thread[%lu] cancel\n", p_th->id);
        pthread_cancel(p_th->id);
        usleep(500 * 1000);
    }

label_thread_join:
    if (pthread_join(p_th->id, NULL)) {
        I_LOG_ERROR("pthread_join\n");
    }

    i_log_fifo_rm(p_th->pipe_path);

#ifdef THD_JDG
#undef THD_JDG
#endif
}

static int
i_log_pipe_create(const char *p_pipe)
{
    int ret;

    mode_t mode =
        S_IRUSR |
        S_IWUSR |
        S_IRGRP |
        S_IWGRP |
        S_IROTH |
        S_IWOTH;
    internal_file_mkfifo(ret, p_pipe, mode);
    if (ret) return ret;

    ret = pthread_create(&(g_h.p_thd->id), NULL,
        (void *)i_log_pipe_thd, (void *)p_pipe);
    if (ret) {
        I_LOG_ERROR("pthread_create: [%d]\n", ret);
        perror(NULL);
        i_log_fifo_rm(p_pipe);
    }

    return ret;
}

static inline void
i_log_param_init(const sirius_log_cr_t *p_cr)
{
    i_log_atomic_clear();
    g_h.log_lv = p_cr->log_lv;
    g_h.is_init = true;
}

int
sirius_log_print(sirius_log_lv_t log_lv,
    const char *p_color,
    const char *p_mod,
    const char *p_file,
    const char *p_func,
    int line,
    const char *p_fmt, ...)
{
    if (unlikely(!(g_h.is_init))) return 0;
    if (g_h.log_lv < log_lv) return 0;

    char buf[LOG_PRNT_BUF_SIZE];
    int n = 0;
    time_t raw_tm;
    struct tm *p_tm_info;

    va_list args;
    va_start(args, p_fmt);

#define i_log_prnt(fd, type) \
    time(&raw_tm); \
    p_tm_info = localtime(&raw_tm); \
    n = snprintf(buf, sizeof(buf), \
        "%s[%02d:%02d:%02d " #type " %s %lu " \
        "%s (%s|%d)] ", \
            p_color, \
            p_tm_info->tm_hour, \
            p_tm_info->tm_min, \
            p_tm_info->tm_sec, \
            p_mod, syscall(__NR_gettid), \
            p_file, p_func, line); \
    n += vsnprintf( \
        buf + n, sizeof(buf) - n, p_fmt, args); \
    n += snprintf(buf + n, sizeof(buf) - n, LOG_NONE); \
    i_log_atomic_set(); \
    write(fd, buf, n + 1); \
    i_log_atomic_clear();

    switch (log_lv) {
        case SIRIUS_LOG_LV_0:
            return 0;
        case SIRIUS_LOG_LV_DEFAULT:
            i_log_prnt(STDOUT_FILENO, prnt)
            break;
        case SIRIUS_LOG_LV_ERROR:
            i_log_prnt(STDERR_FILENO, error)
            break;
        case SIRIUS_LOG_LV_WARN:
            i_log_prnt(STDERR_FILENO, warn)
            break;
        case SIRIUS_LOG_LV_INFO:
            i_log_prnt(STDOUT_FILENO, info)
            break;
        case SIRIUS_LOG_LV_DEBG:
            i_log_prnt(STDOUT_FILENO, debg)
            break;
        default:
            I_LOG_WARN("invalid log level: %d\n", log_lv);
            return 0;
    }

    va_end(args);

#undef i_log_prnt
    return n;
}

void
sirius_log_deinit()
{
    if (!(g_h.is_init)) {
        return;
    }

    i_log_pipe_destory();

    if (g_h.p_thd) {
        free(g_h.p_thd);
        g_h.p_thd = NULL;
    }

    memset(&g_h, 0, sizeof(i_log_t));
}

int
sirius_log_init(const sirius_log_cr_t *p_cr)
{
    if (g_h.is_init) {
        SIRIUS_DEBG("repeat initialization\n");
        return SIRIUS_OK;
    }

    if (!(p_cr)) {
        I_LOG_ERROR("null pointer\n");
        return SIRIUS_ERR_INVALID_ENTRY;
    }

    g_h.p_thd = (i_log_thd_t *)calloc(
        1, sizeof(i_log_thd_t));
    if (!(g_h.p_thd)) {
        I_LOG_ERROR("calloc\n");
        return SIRIUS_ERR_MEMORY_ALLOC;
    }

    strncpy(g_h.p_thd->pipe_path, p_cr->p_pipe,
        I_LOG_FIFO_PATH_SIZE - 1);
    int ret = i_log_pipe_create(g_h.p_thd->pipe_path);
    if (ret) {
        I_LOG_ERROR(
            "i_log_pipe_create: [%d]\n", ret);
        return ret;
    }

    i_log_param_init(p_cr);

    return SIRIUS_OK;
}
