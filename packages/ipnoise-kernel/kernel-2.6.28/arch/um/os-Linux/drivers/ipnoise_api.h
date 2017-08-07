#ifndef IPNOISE_API_H
#define IPNOISE_API_H

#include <linux/ipnoise.h>"

#define MODULE_NAME         "ipnoise_api"
#define MODULE_VER          "0.1"
#define MODULE_AUTH         "Roman E. Chechnev <kernel|d|o|g|chechnev.ru>"
#define MODULE_DESCR        "API transport for IPNoise"

#define MDEBUG(fmt, args...) \
        PDEBUG(MODULE_NAME, fmt, ## args);

#define MERROR(fmt, args...) \
        PERROR(MODULE_NAME, fmt, ## args);

#define MWARN(fmt, args...) \
        PWARN(MODULE_NAME, fmt, ## args);

#define MINFO(fmt, args...) \
        PINFO(MODULE_NAME, fmt, ## args);

static int __init ipnoise_api_init(void);
static void __exit ipnoise_api_exit(void);

#endif

