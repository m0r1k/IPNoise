#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

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

typedef struct
{
    struct evbuffer     *name;
    struct evbuffer     *val;
    struct list_head    list;
} HttpHeader;

void            http_header_free(HttpHeader *a_header);
HttpHeader *    http_header_alloc();

void            http_headers_dump(
    const char  *a_prefix,
    HttpHeader  *a_list
);
HttpHeader *    http_header_get(
    HttpHeader      *a_list,
    const char      *a_name
);
HttpHeader *    http_header_get_create(
    HttpHeader      *a_list,
    const char      *a_name
);
void            http_header_remove(
    HttpHeader  *a_list,
    const char  *a_name
);
HttpHeader *    http_header_add(
    HttpHeader      *a_list,
    const char      *a_name,
    const char      *a_val
);

void http_headers_free(
    HttpHeader *a_list
);

#endif

