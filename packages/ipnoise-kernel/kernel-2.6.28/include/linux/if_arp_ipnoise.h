#ifndef IFARP_IPNOISE_H
#define IFARP_IPNOISE_H

#define ARPHRD_IPNOISE  0x2210  // IPNoise UIN transport
#define IPNOISE_ALEN    22      // IPNoise one link address length

// Needed for user-space include
// must be changed in include/linux/socket.h too
#define AF_HOSTOS 36
#define PF_HOSTOS AF_HOSTOS

/*
 *	This is an IPNoise frame header.
 */
struct ipnoisehdr {
    int             magic;
	unsigned char	h_dest[IPNOISE_ALEN];	/* destination addr	*/
	unsigned char	h_source[IPNOISE_ALEN];	/* source addr	*/
} __attribute__((packed));

#define IPNOISE_HLEN  sizeof(struct ipnoisehdr) // IPNoise header length

#ifdef __KERNEL__
// this will be see only in kernel space
#include <linux/skbuff.h>
#include <linux/string.h>
static inline struct ipnoisehdr *ipnoise_hdr(const struct sk_buff *skb)
{
    return (struct ipnoisehdr *)skb_mac_header(skb);
}
//
#else
//
// for memset
#include <linux/string.h>
#endif

static inline void ipnoise_hdr_init(struct ipnoisehdr *hdr)
{
    memset(hdr, 0x00, sizeof(*hdr));
    hdr->magic = ARPHRD_IPNOISE;
}

static inline int ipnoise_hdr_is_valid(struct ipnoisehdr *hdr)
{
    return hdr->magic == ARPHRD_IPNOISE;
}

#endif

