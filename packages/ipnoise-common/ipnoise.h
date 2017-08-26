#ifndef IPNOISE_COMMON_H
#define IPNOISE_COMMON_H

#ifndef __KERNEL__
#include <string.h>
#else
#include <linux/string.h>
#endif

#define ARPHRD_IPNOISE      0x2210
#define IPNOISE_ALEN        22

// Needed for user-space include
// check what it is free in include/linux/socket.h
// and don't forget update AF_MAX constant
#define AF_HOSTOS 44
#define PF_HOSTOS AF_HOSTOS

// check what it is free in include/uapi/linux/if_tun.h
// bit fields: IFF_TUN, IFF_TAP, IFF_IPNOISE
//#define IFF_IPNOISE	0x0004

// multicast
#define MULTICAST_USER_REAL_NICKNAME "multicast"
#define MULTICAST_USER_HUID "2210:0000:0000:0000:0000:0000:0000:0001"

struct ipnoisehdr
{
    int             magic;
	unsigned char	h_dest[IPNOISE_ALEN];
	unsigned char	h_source[IPNOISE_ALEN];
};

#define IPNOISE_HLEN sizeof(struct ipnoisehdr)

static inline void ipnoise_hdr_init(struct ipnoisehdr *hdr)
{
    memset(hdr, 0x00, sizeof(*hdr));
    hdr->magic = ARPHRD_IPNOISE;
}

static inline int ipnoise_hdr_is_valid(struct ipnoisehdr *hdr)
{
    return ARPHRD_IPNOISE == hdr->magic;
}

#endif

