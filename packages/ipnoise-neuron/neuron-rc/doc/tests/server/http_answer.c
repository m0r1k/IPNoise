#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ipnoise-common/log_common.h>
#include "libevent.h"

#include "http_answer.h"

HttpAnswer * http_answer_alloc()
{
    HttpAnswer  *ret = NULL;
    uint32_t    size = sizeof(*ret);

    ret = (HttpAnswer *)malloc(size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }

    memset(ret, 0x00, size);

    ret->body = evbuffer_new();
    if (!ret->body){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }

    ret->status_code = 500;

    ret->status_line = evbuffer_new();
    if (!ret->status_line){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }
    evbuffer_add_printf(
        ret->status_line,
        "Internal error"
    );

    ret->proto = evbuffer_new();
    if (!ret->proto){
        PERROR("evbuffer_new failed\n");
        goto fail;
    }
    evbuffer_add_printf(
        ret->proto,
        "HTTP/1.1"
    );

    INIT_LIST_HEAD(&ret->headers.list);

out:
    return ret;
fail:
    if (ret){
        http_answer_free(ret);
        ret = NULL;
    }
    goto out;
}

void http_answer_free(
    HttpAnswer *a_answer)
{
    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    // free headers
    http_headers_free(&a_answer->headers);

    if (a_answer->body){
        evbuffer_free(a_answer->body);
        a_answer->body = NULL;
    }

    if (a_answer->status_line){
        evbuffer_free(a_answer->status_line);
        a_answer->status_line = NULL;
    }

    if (a_answer->proto){
        evbuffer_free(a_answer->proto);
        a_answer->proto = NULL;
    }

    free(a_answer);
}

int32_t http_answer_send(
    HttpAnswer *a_answer)
{
    struct evbuffer *bev        = NULL;
    int32_t         fd          = -1;
    int32_t         err    = -1;
//    uint64_t        wrote       = 0;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    if (!a_answer->request){
        PERROR("attempt to send answer,"
            " but a_answer->request is NULL\n"
        );
        goto fail;
    }

    if (!a_answer->request->bev){
        PERROR("attempt to send answer,"
            " but a_answer->request->bev is NULL\n"
        );
        goto fail;
    }

    fd = bufferevent_getfd(a_answer->request->bev);
    if (0 > fd){
        PERROR("attempt to send answer,"
            " but request doesn't have correct fd"
            " (fd: '%d')\n",
            fd
        );
        goto fail;
    }

    // create answer
    bev = evbuffer_new();
    if (!bev){
        PERROR("evbuffer_new() failed\n");
        goto fail;
    }

    // serialize answer
    http_answer_serialize(a_answer, bev);

    //PWARN("send answer to fd: '%d'\n"
    //    "%s\n",
    //    fd,
    //    (const char *)EVBUFFER_DATA(bev)
    //);

    // send answer
    do {
        evbuffer_write(bev, fd);
//        if (0 < res){
//            wrote += res;
//        }
    } while (EVBUFFER_LENGTH(bev));

/*
    err = write(
        fd,
        (const char *)EVBUFFER_DATA(bev),
        EVBUFFER_LENGTH(bev)
    );
    if (wrote != EVBUFFER_LENGTH(bev)){
        PFATAL("partial write content:\n"
            " was wrote: '%llu' of '%d' byte(s)\n",
            wrote,
            EVBUFFER_LENGTH(bev)
        );
    }
*/

    // freebuffer
    evbuffer_free(bev);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void http_answer_serialize(
    HttpAnswer          *a_answer,
    struct evbuffer     *a_bev)
{
    struct list_head    *cur        = NULL;
    struct list_head    *tmp        = NULL;
    HttpHeader          *cur_header = NULL;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    if (!a_bev){
        PFATAL("missing argument: 'a_bev'\n");
    }

    // add status line
    evbuffer_add_printf(
        a_bev,
        "%s %u %s\r\n",
        (const char *)EVBUFFER_DATA(
            a_answer->proto
        ),
        a_answer->status_code,
        (const char *)EVBUFFER_DATA(
            a_answer->status_line
        )
    );

