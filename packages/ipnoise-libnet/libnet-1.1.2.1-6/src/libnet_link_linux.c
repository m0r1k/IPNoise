/*
 *  $Id: libnet_link_linux.c,v 1.5 2004/01/03 20:31:02 mike Exp $
 *
 *  libnet 1.1
 *  libnet_link_linux.c - linux packet socket and pack socket routines
 *
 *  Copyright (c) 1998 - 2004 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include <sys/time.h>

#include <net/if.h>

#if (__GLIBC__)
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#else
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif

#include <arpa/inet.h>
#include "ipnoise-common/ipnoise.h"

#if (HAVE_PACKET_SOCKET)
#ifndef SOL_PACKET
#define SOL_PACKET 263
#endif  /* SOL_PACKET */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#endif  /* HAVE_PACKET_SOCKET */

#include "../include/bpf.h"
#include "../include/libnet.h"

#include "../include/gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif


int
libnet_open_link(libnet_t *l)
{
    struct ifreq ifr;
    int n       = 1;

    if (l == NULL)
    { 
        return (-1);
    } 

#if (HAVE_PACKET_SOCKET)
    l->fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
#else
    l->fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
#endif
    if (l->fd == -1)
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "socket: %s", strerror(errno));
        goto bad;
    }

    memset(&ifr, 0, sizeof (ifr));
    strncpy(ifr.ifr_name, l->device, sizeof (ifr.ifr_name) -1);
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

    if (ioctl(l->fd, SIOCGIFHWADDR, &ifr) < 0 )
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "SIOCGIFHWADDR: %s", strerror(errno));
        goto bad;
    }

    switch (ifr.ifr_hwaddr.sa_family)
    {
        case ARPHRD_IPNOISE:
            l->link_type = ARPHRD_IPNOISE;
            break;
        case ARPHRD_ETHER:
        case ARPHRD_METRICOM:
#ifdef ARPHRD_LOOPBACK
        case ARPHRD_LOOPBACK:   
#endif
            l->link_type = DLT_EN10MB;
            l->link_offset = 0xe;
            break;
        case ARPHRD_SLIP:
        case ARPHRD_CSLIP:
        case ARPHRD_SLIP6:
        case ARPHRD_CSLIP6:
        case ARPHRD_PPP:
            l->link_type = DLT_RAW;
            break;
        case ARPHRD_FDDI:
            l->link_type   = DLT_FDDI;
            l->link_offset = 0x15;
            break;
        /* Token Ring */
        case ARPHRD_IEEE802:
        case ARPHRD_IEEE802_TR:
        case ARPHRD_PRONET:
            l->link_type   = DLT_PRONET;
            l->link_offset = 0x16;
            break;

        default:
            snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "unknown physical layer type 0x%x\n",
                ifr.ifr_hwaddr.sa_family);
        goto bad;
    }
#ifdef SO_BROADCAST
/*
 * man 7 socket
 *
 * Set or get the broadcast flag. When  enabled,  datagram  sockets
 * receive packets sent to a broadcast address and they are allowed
 * to send packets to a broadcast  address.   This  option  has  no
 * effect on stream-oriented sockets.
 */
    if (setsockopt(l->fd, SOL_SOCKET, SO_BROADCAST, &n, sizeof(n)) == -1)
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
		 "%s: set SO_BROADCAST failed: %s\n",
		 __func__, strerror(errno));
        goto bad;
    }
#endif  /*  SO_BROADCAST  */

    return (1);

bad:
    if (l->fd >= 0)
    {
        close(l->fd);
    }
    return (-1);
}


int
libnet_close_link(libnet_t *l)
{
    if (close(l->fd) == 0)
    {
        return (1);
    }
    else
    {
        return (-1);
    }
}


