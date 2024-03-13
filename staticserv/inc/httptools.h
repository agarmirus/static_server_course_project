#ifndef HTTPTOOLS_H
#define HTTPTOOLS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "conf.h"
#include "serverror.h"

#define HTTP_CODE_OK 200

#define HTTP_CODE_FORBIDDEN 403
#define HTTP_CODE_NOT_FOUND 404
#define HTTP_CODE_METHOD_NOT_ALLOWED 405

#define HTTP_CODE_INTERNAL_SERVER_ERROR 500

#define HTTP_CONTENT_TYPE_TXT "text/plain"
#define HTTP_CONTENT_TYPE_HTML "text/html"
#define HTTP_CONTENT_TYPE_CSS "text/css"
#define HTTP_CONTENT_TYPE_JS "text/javascript"
#define HTTP_CONTENT_TYPE_PNG "image/png"
#define HTTP_CONTENT_TYPE_JPG "image/jpg"
#define HTTP_CONTENT_TYPE_JPEG "image/jpeg"
#define HTTP_CONTENT_TYPE_SWF "application/x-shockwave-flash"
#define HTTP_CONTENT_TYPE_GIF "image/gif"

typedef enum
{
    UNKNOWN,
    GET,
    HEAD
} httpverb_t;

typedef struct
{
    char *name;
    char *value;
} httpheader_t;

typedef struct
{
    size_t buf_size;
    size_t length;

    httpheader_t *arr;
} httpheaders_t;

typedef struct
{
    httpverb_t verb;
    char *path;
} httpreq_t;

typedef struct
{
    short unsigned code;
    char *description;

    httpheaders_t headers;
} httpres_t;

httpreq_t httpreq_parse_from_client(
    struct sockaddr_in *const,
    socklen_t *const,
    const int
);

void httpreq_free(httpreq_t *const);

bool httpreq_is_null(const httpreq_t *const);

httpverb_t httpreq_verb(const httpreq_t *const);

const char *httpreq_path(const httpreq_t *const);

httpres_t httpres_init(const short unsigned, const char *);

void httpres_free(httpres_t *const);

int httpres_add_header(httpres_t *const, const char *, const char *);

void httpres_send(
    const int,
    struct sockaddr_in *const,
    socklen_t *const,
    const httpres_t *const
);

#endif
