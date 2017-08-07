#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#if (!(_WIN32) || (__CYGWIN__))
#include "../include/libnet.h"
#else
#include "../include/win32/libnet.h"
#endif

#include "ipnoise-common/ipnoise.h"

libnet_ptag_t
libnet_build_ipnoise(u_int8_t *dst,
u_int8_t *payload, u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct ipnoisehdr _ipnoise_hdr;
    ipnoise_hdr_init(&_ipnoise_hdr);

    if (l == NULL)
    {
        return (-1);
    }

    n = IPNOISE_HLEN + payload_s;
    h = 0;

    /*
     *  Find the existing protocol block if a ptag is specified, or create
     *  a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_IPNOISE_H);
    if (p == NULL)
    {
        return (-1);
    }

    // dest address
    memcpy(_ipnoise_hdr.h_dest, dst, sizeof(_ipnoise_hdr.h_dest));

    n = libnet_pblock_append(l, p, (u_int8_t *)&_ipnoise_hdr, IPNOISE_HLEN);
    if (n == -1)
    {
        goto bad;
    }

    if ((payload && !payload_s) || (!payload && payload_s))
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "%s(): payload inconsistency\n", __func__);
        goto bad;
    }

    if (payload && payload_s)
    {
        n = libnet_pblock_append(l, p, payload, payload_s);
        if (n == -1)
        {
            goto bad;
        }
    }

    return (ptag ? ptag : libnet_pblock_update(l, p, h, LIBNET_PBLOCK_IPNOISE_H));

bad:
    libnet_pblock_delete(l, p);
    return (-1);
}

