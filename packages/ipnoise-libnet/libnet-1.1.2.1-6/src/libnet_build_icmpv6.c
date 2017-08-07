/*
 *  $Id: libnet_build_icmpv6.c,v 1.0 2006/05/09 18:24:34 clem1 Exp $
 *
 *  libnet
 *  libnet_build_icmpv6.c - ICMP6 packet assemblers
 *
 *  Copyritht (c) 2006 Clément Lecigne <clem1@FreeBSD.org>
 *  Copyright (c) 1998 - 2004 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#if (!(_WIN32) || (__CYGWIN__)) 
#include "../include/libnet.h"
#else
#include "../include/win32/libnet.h"
#endif

libnet_ptag_t
libnet_build_icmpv6_echo(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int16_t id, u_int16_t seq, u_int8_t *payload, u_int32_t payload_s,
libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_ECHO_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_ECHO_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_ECHO_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_id = htons(id);   /* packet id */
    icmp_hdr.icmp_seq = htons(seq); /* packet seq */

    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_ECHO_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_ECHO_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_parampb(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t pointer, u_int8_t *payload, u_int32_t payload_s,
libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_PARAMPB_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_PARAMPB_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_PARAMPB_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_pointer = htonl(pointer);   /* pointer */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_PARAMPB_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_PARAMPB_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}  

