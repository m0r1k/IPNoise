#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "http_header.h"
#include "http_request.h"

static HttpRequest * g_requests[MAX_FDS];

HttpRequest * _http_request_alloc()
{
    HttpRequest *ret = NULL;
    int32_t     size = sizeof(*ret);

    ret = (HttpRequest *)malloc(size);
    if (!ret){
        PERROR("malloc failed,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }
    memset(ret, 0x00, size);

    ret->method = evbuffer_new();
    if (!ret->method){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    ret->url = evbuffer_new();
    if (!ret->url){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    ret->args = evbuffer_new();
    if (!ret->args){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    ret->proto = evbuffer_new();
    if (!ret->proto){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    ret->body = evbuffer_new();
    if (!ret->body){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    INIT_LIST_HEAD(&ret->headers.list);

out:
    return ret;
fail:
    if (ret){
        _http_request_free(ret);
        ret = NULL;
    }
    goto out;
}

void _http_request_free(
    HttpRequest *a_req)
{
    if (a_req->websock){
        websocket_free(a_req->websock);
        a_req->websock = NULL;
    }

    if (a_req->bev){
        bufferevent_free(a_req->bev);
        a_req->bev = NULL;
    }

    if (!a_req){
        PFATAL("missing argument: 'a_req'\n");
    }
    if (a_req->method){
        evbuffer_free(a_req->method);
        a_req->method = NULL;
    }
    if (a_req->url){
        evbuffer_free(a_req->url);
        a_req->url = NULL;
    }
    if (a_req->args){
        evbuffer_free(a_req->args);
        a_req->args = NULL;
    }
    if (a_req->proto){
        evbuffer_free(a_req->proto);
        a_req->proto = NULL;
    }
    if (a_req->body){
        evbuffer_free(a_req->body);
        a_req->body = NULL;
    }

    // free headers
    http_headers_free(&a_req->headers);

    free(a_req);
}

HttpRequest * http_request_alloc(
    const int32_t a_fd)
{
    HttpRequest *req = NULL;

    if (MAX_FDS <= a_fd){
        PFATAL("a_fd: '%d' >= MAX_FD", a_fd);
    }

    req = g_requests[a_fd];
    if (req){
        PWARN("request for fd: '%d' was not free\n",
            a_fd
        );
        _http_request_free(req);
    }

    req = _http_request_alloc();
    if (!req){
        PFATAL("cannot allocate request for fd: '%d'\n",
            a_fd
        );
    }

    // store request
    g_requests[a_fd] = req;

    return req;
}

void http_request_free(
    const int32_t a_fd)
{
    HttpRequest *req = NULL;

    if (MAX_FDS <= a_fd){
        PFATAL("a_fd: '%d' >= MAX_FD", a_fd);
    }

    req = g_requests[a_fd];
    if (req){
        _http_request_free(req);
        g_requests[a_fd] = NULL;
    }
}

HttpRequest * http_request_get(
    const int32_t a_fd)
{
    HttpRequest *req = NULL;

    if (MAX_FDS <= a_fd){
        PFATAL("a_fd: '%d' >= MAX_FD", a_fd);
    }

    req = g_requests[a_fd];

    return req;
}

void http_request_parse_char(
    HttpRequest     *a_req,
    const uint8_t   a_c)
{
    int64_t res;

    if (!a_req){
        PFATAL("missing argument: 'a_req'\n");
    }

    res = http_request_is_parsed(a_req);
    if (res){
        PERROR("attempt to parse request"
            " after it was marked as parsed\n"
        );
        goto fail;
    }

    PDEBUG(100, "c: '%c', state: '%d'\n",
        a_c,
        a_req->parse_state
    );

again:
    switch (a_req->parse_state){
        case PARSE_END:
            break;

        case PARSE_BODY:
            if (a_req->content_length <= EVBUFFER_LENGTH(a_req->body))
            {
                a_req->parse_state = PARSE_END;
            } else {
                res = evbuffer_add_printf(
                    a_req->body,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        case PARSE_HEADER_VAL_WAIT:
            if ('\r' == a_c){
            } else if ('\n' == a_c){
                a_req->parse_state = PARSE_HEADERS_MAY_BE_END;
                a_req->last_header = NULL;
            } else if (' ' == a_c){
                // skip spaces
            } else {
                a_req->parse_state = PARSE_HEADER_VAL;
                goto again;
            }
            break;

        case PARSE_HEADER_VAL:
            if ('\r' == a_c){
            } else if ('\n' == a_c){
                a_req->parse_state = PARSE_HEADERS_MAY_BE_END;
                a_req->last_header = NULL;
            } else {
                if (!a_req->last_header){
                    PFATAL("header was not allocated\n");
                }
                res = evbuffer_add_printf(
                    a_req->last_header->val,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        case PARSE_HEADERS_MAY_BE_END:
            if ('\r' == a_c){
            } else if ('\n' == a_c){
                HttpHeader *header = NULL;
                header = http_request_header_get(
                    a_req,
                    "Content-length"
                );
                if (header){
                    a_req->parse_state    = PARSE_BODY;
                    a_req->content_length = strtoul(
                        (const char *)EVBUFFER_DATA(header->val),
                        NULL,
                        10
                    );
                } else {
                    a_req->parse_state = PARSE_END;
                }
                break;
            } else {
                a_req->parse_state = PARSE_HEADER_NAME;
                goto again;
            }
            break;

        case PARSE_HEADER_NAME:
            if ('\r' == a_c){
            } else if ('\n' == a_c){
                a_req->parse_state = PARSE_HEADERS_MAY_BE_END;
            } else if (' ' == a_c){
                // skip spaces
            } else if (':' == a_c){
                a_req->parse_state = PARSE_HEADER_VAL_WAIT;
            } else {
                if (!a_req->last_header){
                    // alloc new header
                    a_req->last_header = http_header_alloc();
                    list_add_tail(
                        &a_req->last_header->list,
                        &a_req->headers.list
                    );
                }
                res = evbuffer_add_printf(
                    a_req->last_header->name,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

       case PARSE_PROTO:
            if ('\n' == a_c){
                a_req->parse_state = PARSE_HEADER_NAME;
            } else if ('\r' == a_c){
            } else {
                res = evbuffer_add_printf(
                    a_req->proto,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        case PARSE_ARGS:
            if (' ' == a_c){
                a_req->parse_state = PARSE_PROTO;
            } else {
                res = evbuffer_add_printf(
                    a_req->args,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        case PARSE_URL:
            if (' ' == a_c){
                a_req->parse_state = PARSE_PROTO;
            } else if ('?' == a_c){
                a_req->parse_state = PARSE_ARGS;
            } else {
                res = evbuffer_add_printf(
                    a_req->url,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        case PARSE_METHOD:
            if (' ' == a_c){
                a_req->parse_state = PARSE_URL;
            } else {
                res = evbuffer_add_printf(
                    a_req->method,
                    "%c",
                    a_c
                );
                if (0 >= res){
                    PFATAL("evbuffer_add failed\n");
                }
            }
            break;

        default:
            PERROR("unsupported state: '%d'\n",
                a_req->parse_state
            );
            break;
    }

out:
    return;
fail:
    goto out;
}

void http_request_parse_bev(
    HttpRequest         *a_req,
    struct bufferevent  *a_bev)
{
    uint8_t     c;
    int64_t     res;
    int32_t     fd      = -1;

    if (!a_req){
        PFATAL("missing argument: 'a_req'\n");
    }

    if (!a_bev){
        PFATAL("missing argument: 'a_bev'\n");
    }

    fd = bufferevent_getfd(a_bev);

    do {
        res = bufferevent_read(
            a_bev,
            &c,
            sizeof(c)
        );
        PDEBUG(100, "fd: '%d', c: '%c', state: '%d'\n",
            fd,
            c,
            a_req->parse_state
        );
        if (0 >= res){
            break;
        }

        http_request_parse_char(a_req, c);

        res = http_request_is_parsed(a_req);
        if (res){
            break;
        }

    } while (1);

    PDEBUG(55, "method: '%s'\n", EVBUFFER_DATA(a_req->method));
    PDEBUG(55, "url:    '%s'\n", EVBUFFER_DATA(a_req->url));
    PDEBUG(55, "args:   '%s'\n", EVBUFFER_DATA(a_req->args));
    PDEBUG(55, "proto:  '%s'\n", EVBUFFER_DATA(a_req->proto));
    PDEBUG(55, "body:   '%s'\n", EVBUFFER_DATA(a_req->body));

    if (55 <= g_debug_level){
        http_request_headers_dump(a_req);
    }
}

void http_request_parse_evb(
    HttpRequest         *a_req,
    struct evbuffer     *a_evb)
{
    uint8_t     c;
    int64_t     res;

    if (!a_req){
        PFATAL("missing argument: 'a_req'\n");
    }

    if (!a_evb){
        PFATAL("missing argument: 'a_evb'\n");
    }

    do {
        res = evbuffer_remove(a_evb, &c, sizeof(c));
        if (0 >= res){
            break;
        }

        http_request_parse_char(a_req, c);

        res = http_request_is_parsed(a_req);
        if (res){
            break;
        }
    } while (1);

    PDEBUG(55, "method: '%s'\n", EVBUFFER_DATA(a_req->method));
    PDEBUG(55, "url:    '%s'\n", EVBUFFER_DATA(a_req->url));
    PDEBUG(55, "args:   '%s'\n", EVBUFFER_DATA(a_req->args));
    PDEBUG(55, "proto:  '%s'\n", EVBUFFER_DATA(a_req->proto));
    PDEBUG(55, "body:   '%s'\n", EVBUFFER_DATA(a_req->body));

    if (55 <= g_debug_level){
        http_request_headers_dump(a_req);
    }
}

HttpHeader * http_request_header_get(
    HttpRequest     *a_request,
    const char      *a_name)
{
    HttpHeader  *ret = NULL;

    if (!a_request){
        PFATAL("missing argument: 'a_request'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    ret = http_header_get(
        &a_request->headers,
        a_name
    );

    return ret;
}

void http_request_headers_dump(
    HttpRequest *a_request)
{
    if (!a_request){
        PFATAL("missing argument: 'a_request'\n");
    }

    http_headers_dump(
        "request headers:",
        &a_request->headers
    );
}

int32_t http_request_is_parsed(
    HttpRequest *a_req)
{
    if (!a_req){
        PFATAL("missing argument: 'a_req'\n");
    }

    return (PARSE_END == a_req->parse_state);
}

void http_requests_free()
{
    uint32_t fd = 0;

    for (fd = 0; fd < MAX_FDS; fd++){
        http_request_free(fd);
    }
}

