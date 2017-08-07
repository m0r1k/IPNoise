#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdint.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/listener.h>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/list.h>

#define MAX_FDS  100

#include "http_header.h"
#include "websocket.h"

enum ParseState
{
    PARSE_METHOD                = 0,
    PARSE_URL,
    PARSE_ARGS,
    PARSE_PROTO,
    PARSE_HEADER_NAME,
    PARSE_HEADER_VAL_WAIT,
    PARSE_HEADER_VAL,
    PARSE_HEADERS_MAY_BE_END,
    PARSE_BODY,
    PARSE_END,
};

typedef struct
{
    struct bufferevent  *bev;
    int32_t             parse_state;
    struct evbuffer     *method;
    struct evbuffer     *url;
    struct evbuffer     *args;
    struct evbuffer     *proto;
    HttpHeader          headers;
    struct evbuffer     *body;
    uint64_t            content_length;
    HttpHeader          *last_header;
    Websocket           *websock;
} HttpRequest;

HttpRequest *   _http_request_alloc();
void            _http_request_free(HttpRequest *);
HttpRequest *   http_request_alloc(const int32_t a_fd);
void            http_request_free(const int32_t a_fd);
HttpRequest *   http_request_get(const int32_t a_fd);
void            http_request_parse_char(
    HttpRequest *,
    const uint8_t
);
void            http_request_parse_bev(
    HttpRequest         *a_req,
    struct bufferevent  *a_bev
);
void            http_request_parse_evb(
    HttpRequest         *a_req,
    struct evbuffer     *a_evb
);

HttpHeader *    http_request_header_get(
    HttpRequest     *,
    const char      *a_name
);
void            http_request_headers_dump(HttpRequest *);
void            http_header_free(HttpHeader *);
HttpHeader *    http_header_alloc();
int32_t         http_request_is_parsed(HttpRequest *);
void            http_requests_free();

#endif