    // add Content-length header if content exist
    if (EVBUFFER_LENGTH(a_answer->body)){
        char buffer[512] = { 0x00 };
#ifdef __x86_64__
        snprintf(buffer, sizeof(buffer),
            "%lu",
            EVBUFFER_LENGTH(a_answer->body)
        );
#else
        snprintf(buffer, sizeof(buffer),
            "%u",
            EVBUFFER_LENGTH(a_answer->body)
        );
#endif
        http_answer_header_set(
            a_answer,
            "Content-Length",
            buffer
        );
    }

    // add headers
    list_for_each_safe(cur, tmp, &a_answer->headers.list){
        cur_header = list_entry(cur, HttpHeader, list);
        evbuffer_add_printf(
            a_bev,
            "%s: %s\r\n",
            (const char *)EVBUFFER_DATA(
                cur_header->name
            ),
            (const char *)EVBUFFER_DATA(
                cur_header->val
            )
        );
    }

    // add eof
    evbuffer_add_printf(
        a_bev,
        "\r\n"
    );

    // add body if exist
    if (EVBUFFER_LENGTH(a_answer->body)){
        evbuffer_add_buffer(a_bev, a_answer->body);
    }
}

int32_t http_answer_body_file_add(
    HttpAnswer  *a_answer,
    const char  *a_path)
{
    int32_t     res, err    = -1;
    int32_t     new_fd      = -1;
    struct stat st;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    new_fd = open(a_path, O_RDONLY);
    if (0 > new_fd){
        PERROR("cannot open file: '%s' for read\n",
            a_path
        );
        goto fail;
    }

    res = fstat(new_fd, &st);
    if (res){
        PERROR("stat failed, file: '%s'\n",
            a_path
        );
        goto fail;
    }

    // evbuffer_add_file will close socket
    res = evbuffer_add_file(
        a_answer->body,
        new_fd,
        0,
        st.st_size
    );
    if (res){
        PERROR("evbuffer_add_file: '%s' failed\n",
            a_path
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t http_answer_proto_set(
    HttpAnswer  *a_answer,
    const char  *a_proto)
{
    int32_t res, err = -1;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    if (!a_proto){
        PFATAL("missing argument: 'a_proto'\n");
    }

    if (!strlen(a_proto)){
        PFATAL("empty argument: 'a_proto'\n");
    }

    if (a_answer->proto){
        evbuffer_free(a_answer->proto);
        a_answer->proto = evbuffer_new();
    }

    res = evbuffer_add_printf(
        a_answer->proto,
        "%s",
        a_proto
    );

    if (0 >= res){
        PERROR("evbuffer_add_printf failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;

}

int32_t http_answer_status_set(
    HttpAnswer  *a_answer,
    uint32_t    a_status_code,
    const char  *a_status_line)
{
    int32_t res, err = -1;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }

    if (!a_status_line){
        PFATAL("missing argument: 'a_status_line'\n");
    }

    if (a_answer->status_line){
        evbuffer_free(a_answer->status_line);
        a_answer->status_line = NULL;
    }

    a_answer->status_code   = a_status_code;
    a_answer->status_line   = evbuffer_new();

    res = evbuffer_add_printf(
        a_answer->status_line,
        "%s",
        a_status_line
    );

    if (0 >= res){
        PERROR("evbuffer_add_printf failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

HttpHeader * http_answer_header_get(
    HttpAnswer  *a_answer,
    const char  *a_name)
{
    HttpHeader  *ret = NULL;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    ret = http_header_get(
        &a_answer->headers,
        a_name
    );

    return ret;
}

HttpHeader * http_answer_header_set(
    HttpAnswer  *a_answer,
    const char  *a_name,
    const char  *a_val)
{
    HttpHeader  *ret = NULL;

    if (!a_answer){
        PFATAL("missing argument: 'a_answer'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    ret = http_header_add(
        &a_answer->headers,
        a_name,
        a_val
    );

    return ret;
}

