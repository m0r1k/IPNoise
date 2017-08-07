#ifndef IPNOISE_UDP_H
#define IPNOISE_UDP_H

#include "ipnoise_log.h"

#define MODULE_NAME         "ipnoise_udp"
#define MODULE_VER          "0.1"
#define MODULE_AUTH         "Roman E. Chechnev <ipnoise|d|o|g|chechnev.ru>"
#define MODULE_DESCR        "UDP transport for IPNoise"

#define MDEBUG(fmt, args...) \
        PDEBUG(MODULE_NAME, fmt, ## args);

#define MERROR(fmt, args...) \
        PERROR(MODULE_NAME, fmt, ## args);

#define MWARN(fmt, args...) \
        PWARN(MODULE_NAME, fmt, ## args);

#define MINFO(fmt, args...) \
        PINFO(MODULE_NAME, fmt, ## args);


/*   Check what address is valid
 *   1 - valid
 *   0 - not
 */
static inline int is_valid_ipnoise_udp_addr(const u8 *addr)
{
    // TODO add HOST:PORT address validation
    return 1;
};

#endif

