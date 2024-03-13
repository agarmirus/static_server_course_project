#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#include "conf.h"
#include "logger.h"
#include "fdsarr.h"
#include "serverror.h"
#include "httptools.h"
#include "pathtools.h"

#define READ_BUF_SIZE 1024

#define max(a, b) a >= b ? a : b

static int listen_fd = 0;

logger_t logger = {0, 0};

void server_shutdown(int signum)
{
    logger_info(&logger, "server shutdown");

    logger_trace(&logger, "closing listen socket");
    close(listen_fd);

    logger_trace(&logger, "closing logger");
    logger_close(&logger);

    exit(EXIT_SUCCESS);
}

static void change_sig_handlers(void)
{
    logger_trace(&logger, "setting SIGINT signal handler");
    signal(SIGINT, server_shutdown);

    logger_trace(&logger, "setting SIGTERM signal handler");
    signal(SIGTERM, server_shutdown);

    logger_trace(&logger, "setting SIGTSTP signal ignoring");
    signal(SIGTSTP, SIG_IGN);

    logger_trace(&logger, "setting SIGCHLD signal ignoring");
    signal(SIGCHLD, SIG_IGN);
}

static void init_listen_fd(void)
{
    logger_trace(&logger, "initializing server socket address struct");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);
    
    logger_trace(&logger, "creating listen cosket");
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1)
    {
        logger_fatal(&logger, "error while creating listen socket");
        exit(ERROR_SOCKET_INIT);
    }

    logger_trace(&logger, "binding listen socket to server address");
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        logger_fatal(&logger, "error while creating listen socket");

        logger_trace(&logger, "closing listen socket");
        close(listen_fd);

        exit(ERROR_SOCKET_BIND);
    }

    logger_trace(&logger, "marking listen socket as connection-mode");
    if (listen(listen_fd, LISTEN_MAX_CONNECTIONS) == -1)
    {
        logger_fatal(&logger, "error while marking listen socket as connection-mode");

        logger_trace(&logger, "closing listen socket");
        close(listen_fd);

        exit(ERROR_LISTEN_SOCKET);
    }
}

static sigset_t init_pselect_sigset(void)
{
    logger_info(&logger, "initializing pselect sigset");

    sigset_t pselect_sigmask;

    sigemptyset(&pselect_sigmask);
    // sigaddset(&pselect_sigmask, SIGINT);
    sigaddset(&pselect_sigmask, SIGTERM);

    return pselect_sigmask;
}

static void send_error(
    const int fd,
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const int code,
    const char *description
)
{
    logger_info(&logger, "sending HTTP-error");

    httpres_t res = httpres_init(code, description);

    char content[100] = {0};
    sprintf(content, "<h1>%d %s</h1>", code, description);

    char content_length_str[100] = {0};
    sprintf(content_length_str, "%zu", strlen(content));

    httpres_add_header(&res, "Content-Type", "text/html");
    httpres_add_header(&res, "Content-Length", content_length_str);

    httpres_send(fd, client_address, client_address_len, &res);

    sendto(fd, content, strlen(content), 0, (struct sockaddr *)client_address, *client_address_len);
}

static int form_httpres_headers(
    httpres_t *const httpres,
    const char *content_type,
    const int file_fd
)
{
    logger_info(&logger, "forming http headers");

    int rc = httpres_add_header(httpres, "Content-Type", content_type);

    if (rc != EXIT_SUCCESS)
    {
        return rc;
    }

    char buf[100] = {0};
    sprintf(buf, "%zu", file_size(file_fd));

    rc = httpres_add_header(httpres, "Content-Length", buf);

    return rc;
}

static void send_file_content(
    const int client_fd,
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const int file_fd
)
{
    logger_info(&logger, "sending file content");

    char buf[READ_BUF_SIZE] = {0};

    ssize_t read_bytes = read(file_fd, buf, READ_BUF_SIZE);

    while (read_bytes > 0)
    {
        sendto(client_fd, buf, read_bytes, 0, (struct sockaddr *)client_address, *client_address_len);

        read_bytes = read(file_fd, buf, READ_BUF_SIZE);
    }
}

