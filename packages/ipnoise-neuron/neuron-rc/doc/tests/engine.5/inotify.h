#ifndef INOTIFY_H
#define INOTIFY_H

#include <limits.h>
#include "libevent.h"

#define DB_ROOT_DIR         "/mnt/multimedia/git/IPNoise/packages/ipnoise-neuron/neuron-rc/doc/tests/engine/db"
#define DB_OBJECTS_DIR      DB_ROOT_DIR"objects/"
#define DEFAULT_MKDIR_MODE  0755

#define MAX_EVENTS          1024
#define EVENT_SIZE          (sizeof (struct inotify_event))
#define EVENT_BUF_LEN       (MAX_EVENTS * (EVENT_SIZE + PATH_MAX))

typedef struct
{
    int32_t         fd;
    LibEvent        *libevent;
    struct event    *ev;
} Inotify;

void        inotify_free(Inotify *a_val);
Inotify *   inotify_alloc();

void inotify_cb(
    int32_t a_fd,
    short   a_event,
    void    *a_arg
);

#endif

