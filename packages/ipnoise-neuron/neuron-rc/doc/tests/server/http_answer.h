#ifndef HTTP_ANSWER_H
#define HTTP_ANSWER_H

#include "http_header.h"
#include "http_request.h"

typedef struct
{
    HttpRequest         *request;
    struct evbuffer     *body;
    uint32_t            status_code;
    struct evbuffer     *status_line;
    struct evbuffer     *proto;
    HttpHeader          headers;
} HttpAnswer;

HttpAnswer *    http_answer_alloc();
void            http_answer_free(HttpAnswer *);
int32_t         http_answer_send(
    HttpAnswer *a_answer
);
void            http_answer_serialize(
    HttpAnswer          *a_answer,
    struct evbuffer     *a_bev
);
int32_t         http_answer_body_file_add(
    HttpAnswer  *a_answer,
    const char  *a_path
);
int32_t         http_answer_proto_set(
    HttpAnswer  *a_answer,
    const char  *a_proto
);
int32_t         http_answer_status_set(
    HttpAnswer  *a_answer,
    uint32_t    a_status_code,
    const char  *a_status_line
);

HttpHeader * http_answer_header_get(
    HttpAnswer  *a_answer,
    const char  *a_name
);

HttpHeader * http_answer_header_set(
    HttpAnswer  *a_answer,
    const char  *a_name,
    const char  *a_val
);

HttpHeader * http_answer_header_add(
    HttpAnswer  *a_answer,
    const char  *a_name,
    const char  *a_val
);

HttpHeader * http_header_get_create(
    HttpHeader      *a_header,
    const char      *a_name
);

#endif