static void send_response(
    const int client_fd,
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const int file_fd,
    const filetype_t filetype,
    const httpverb_t verb
)
{
    logger_info(&logger, "sending OK http response");

    logger_trace(&logger, "send_response - initializing httpes");
    httpres_t httpres = httpres_init(HTTP_CODE_OK, "OK");

    int rc = EXIT_SUCCESS;

    logger_trace(&logger, "send_response - generating httpres headers according to file type");
    switch (filetype)
    {
        case NONE:
            rc = ERROR_HTTP_UKNOWN_FILE_TYPE;
            break;
        case TXT:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_TXT, file_fd);
            break;
        case HTML:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_HTML, file_fd);
            break;
        case CSS:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_CSS, file_fd);
            break;
        case JS:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_JS, file_fd);
            break;
        case PNG:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_PNG, file_fd);
            break;
        case JPG:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_JPG, file_fd);
            break;
        case JPEG:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_JPEG, file_fd);
            break;
        case SWF:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_SWF, file_fd);
            break;
        case GIF:
            rc = form_httpres_headers(&httpres, HTTP_CONTENT_TYPE_GIF, file_fd);
            break;
    }

    if (rc == EXIT_SUCCESS)
    {
        logger_trace(&logger, "send_response - generating httpres headers according to file type");
        httpres_send(client_fd, client_address, client_address_len, &httpres);

        if (verb == GET)
        {
            send_file_content(client_fd, client_address, client_address_len, file_fd);
        }
    }
    else
    {
        logger_info(&logger, "internal server error while generating httpres headers");
        send_error(client_fd, client_address, client_address_len, HTTP_CODE_INTERNAL_SERVER_ERROR, "Internal server error");
    }

    httpres_free(&httpres);
}

static void perform_request(
    const int client_fd,
    struct sockaddr_in *const client_address,
    socklen_t *const client_address_len,
    const httpreq_t *const httpreq_ptr
)
{
    logger_info(&logger, "performing http request");

    logger_trace(&logger, "perform_request - checking http verb");
    httpverb_t verb = httpreq_verb(httpreq_ptr);

    if (verb != GET && verb != HEAD)
    {
        logger_info(&logger, "sending 405 Method Not Allowed");
        send_error(client_fd, client_address, client_address_len, HTTP_CODE_METHOD_NOT_ALLOWED, "Method not allowed");
        return;
    }

    logger_trace(&logger, "perform_request - getting http URI");
    const char *path = httpreq_path(httpreq_ptr);

    if (!path_is_inside(path))
    {
        logger_info(&logger, "sending 403 Forbidden");
        send_error(client_fd, client_address, client_address_len, HTTP_CODE_FORBIDDEN, "Forbidden");
        return;
    }

    logger_trace(&logger, "perform_request - opening source file for reading");
    int requested_fd = open(path, O_RDONLY);

    if (requested_fd == -1)
    {
        logger_info(&logger, "sending 404 Not Found");
        send_error(client_fd, client_address, client_address_len, HTTP_CODE_NOT_FOUND, "Not found");
        return;
    }

    logger_trace(&logger, "perform_request - getting source file type");
    filetype_t filetype = file_type(path);

    if (filetype == NONE)
    {
        logger_info(&logger, "unknown file type, sending 405 Method Not Allowed");
        send_error(client_fd, client_address, client_address_len, HTTP_CODE_METHOD_NOT_ALLOWED, "Method not allowed");
        close(requested_fd);
        return;
    }

    send_response(client_fd, client_address, client_address_len, requested_fd, filetype, verb);

    logger_trace(&logger, "perform_request - closing source file");
    close(requested_fd);
}

static int serve_client(const int client_fd)
{
    logger_info(&logger, "forking proccess for client service");
    
    int pid = -1;

    if ((pid = fork()) == -1)
    {
        logger_error(&logger, "cannot fork");

        return ERROR_FORK_PROCCESS;
    }

    if (pid == 0)
    {
        logger_info(&logger, "child process is serving clients request");

        logger_trace(&logger, "serve_client - parsing http request from client");

        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        httpreq_t httpreq = httpreq_parse_from_client(&client_address, &client_address_len, client_fd);

        if (httpreq_is_null(&httpreq))
        {
            logger_error(&logger, "cannot parse http request, child process is beeing killed");
            
            exit(ERROR_HTTP_REQ_PARSE);
        }

        logger_trace(&logger, "serve_client - getting http verb");
        httpverb_t verb = httpreq_verb(&httpreq);

        perform_request(client_fd, &client_address, &client_address_len, &httpreq);

        logger_trace(&logger, "serve_client - free http request structure memory");
        httpreq_free(&httpreq);
    }

    close(client_fd);

    if (pid == 0)
        exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

static int connect_new_client(
    fdsarr_t *const client_fds_arr_ptr
)
{
    logger_info(&logger, "accepting new connection");

    logger_trace(&logger, "connect_new_client - accepting new client fd with accept");
    struct sockaddr client_addr; 
    socklen_t client_len;
    
    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_fd == -1)
    {
        logger_error(&logger, "client socket accepting error");

        return ERROR_SOCKET_ACCEPT;
    }

    logger_trace(&logger, "connect_new_client - appending new client fd to clients fds array");
    int rc = fdsarr_append(client_fds_arr_ptr, client_fd);

    if (rc != EXIT_SUCCESS)
    {
        logger_error(&logger, "connect_new_client - cannot append new client fd to clients fds array");

        return ERROR_FDSARR_APPEND;
    }

    return EXIT_SUCCESS;
}