libnet_ptag_t
libnet_build_icmpv6_timxceed(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t unused, u_int8_t *payload, u_int32_t payload_s,
libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_TIMXCEED_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_TIMXCEED_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_TIMXCEED_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_unused = htonl(unused);   /* pointer */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_TIMXCEED_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_TIMXCEED_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_unreach(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t unused, u_int8_t *payload, u_int32_t payload_s,
libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_UNREACH_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_UNREACH_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_UNREACH_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_unused = htonl(unused);   /* pointer */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_UNREACH_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_UNREACH_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_toobig(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t mtu, u_int8_t *payload, u_int32_t payload_s,
libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_TOOBIG_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_TOOBIG_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_TOOBIG_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_mtu = htonl(mtu);   /* pointer */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_TOOBIG_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_TOOBIG_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_redirect(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t unused, struct libnet_in6_addr target, struct libnet_in6_addr dst, 
u_int8_t *payload, u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_REDIRECT_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_REDIRECT_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_REDIRECT_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_unused = htonl(unused);   /* pointer */
    for (i = 0; i < 8; i++)
    {
        icmp_hdr.icmp_target1[i] = target.libnet_s6_addr[i];  /* target1 */
    }
    for (n = 0; n < 8; n++)
    {
        icmp_hdr.icmp_target2[n] = target.libnet_s6_addr[i++]; /* target2 */
    }
    icmp_hdr.icmp_dst = dst;        /* dst */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_REDIRECT_H);
    
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

    /*
     * Some shits around icmpv6 option and related to checksumming
     */
    if (p->prev && sum == 0)
    {
        switch(p->prev->type)
        {
            case LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H:
            case LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H:
                h += LIBNET_ICMPV6_OPT_TLLA_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_PI_H:
                h += LIBNET_ICMPV6_OPT_PI_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_RH_H:
                h += LIBNET_ICMPV6_OPT_RH_H;
                break;
             case LIBNET_PBLOCK_ICMPV6_OPT_MTU_H:
                h += LIBNET_ICMPV6_OPT_MTU_H;
                break;
             default:
                break;
        }
    }
    
    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_REDIRECT_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}


libnet_ptag_t
libnet_build_icmpv6_ni(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int16_t qtype, u_int16_t flags, u_int8_t *nonce, u_int8_t *payload, 
u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_NI_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_NI_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_NI_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_qtype = htonl(qtype);   /* qtype */
    icmp_hdr.icmp_flags = htonl(flags);   /* flags */
    for (i = 0; i < 8; i++)
    {
        icmp_hdr.icmp_nonce[i] = nonce[i];  /* nonce */
    }
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_NI_H);
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

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_NI_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_ra(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int8_t chl, u_int8_t m, u_int8_t o, u_int16_t rlf, u_int32_t rct,
u_int16_t rtt, u_int8_t *payload, 
u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_RA_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_RA_H + payload_s; /* hl for checksum */
    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_ROUTERADV_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_chl = htons(chl);   /* current hop limit */
    icmp_hdr.icmp_mo = (m << 7) + (o << 6); /* managed & other bits */
    icmp_hdr.icmp_rlf = htons(rlf);     /* router lifetime */
    icmp_hdr.icmp_rct = htonl(rct);     /* recheable time */
    icmp_hdr.icmp_rtt = htonl(rtt);     /* retrans time */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_RA_H);
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

    /*
     * Some shits around icmpv6 option and related to checksumming
     */
    if (p->prev && sum == 0)
    {
        switch(p->prev->type)
        {
            case LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H:
            case LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H:
                h += LIBNET_ICMPV6_OPT_TLLA_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_PI_H:
                h += LIBNET_ICMPV6_OPT_PI_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_RH_H:
                h += LIBNET_ICMPV6_OPT_RH_H;
                break;
             case LIBNET_PBLOCK_ICMPV6_OPT_MTU_H:
                h += LIBNET_ICMPV6_OPT_MTU_H;
                break;
             default:
                break;
        }
    } 
    
    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_ROUTERADV_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}


libnet_ptag_t
libnet_build_icmpv6_rs(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t unused, u_int8_t *payload, u_int32_t payload_s, libnet_t *l, 
libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_RS_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_RS_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_ROUTERSO_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_unused = htonl(unused);   /* unused field */
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_RS_H);
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

    /*
     * Some shits around icmpv6 option and related to checksumming
     */
    if (p->prev && sum == 0)
    {
        switch(p->prev->type)
        {
            case LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H:
            case LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H:
                h += LIBNET_ICMPV6_OPT_TLLA_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_PI_H:
                h += LIBNET_ICMPV6_OPT_PI_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_RH_H:
                h += LIBNET_ICMPV6_OPT_RH_H;
                break;
             case LIBNET_PBLOCK_ICMPV6_OPT_MTU_H:
                h += LIBNET_ICMPV6_OPT_MTU_H;
                break;
             default:
                break;
        }
    }
    
    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_ROUTERSO_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_ns(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int32_t reserved, struct libnet_in6_addr target, u_int8_t *payload, 
u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_NS_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_NS_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_NEIGHBORSO_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_reserved4 = htonl(reserved);   /* unused field */
    for (i = 0; i < 8; i++)
    {
        icmp_hdr.icmp_target1[i] = target.libnet_s6_addr[i];  /* target1 */
    }
    for (n = 0; n < 8; n++)
    {
        icmp_hdr.icmp_target2[n] = target.libnet_s6_addr[i++]; /* target2 */
    }

    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_NS_H);
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

    /*
     * Some shits around icmpv6 option and related to checksumming
     */
    if (p->prev && sum == 0)
    {
        switch(p->prev->type)
        {
            case LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H:
            case LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H:
                h += LIBNET_ICMPV6_OPT_TLLA_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_PI_H:
                h += LIBNET_ICMPV6_OPT_PI_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_RH_H:
                h += LIBNET_ICMPV6_OPT_RH_H;
                break;
             case LIBNET_PBLOCK_ICMPV6_OPT_MTU_H:
                h += LIBNET_ICMPV6_OPT_MTU_H;
                break;
             default:
                break;
        }
    }
    
    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_NEIGHBORSO_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_na(u_int8_t type, u_int8_t code, u_int16_t sum,
u_int8_t router, u_int8_t solicited, u_int8_t override, 
struct libnet_in6_addr target, u_int8_t *payload, 
u_int32_t payload_s, libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_hdr icmp_hdr;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_NA_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_NA_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_NEIGHBORADV_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = type;  /* packet type */
    icmp_hdr.icmp_code = code;  /* packet code */
    icmp_hdr.icmp_sum = (sum ? htons(sum) : 0); /* checksum */
    icmp_hdr.icmp_rso = htonl((router << 31) + (solicited << 30) + (override << 29));   /* router, solicited and override bits */
    for (i = 0; i < 8; i++)
    {
        icmp_hdr.icmp_target1[i] = target.libnet_s6_addr[i];  /* target1 */
    }
    for (n = 0; n < 8; n++)
    {
        icmp_hdr.icmp_target2[n] = target.libnet_s6_addr[i++]; /* target2 */
    }

    n = libnet_pblock_append(l, p, (u_int8_t *)&icmp_hdr, LIBNET_ICMPV6_NA_H);
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

    /*
     * Some shits around icmpv6 option and related to checksumming
     */
    if (p->prev && sum == 0)
    {
        switch(p->prev->type)
        {
            case LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H:
            case LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H:
                h += LIBNET_ICMPV6_OPT_TLLA_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_PI_H:
                h += LIBNET_ICMPV6_OPT_PI_H;
                break;
            case LIBNET_PBLOCK_ICMPV6_OPT_RH_H:
                h += LIBNET_ICMPV6_OPT_RH_H;
                break;
             case LIBNET_PBLOCK_ICMPV6_OPT_MTU_H:
                h += LIBNET_ICMPV6_OPT_MTU_H;
                break;
             default:
                break;
        }
    }          

    if (sum == 0)
    {
        /*
         *  If checksum is zero, by default libnet will compute a checksum
         *  for the user.  The programmer can override this by calling
         *  libnet_toggle_checksum(l, ptag, 1);
         */
        libnet_pblock_setflags(p, LIBNET_PBLOCK_DO_CHECKSUM);
    }
    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_NEIGHBORADV_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_opt_tlla(u_int8_t type, u_int8_t length,
        u_int8_t *target, u_int8_t *payload, u_int32_t payload_s, 
        libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_opt_lla_hdr opt;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_OPT_TLLA_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_OPT_TLLA_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&opt, 0, sizeof(opt));
    opt.type = type;  /* option type */
    opt.len = length;     /* header len */
    for (i = 0; i < 8; i++)
    {
        opt.addr[i] = target[i];  /* target */
    }
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&opt, LIBNET_ICMPV6_OPT_TLLA_H);
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

    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_OPT_TLLA_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_opt_slla(u_int8_t type, u_int8_t length,
        u_int8_t *source, u_int8_t *payload, u_int32_t payload_s, 
        libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h, i;
    libnet_pblock_t *p;
    struct libnet_icmpv6_opt_lla_hdr opt;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_OPT_SLLA_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_OPT_SLLA_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&opt, 0, sizeof(opt));
    opt.type = type;  /* option type */
    opt.len = length;  /* packet code */
    for (i = 0; i < 8; i++)
    {
        opt.addr[i] = source[i];  /* target */
    }
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&opt, LIBNET_ICMPV6_OPT_SLLA_H);
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

    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_OPT_SLLA_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_opt_pi(u_int8_t type, u_int8_t length, u_int8_t prefixlen,
        u_int8_t ol, u_int8_t a, u_int32_t validlt, u_int32_t preferredlt,
        u_int32_t reserved, struct libnet_in6_addr prefix,
        u_int32_t payload_s, u_int8_t *payload, libnet_t *l, 
        libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_opt_pi_hdr opt;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_OPT_PI_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_OPT_PI_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_OPT_PI_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&opt, 0, sizeof(opt));
    opt.type = type;  /* option type */
    opt.len = length;  /* packet header len */
    opt.prefixlen = prefixlen;
    opt.la = (ol << 7) + (a << 6);
    opt.validlt = validlt;
    opt.preferredlt = preferredlt;
    opt.reserved = reserved;
    opt.prefix = prefix;
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&opt, LIBNET_ICMPV6_OPT_PI_H);
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

    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_OPT_PI_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_opt_rh(u_int8_t type, u_int8_t length, u_int16_t r1,
        u_int32_t r2, u_int32_t payload_s, u_int8_t *payload, libnet_t *l, 
        libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_opt_rh_hdr opt;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_OPT_RH_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_OPT_RH_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_OPT_RH_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&opt, 0, sizeof(opt));
    opt.type = type;  /* option type */
    opt.len = length;  /* packet header len */
    opt.reserved1 = r1;
    opt.reserved2 = r2;
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&opt, LIBNET_ICMPV6_OPT_RH_H);
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

    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_OPT_RH_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}

