#ifndef IPNOISE_ICQ_H
#define IPNOISE_ICQ_H

#define MODULE_NAME         "ipnoise_icq"
#define MODULE_VER          "0.1"
#define MODULE_AUTH         "Roman E. Chechnev <kernel|d|o|g|chechnev.ru>"
#define MODULE_DESCR        "ICQ transport for IPNoise"

/*   Check what address is valid
 *   1 - valid
 *   0 - not
 */
static inline int is_valid_icq_addr(const u8 *addr)
{
    // TODO add ICQ UIN validation
    return 1;
};

#endif

