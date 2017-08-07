#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "libevent.h"

void parse(
    struct evbuffer *a_evb
);

int32_t file_read(
    const char      *a_path,
    struct evbuffer *a_evb
);

int32_t main(
    int32_t a_argc,
    char    **a_argv
);

#endif

