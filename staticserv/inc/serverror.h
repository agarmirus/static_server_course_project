#ifndef SERVERROR_H
#define SERVERROR_H

#define ERROR_LOGGER_INIT 50

#define ERROR_SOCKET_INIT 100
#define ERROR_SOCKET_BIND 101
#define ERROR_LISTEN_SOCKET 102
#define ERROR_SOCKET_ACCEPT 103
#define ERROR_PSELECT_READ 104
#define ERROR_PSELECT_WRITE 105
#define ERROR_PSELECT_ERR 106
#define ERROR_FORK_PROCCESS 107
#define ERROR_CLIENT_SOCKET 108

#define ERROR_FDSARR_ALLOC 150
#define ERROR_FDSARR_APPEND 151

#define ERROR_HTTP_REQ_PARSE 200
#define ERROR_HTTP_RES_ADD_HEADER 201
#define ERROR_HTTP_UKNOWN_FILE_TYPE 202

#endif