#if (HAVE_PACKET_SOCKET)
static int
get_iface_index(int fd, const int8_t *device)
{
    struct ifreq ifr;
 
    /* memset(&ifr, 0, sizeof(ifr)); */
    strncpy (ifr.ifr_name, device, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
 
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
    {
        return (-1);
    }
 
    return ifr.ifr_ifindex;
}
#endif


int
libnet_write_link(libnet_t *l, u_int8_t *packet, u_int32_t size)
{
    int c;
    struct ipnoisehdr   *ipnoise_hdr = NULL;

#if (HAVE_PACKET_SOCKET)
    // from include/linux/skbuff.h, struct sk_buff, field: "cb"
    //
    // This is the control buffer. It is free to use for every
    // layer. Please put your private variables there. If you
    // want to keep them across layers you have to do a skb_clone()
    // first. This is owned by whoever has the skb queued ATM.
    //
    // struct sockaddr_ll have to liave in sk_buff.cb field,
    // but sockaddr_ll.sll_addr length is only 8 bytes!!
    // are you stupid? why do you have limit l2 addr to 8 bytes,
    // if sk_buff.cb length - 48 bytes!
    //
    // do you think what all l2 addrs can be placed in 8 bytes?
    // 20101113 morik
    struct skb_cb_sockaddr_ll {
        struct sockaddr_ll  sa;
        unsigned char       unused[28];
    } sa_struct;

    struct sockaddr_ll  *sa = (struct sockaddr_ll *)&sa_struct;
    int sa_len              = sizeof(sa_struct);
#else
    struct sockaddr sa_struct;
    struct sockaddr_ll  *sa = &sa_struct;
    int sa_len              = sizeof(sa_struct);
#endif

    // MORIK {
//    fprintf(stderr, "[ MORIK: %s:%d (%s)] sendmsg:\n", __FILE__, __LINE__, __FUNCTION__);
//    ipnoise_hexdump((unsigned char*)packet, (int)size);
    // MORIK }

    if (!l){
        return (-1);
    }

    memset(sa, 0, sa_len);

#if (HAVE_PACKET_SOCKET)
    sa->sll_family    = AF_PACKET;
    sa->sll_ifindex   = get_iface_index(l->fd, l->device);
    if (sa->sll_ifindex == -1)
    {
        return (-1);
    }
    if (l->link_type == ARPHRD_IPNOISE){
        ipnoise_hdr         = (struct ipnoisehdr *)packet;
        sa->sll_protocol    = htons(ETH_P_IPV6);
        sa->sll_hatype      = ARPHRD_IPNOISE;
        sa->sll_halen       = IPNOISE_ALEN;
        memcpy(sa->sll_addr, ipnoise_hdr->h_dest, IPNOISE_ALEN);

//        fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
//        ipnoise_hexdump((unsigned char*)sa->sll_addr, (int)IPNOISE_ALEN);
        packet  += sizeof(struct ipnoisehdr);
        size    -= sizeof(struct ipnoisehdr);
    } else {
        sa->sll_protocol = htons(ETH_P_ALL);
    }
#else
	strncpy(sa->sa_data, l->device, sizeof (sa->sa_data) - 1);
    sa->sa_data[sizeof (sa->sa_data) - 1] = 0;
#endif

    c = sendto(l->fd, packet, size, 0,
        (struct sockaddr *)sa, sa_len);
    if (c != size)
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "libnet_write_link(): only %d bytes written (%s)\n", c,
                strerror(errno));
    }
    return (c);
}

struct libnet_ether_addr *
libnet_get_hwaddr(libnet_t *l)
{
    int fd;
    struct ifreq ifr;
    struct libnet_ether_addr *eap;
    /*
     *  XXX - non-re-entrant!
     */
    static struct libnet_ether_addr ea;

    if (l == NULL)
    { 
        return (NULL);
    } 

    if (l->device == NULL)
    {           
        if (libnet_select_device(l) == -1)
        {   
            snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                    "libnet_get_hwaddr: can't figure out a device to use\n");
            return (NULL);
        }
    }

    /*
     *  Create dummy socket to perform an ioctl upon.
     */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "socket: %s", strerror(errno));
        goto bad;
    }

    memset(&ifr, 0, sizeof(ifr));
    eap = &ea;
   	strncpy(ifr.ifr_name, l->device, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

    if (ioctl(fd, SIOCGIFHWADDR, (int8_t *)&ifr) < 0)
    {
        close(fd);
        snprintf(l->err_buf, LIBNET_ERRBUF_SIZE,
                "ioctl: %s", strerror(errno));
        goto bad;
    }
    memcpy(eap, &ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
    close(fd);
    return (eap);

bad:
    return (NULL);
}


/* EOF */