static void handle_read_sockets(
    fdsarr_t *const client_fds_arr_ptr,
    fd_set *const read_fd_set_ptr
)
{
    logger_info(&logger, "handling sockets");

    size_t i = 0;

    while (i < client_fds_arr_ptr->length)
    {
        int socket_fd = client_fds_arr_ptr->arr[i];

        if (FD_ISSET(socket_fd, read_fd_set_ptr))
        {
            FD_CLR(socket_fd, read_fd_set_ptr);
            fdsarr_remove(client_fds_arr_ptr, socket_fd);

            serve_client(socket_fd);
        }
        else
            ++i;
    }

    if (FD_ISSET(listen_fd, read_fd_set_ptr))
        connect_new_client(client_fds_arr_ptr);
}

static void update_read_fds_set(
    int *const nfds_ptr,
    fd_set *const read_fd_set_ptr,
    const fdsarr_t *const client_fds_arr_ptr
)
{
    logger_info(&logger, "updating read fds set");

    FD_ZERO(read_fd_set_ptr);

    size_t client_fds_count = fdsarr_length(client_fds_arr_ptr);

    for (size_t i = 0; i < client_fds_count; ++i)
        FD_SET(client_fds_arr_ptr->arr[i], read_fd_set_ptr);

    FD_SET(listen_fd, read_fd_set_ptr);

    *nfds_ptr = max(listen_fd, fdsarr_max(client_fds_arr_ptr));
}

int main(void)
{
    int rc = logger_init(&logger, DEFAULT_LOG_FILE_NAME, DEFAULT_LOG_LEVEL);

    if (rc != EXIT_SUCCESS)
        return ERROR_LOGGER_INIT;

    logger_info(&logger, "setting signals handlers");
    change_sig_handlers();

    logger_info(&logger, "initializing listen socket");
    init_listen_fd();

    logger_info(&logger, "initializing read fd set");
    fd_set read_fd_set;
    FD_ZERO(&read_fd_set);
    FD_SET(listen_fd, &read_fd_set);

    logger_info(&logger, "allocation memory for clients fds array");
    fdsarr_t client_fds_arr = fdsarr_alloc(DEFAULT_FDSARR_BUF_SIZE);

    if (fdsarr_is_null(&client_fds_arr))
    {
        logger_fatal(&logger, "cannot allocate memory for clients fds array");

        logger_info(&logger, "closing listen socket");
        close(listen_fd);

        logger_info(&logger, "closing logger");
        logger_close(&logger);

        exit(ERROR_FDSARR_ALLOC);
    }

    sigset_t pselect_sigmask = init_pselect_sigset();

    int nfds = listen_fd;

    logger_info(&logger, "running server");

    while (1)
    {
        logger_info(&logger, "searching for fds with pselect");
        rc = pselect(nfds + 1, &read_fd_set, NULL, NULL, NULL, &pselect_sigmask);

        if (rc == -1)
        {
            logger_info(&logger, "closing listen socket");
            close(listen_fd);

            logger_info(&logger, "closing read fds from fdsarr");
            fdsarr_close(&client_fds_arr);

            logger_info(&logger, "free fdsarr allocated memory");
            fdsarr_free(&client_fds_arr);

            logger_info(&logger, "closing logger");
            logger_close(&logger);

            exit(ERROR_PSELECT_READ);
        }

        handle_read_sockets(&client_fds_arr, &read_fd_set);

        update_read_fds_set(&nfds, &read_fd_set, &client_fds_arr);
    }

    return EXIT_SUCCESS;
}
