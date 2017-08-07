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

#include "websocket_video_play.h"

uint32_t g_read_id = 0;

void websocket_frame_ready_video_play(
    Websocket           *a_websock,
    struct evbuffer     *a_frame,
    void                *a_ctx)
{
    HttpRequest     *req        = (HttpRequest *)a_ctx;
    char            buffer[512] = { 0x00 };
    int32_t         res         = 0;
    int32_t         fd          = bufferevent_getfd(req->bev);
    int32_t         tmp_fd      = -1;
    struct evbuffer *frame_data = evbuffer_new();
    struct stat     st;

    if (!frame_data){
        PERROR("evbuffer_new() failed\n");
        goto fail;
    }

#ifdef __x86_64__
    PWARN("websock frame ready, len: '%lu'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#else
    PWARN("websock frame ready, len: '%u'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#endif

again:
    snprintf(buffer, sizeof(buffer),
        "tmp/%d.jpg",
        g_read_id++
    );

    do {
        tmp_fd = open(buffer, O_RDONLY);
        if (0 > tmp_fd){
            PWARN("cannot open: '%s'"
                " for write\n",
                buffer
            );
            if (1 == g_read_id){
                // it is first frame and it is not exist
                // go away from here
                break;
            }
            // play from start
            g_read_id = 0;
            goto again;
        }
        res = fstat(tmp_fd, &st);
        if (res){
            PERROR("stat failed, file: '%s'\n",
                buffer
            );
            close(fd);
            break;
        }

        // evbuffer_add_file will close fd
        res = evbuffer_add_file(
            frame_data,
            tmp_fd,
            0,
            st.st_size
        );
        if (res){
            PERROR("evbuffer_add_file: '%s' failed\n",
                buffer
            );
        }
    } while (0);

    // send answer
    websocket_send(
        fd,
        FRAME_OPCODE_BIN,
        frame_data
    );

out:
    if (frame_data){
        evbuffer_free(frame_data);
        frame_data = NULL;
    }
    return;
fail:
    goto out;
}

