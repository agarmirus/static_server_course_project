#include "logger.h"

int logger_init(
    logger_t *const logger_ptr,
    const char *log_file_name,
    const loglevel_t level
)
{
    logger_ptr->log_file_fd = open(log_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (logger_ptr->log_file_fd == -1)
        return EXIT_FAILURE;

    logger_ptr->level = level;

    return EXIT_SUCCESS;
}

void logger_close(logger_t *const logger_ptr)
{
    close(logger_ptr->log_file_fd);
}

static void print_msg(
    const int fd,
    const char *msg,
    const char *prefix
)
{
    char log_msg[MAX_LOG_MSG_LENGTH] = {0};

    time_t cur_local_time = time(NULL);

    char *cur_local_time_str = asctime(localtime(&cur_local_time));
    cur_local_time_str[strlen(cur_local_time_str) - 1] = 0;

    sprintf(
        log_msg,
        "[%s] %s: %s\n",
        cur_local_time_str,
        prefix,
        msg
    );

    write(fd, log_msg, strlen(log_msg) + 1);
}

void logger_fatal(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= FATAL)
        print_msg(logger_ptr->log_file_fd, msg, "FATAL");
}

void logger_error(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= ERROR)
        print_msg(logger_ptr->log_file_fd, msg, "ERROR");
}

void logger_warn(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= WARN)
        print_msg(logger_ptr->log_file_fd, msg, "WARN");
}

void logger_info(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= INFO)
        print_msg(logger_ptr->log_file_fd, msg, "INFO");
}

void logger_debug(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= DEBUG)
        print_msg(logger_ptr->log_file_fd, msg, "DEBUG");
}

void logger_trace(
    logger_t *const logger_ptr,
    const char *msg
)
{
    if (logger_ptr->level >= TRACE)
        print_msg(logger_ptr->log_file_fd, msg, "TRACE");
}
