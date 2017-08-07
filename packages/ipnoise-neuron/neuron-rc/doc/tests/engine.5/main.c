#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <ipnoise-common/log_common.h>
#include "libevent.h"
#include "inotify.h"
#include "main.h"

int32_t main(void)
{
    int32_t     err         = -1;
    LibEvent    *libevent   = NULL;
    Inotify     *inotify    = NULL;

    libevent = libevent_alloc();
    if (!libevent){
        PERROR("libevent_alloc() failed\n");
        goto fail;
    }

    inotify = inotify_alloc(libevent);
    if (!inotify){
        PERROR("inotify_alloc() failed\n");
        goto fail;
    }

    libevent_loop_start(
        libevent,
        EVLOOP_NO_EXIT_ON_EMPTY
    );

    // all ok
    err = 0;

out:
    if (inotify){
        inotify_free(inotify);
        inotify = NULL;
    }
    if (libevent){
        libevent_free(libevent);
        libevent = NULL;
    }
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

