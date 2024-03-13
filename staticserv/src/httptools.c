#include "httptools.h"

#define REQ_LINE_PARTS_COUNT 2

#define READ_BUF_SIZE 200
#define VERB_BUF_SIZE 10
#define PATH_BUF_SIZE 100

#define DEFAULT_HEADERS_BUF_SIZE 10
#define HTTP_RES_STRING_BUF_SIZE 200

httpreq_t httpreq_parse_from_client(
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const int socket_fd
)
{
    httpreq_t httpreq = {UNKNOWN, NULL};

    char read_buf[READ_BUF_SIZE] = {0};

    if (recvfrom(socket_fd, read_buf, READ_BUF_SIZE, 0, (struct sockaddr *)client_address, client_address_len) == -1)
        return httpreq;

    char verb_buf[VERB_BUF_SIZE] = {0};
    char path_buf[PATH_BUF_SIZE] = {0};

    int read_count = sscanf(read_buf, "%s%s", verb_buf, path_buf);

    if (read_count < 2)
        return httpreq;

    if (strcmp(verb_buf, "GET") == 0)
        httpreq.verb = GET;
    else if (strcmp(verb_buf, "HEAD") == 0)
        httpreq.verb = HEAD;
    else
        return httpreq;

    if (strcmp(path_buf, "/") == 0)
        strcpy(path_buf, DEFAULT_FILE);

    size_t read_path_length = strlen(path_buf);

    char *new_path = calloc(read_path_length + 2, sizeof(char));

    if (new_path)
    {
        strcpy(new_path + 1, path_buf);
        *new_path = '.';

        httpreq.path = new_path;
    }

    return httpreq;
}

void httpreq_free(httpreq_t *const httpreq)
{
    free(httpreq->path);
    httpreq->path = NULL;
}

bool httpreq_is_null(const httpreq_t *const httpreq)
{
    return !(bool)httpreq->path;
}

httpverb_t httpreq_verb(const httpreq_t *const httpreq)
{
    return httpreq->verb;
}

const char *httpreq_path(const httpreq_t *const httpreq)
{
    return httpreq->path;
}

static httpheaders_t httpheaders_init(void)
{
    httpheaders_t httpheaders = {0, 0, NULL};

    return httpheaders;
}

httpres_t httpres_init(
    const short unsigned code,
    const char *description
)
{
    httpheaders_t httpheaders = httpheaders_init();
    httpres_t httpres = {0, NULL, httpheaders};

    size_t description_length = strlen(description);

    char *new_description = malloc(description_length);

    if (new_description)
    {
        memcpy(new_description, description, description_length + 1);

        httpres.code = code;
        httpres.description = new_description;
    }

    return httpres;
}

static void httpheaders_free(httpheaders_t *const httpheaders)
{
    for (size_t i = 0; i < httpheaders->length; ++i)
    {
        free(httpheaders->arr[i].name);
        free(httpheaders->arr[i].value);
    }

    free(httpheaders->arr);
    httpheaders->arr = NULL;
}

void httpres_free(httpres_t *const httpres)
{
    free(httpres->description);
    httpres->description = NULL;

    httpheaders_free(&httpres->headers);
}

static int httpheaders_add_header(
    httpheaders_t *const httpheaders,
    const httpheader_t *const httpheader
)
{
    if (!httpheaders->arr)
    {
        httpheaders->arr = calloc(DEFAULT_HEADERS_BUF_SIZE, sizeof(httpheader_t));

        if (!httpheaders->arr)
        {
            return ERROR_HTTP_RES_ADD_HEADER;
        }

        httpheaders->buf_size = DEFAULT_HEADERS_BUF_SIZE;
    }

    if (httpheaders->length == httpheaders->buf_size)
    {
        httpheader_t *realloced_arr = realloc(httpheaders->arr, 2 * httpheaders->buf_size);

        if (!realloced_arr)
        {
            return ERROR_HTTP_RES_ADD_HEADER;
        }

        httpheaders->arr = realloced_arr;
        httpheaders->buf_size *= 2;
    }

    httpheaders->arr[httpheaders->length++] = *httpheader;

    return EXIT_SUCCESS;
}

int httpres_add_header(
    httpres_t *const httpres,
    const char *name,
    const char *value
)
{
    size_t name_length = strlen(name);
    size_t value_length = strlen(value);

    char *new_name = malloc(name_length);

    if (!new_name)
        return ERROR_HTTP_RES_ADD_HEADER;

    char *new_value = malloc(value_length);

    if (!new_value)
    {
        free(new_name);

        return ERROR_HTTP_RES_ADD_HEADER;
    }

    strcpy(new_name, name);
    strcpy(new_value, value);

    httpheader_t header = {new_name, new_value};

    int rc = httpheaders_add_header(&httpres->headers, &header);

    if (rc != EXIT_SUCCESS)
    {
        free(new_name);
        free(new_value);
    }

    return rc;
}

void httpres_send(
    const int socket_fd,
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const httpres_t *const httpres
)
{
    char buf[HTTP_RES_STRING_BUF_SIZE] = {0};

    sprintf(buf, "HTTP/1.0 %d %s\r\n", httpres->code, httpres->description);
    sendto(socket_fd, buf, strlen(buf), 0, (struct sockaddr *)client_address, *client_address_len);

    for (size_t i = 0; i < httpres->headers.length; ++i)
    {
        sprintf(buf, "%s: %s\r\n", httpres->headers.arr[i].name, httpres->headers.arr[i].value);
        sendto(socket_fd, buf, strlen(buf), 0, (struct sockaddr *)client_address, *client_address_len);
    }

    sendto(socket_fd, "\r\n", 2, 0, (struct sockaddr *)client_address, *client_address_len);
}
