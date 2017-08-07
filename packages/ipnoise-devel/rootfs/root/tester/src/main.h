#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "ipnoise-common/log_common.h"
#include "ipnoise-common/ipnoise.h"

#define VERSION                 "0.03"

#define ECHO_SERVER_DATA_SIZE   10*1024*1024
#define min(a, b) (a < b) ? a : b

#define TCP_PROTO_NUM           6

#define FD_STDIN                0
#define FD_STDOUT               1
#define FD_STDERR               2

typedef struct
{
    char            type[128];
    char            fname[1024];
    char            content[65535];
    int32_t         content_len;
    int32_t         hostos;
    struct in_addr  addr;
    int32_t         port;
} Conf;

Conf conf;

void set_non_block(
    int32_t a_fd
);

#endif

