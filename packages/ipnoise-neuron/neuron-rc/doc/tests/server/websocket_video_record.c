#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "http_request.h"

#include "websocket_video_record.h"

uint32_t g_write_id = 0;

void websocket_frame_ready_video_record(
    Websocket           *a_websock,
    struct evbuffer     *a_frame_data,
    void                *a_ctx)
{
    HttpRequest     *req        = (HttpRequest *)a_ctx;
    char            buffer[512] = { 0x00 };
    int32_t         res         = 0;
    int32_t         fd          = bufferevent_getfd(req->bev);
    struct stat     st;

#ifdef __x86_64__
    PWARN("websock frame ready, len: '%lu'\n",
        EVBUFFER_LENGTH(a_frame_data)
    );
#else
    PWARN("websock frame ready, len: '%u'\n",
        EVBUFFER_LENGTH(a_frame_data)
    );
#endif

    // record
    do {
        int32_t tmp_fd = -1;

        res = stat("tmp", &st);
        if (res){
            res = mkdir("tmp", 0755);
            if (res){
                PFATAL("mkdir failed\n");
            }
        }
        snprintf(buffer, sizeof(buffer),
            "tmp/%d.jpg",
            g_write_id++
        );

        tmp_fd = open(buffer, O_WRONLY | O_CREAT, 0644);
        if (0 > tmp_fd){
            PWARN("cannot open: '%s'"
                " for write\n",
                buffer
            );
            break;
        }

        res = write(
            tmp_fd,
            (const char *)EVBUFFER_DATA(a_frame_data),
            EVBUFFER_LENGTH(a_frame_data)
        );
        if (res != EVBUFFER_LENGTH(a_frame_data)){
#ifdef __x86_64__
            PERROR("partial write content"
                " file: '%s',"
                " res: '%d',"
                " EVBUFFER_LENGTH(a_frame_data): '%lu'"
                "\n",
                buffer,
                res,
                EVBUFFER_LENGTH(a_frame_data)
            );
#else
            PERROR("partial write content"
                " file: '%s',"
                " res: '%d',"
                " EVBUFFER_LENGTH(a_frame_data): '%u'"
                "\n",
                buffer,
                res,
                EVBUFFER_LENGTH(a_frame_data)
            );
#endif
        }

        close(tmp_fd);
    } while (0);

    // send answer
    websocket_send(
        fd,
        FRAME_OPCODE_BIN,
        a_frame_data
    );

    return;
}