libnet_ptag_t
libnet_build_icmpv6_opt_mtu(u_int8_t type, u_int8_t length, u_int16_t r,
        u_int32_t mtu, u_int32_t payload_s, u_int8_t *payload,
        libnet_t *l, libnet_ptag_t ptag)
{
    u_int32_t n, h;
    libnet_pblock_t *p;
    struct libnet_icmpv6_opt_mtu_hdr opt;

    if(l == NULL)
    {
        return (-1);
    }

    n = LIBNET_ICMPV6_OPT_MTU_H + payload_s; /* size of memory block */
    h = LIBNET_ICMPV6_OPT_MTU_H + payload_s; /* hl for checksum */

    /*
     * Find the existing protocol block if a ptag is specified, or create
     * a new one.
     */
    p = libnet_pblock_probe(l, ptag, n, LIBNET_PBLOCK_ICMPV6_OPT_MTU_H);
    if(p == NULL)
    {
        return (-1);
    }

    memset(&opt, 0, sizeof(opt));
    opt.type = type;  /* option type */
    opt.len = length;  /* packet header len */
    opt.reserved = r;
    opt.mtu = mtu;
    
    n = libnet_pblock_append(l, p, (u_int8_t *)&opt, LIBNET_ICMPV6_OPT_MTU_H);
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

    return (ptag ? ptag : libnet_pblock_update(l, p, h, 
            LIBNET_PBLOCK_ICMPV6_OPT_MTU_H));
bad:
    libnet_pblock_delete(l, p);   
    return (-1);
}
