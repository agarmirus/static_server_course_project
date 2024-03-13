#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_LOG_FILE_NAME "log.txt"
#define DEFAULT_LOG_LEVEL TRACE

#define MAX_LOG_MSG_LENGTH 200

typedef enum
{
    OFF,
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
} loglevel_t;

typedef struct
{
    int log_file_fd;
    loglevel_t level;
} logger_t;

int logger_init(logger_t *const, const char *, const loglevel_t);

void logger_close(logger_t *const);

void logger_fatal(logger_t *const, const char *);

void logger_error(logger_t *const, const char *);

void logger_warn(logger_t *const, const char *);

void logger_info(logger_t *const, const char *);

void logger_debug(logger_t *const, const char *);

void logger_trace(logger_t *const, const char *);

#endif
