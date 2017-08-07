#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <ipnoise-common/log_common.h>

#include "inotify.h"

void inotify_free(
    Inotify *a_val)
{
    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    if (0 <= a_val->fd){
        close(a_val->fd);
        a_val->fd = -1;
    }

    if (a_val->ev){
        event_free(a_val->ev);
        a_val->ev = NULL;
    }

    free(a_val);
}

Inotify * inotify_alloc(
    LibEvent *a_libevent)
{
    int32_t     res     = 0;
    Inotify     *ret    = NULL;
    uint32_t    size    = sizeof(*ret);

    if (!a_libevent){
        PFATAL("missing argument: 'a_libevent'\n");
    }

    ret = (Inotify *)calloc(1, size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was need: '%d' byte(s)",
            size
        );
        goto fail;
    }

    ret->libevent = a_libevent;

    res = inotify_init_dirs();
    if (res){
        PERROR("inotify_init_dirs()\n");
        goto fail;
    }

    ret->fd = inotify_init();
    if (0 > ret->fd){
        PERROR("inotify_init() failed\n");
        goto fail;
    }

    ret->ev = libevent_add_event(
        a_libevent,
        ret->fd,
        EV_READ | EV_PERSIST,
        inotify_cb,
        a_libevent
    );

    inotify_add_watch(
        ret->fd,
        DB_OBJECTS_DIR,
        IN_CREATE | IN_DELETE
    );

out:
    return ret;
fail:
    if (ret){
        inotify_free(ret);
        ret = NULL;
    }
    goto out;
}

void inotify_cb(
    int32_t a_fd,
    short   a_event,
    void    *a_arg)
{
    char    buffer[EVENT_BUF_LEN]   = { 0x00 };
    int32_t length                  = 0;
    int32_t i                       = 0;

    length = read(a_fd, buffer, EVENT_BUF_LEN);

    // checking for error
    if (length < 0){
        perror("read");
    }

    while (i < length){
        struct inotify_event *event = NULL;
        event = (struct inotify_event *)&buffer[i];
        if (event->len){
            if (event->mask & IN_CREATE){
                if (event->mask & IN_ISDIR){
                    PINFO("New directory %s created.\n",
                        event->name
                    );
                } else {
                    PINFO("New file %s created.\n",
                        event->name
                    );
                }
            } else if (event->mask & IN_DELETE){
                if (event->mask & IN_ISDIR){
                    PINFO("Directory %s deleted.\n",
                        event->name
                    );
                } else {
                    PINFO("File %s deleted.\n", event->name);
                }
            }
        }
        i += EVENT_SIZE + event->len;
    }

    // inotify_rm_watch(a_fd, wd);
    // close(a_fd);
}

int32_t mkdir(
    char    *a_path,
    int32_t a_mode)
{
    struct stat st;

    res = stat(a_path, &st);

    res = mkdir(
        DB_OBJECTS_DIR,
        DEFAULT_MKDIR_MODE
    );

}

int32_t inotify_init_dirs()
{
    int32_t     res, err = -1;

    if (res){
        PERROR("mkdir: '%s' failed\n",
            DB_OBJECTS_DIR
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

