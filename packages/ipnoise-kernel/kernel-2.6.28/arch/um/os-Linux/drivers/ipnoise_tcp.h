#ifndef IPNOISE_TCP_H
#define IPNOISE_TCP_H

#include "ipnoise_log.h"

#define MODULE_NAME         "ipnoise_tcp"
#define MODULE_VER          "0.2"
#define MODULE_AUTH         "Roman E. Chechnev <ipnoise|d|o|g|chechnev.ru>"
#define MODULE_DESCR        "TCP transport for IPNoise"

#define MDEBUG(fmt, args...) \
        PDEBUG(MODULE_NAME, fmt, ## args);

#define MERROR(fmt, args...) \
        PERROR(MODULE_NAME, fmt, ## args);

#define MWARN(fmt, args...) \
        PWARN(MODULE_NAME, fmt, ## args);

#define MINFO(fmt, args...) \
        PINFO(MODULE_NAME, fmt, ## args);


#define MAX_CLIENTS     64
#define BUFFER_SIZE     65535

#define CLIENT_STATE_FREE           0   // Unused structure
#define CLIENT_STATE_ACCEPT         1   // Prepare accept
#define CLIENT_STATE_CONNECTING     2   // Client trying to connect
#define CLIENT_STATE_CONNECTED      3   // Client was connected to daemon

#define BUFFER_STATE_READ       0   // collect packet in buffer
#define BUFFER_STATE_WAIT_NEXT  1   // wait start of next packet

#define USED_MAGIC 0x221022

struct client
{
    int     used;                   // used flag
    int     os_sd;                  // host socket descriptor
    int     state;                  // client state
    struct  sockaddr_in os_saddr;   // remote part's address
    int     saddr_len;              // size of address
    char    *buffer_start;          // buffer for incoming data
    char    *buffer_ptr;            // buffer ptr
    char    *buffer_end;            // buffer end ptr
    int     buffer_state;           // buffer state
};

/*   Check what address is valid
 *   1 - valid
 *   0 - not
 */
static inline int is_valid_ipnoise_tcp_addr(const u8 *addr)
{
    // TODO add HOST:PORT address validation
    return 1;
};

#endif

