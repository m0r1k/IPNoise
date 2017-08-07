#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <stdint.h>

#include "libevent.h"
#include "http_request.h"

enum {
    PLAY_MODE_ECHO  = 0,
    PLAY_MODE_FILES,
    PLAY_MODE_SNAP_SHOT
};

int32_t create_snapshot_to_file(
    const char *a_path
);

int32_t create_snapshot(
    struct evbuffer *a_ev
);

#endif

