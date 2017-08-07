#include <map>
#include "icmp.h"

// dev_name, libnet
map <string, libnet_t *>           libnets;
map <string, libnet_t *>::iterator libnets_i;

int icmpClass::ping(string src_huid, string dst_huid,
    uint8_t *dst_hw, uint32_t id)
{
    int res,i,err = 1;
    struct libnet_in6_addr dst_ip;
    struct libnet_in6_addr src_ip;
    struct icmp_payload payload;
    char srcname[100],dstname[100];
    libnet_t *l = NULL;

    // prepare payload
    payload.id = id;

    libnets_i = libnets.find(dev);
    if (libnets_i == libnets.end()){
        PDEBUG(5, "Cannot find opened libnet for device: '%s'\n",
            dev.c_str());
        goto fail;
    }
    l = libnets_i->second;

    dst_ip = libnet_name2addr6(l, (char *)dst_huid.c_str(), 1);
    res = strncmp((char*)&dst_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (res == 0){
        PERROR ("Bad IP6 dst address: %s\n", dst_huid.c_str());
        goto fail;
    }

    src_ip = libnet_name2addr6(l, (char *)src_huid.c_str(), 1);
    res = strncmp((char*)&src_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (res == 0){
        PERROR ("Bad IP6 src address: %s\n", src_huid.c_str());
        err = 1;
        goto fail;
    }

    libnet_seed_prand(l);
    libnet_addr2name6_r(src_ip, 1, srcname,sizeof(srcname));
    libnet_addr2name6_r(dst_ip, 1, dstname,sizeof(dstname));

    t = LIBNET_PTAG_INITIALIZER;

    // clear packet
    libnet_clear_packet(l);

    libnet_build_icmpv6_echo(
            ICMP6_ECHO,                     /* type */
            0,                              /* code */
            0,                              /* cksum */
            htons(IPNOISE_ICMP6_REPLY_ID),  /* id */
            0x0001,                         /* seq */
            (u_int8_t*)&payload,            /* payload */
            sizeof(payload),                /* payload_s */
            l,                              /* libnet handle */
            t);                             /* ptag */

    libnet_build_ipv6(
            0,                      /* flow label */
            0,                      /* traffic class */
            LIBNET_ICMPV6_ECHO_H + sizeof(payload),   /* size */
            IPPROTO_ICMP6,          /* next header */
            64,                     /* hops limit */
            src_ip,                 /* ip src */
            dst_ip,                 /* id dst */
            NULL,                   /* payload */
            0,                      /* payload_s */
            l,
            t);

    libnet_build_ipnoise(
            (u_int8_t*)(dst_hw),        /* dst */
            NULL,                       /* payload */
            0,                          /* payload size */
            l,                          /* libnet handle */
            t);

    //PDEBUG(5, "%15s/%5d -> %15s/%5d\n",
    //    srcname,
    //    128,        // source netmask
    //    dstname,
    //    128         // destination mask
    //);

    // send packet
    res = libnet_write(l);
    if (res == -1){
        PDEBUG(5, "libnet_write: %s\n", libnet_geterror(l));
        err = 1;
        goto fail;
    }

    PDEBUG(7, "libnet_write, was wroted: '%d' bytes\n", res);
    err = 0;

ret:
    return err;

fail:
    err = 1;
    goto ret;
}

icmpClass::icmpClass(string &dev_name)
{
    error       = 1;
    dev         = dev_name;
    libnet_t *l = NULL;

    if (!dev.size()){
        goto ret;
    }

    libnets_i = libnets.find(dev);
    if (libnets_i == libnets.end()){

        // Initialize the library.  Root priviledges are required.
        l = libnet_init(
            LIBNET_LINK,            // injection type
            (char *)dev.c_str(),    // network interface
            errbuf                  // error buffer
        );
        if (l == NULL){
            PDEBUG(5, "libnet_init() failed for dev: '%s' (%s)\n",
                dev.c_str(), errbuf);
            goto ret;
        }

        // store libnet for it device
        libnets[dev] = l;
    }

    error = 0;

ret:
    ;
}

icmpClass::~icmpClass()
{
    libnet_t *l = NULL;

again:
    for (libnets_i = libnets.begin();
        libnets_i != libnets.end();
        libnets_i++)
    {
        l = libnets_i->second;
        libnet_destroy(libnets_i->second);
        libnets.erase(libnets_i);
        l = NULL;
        goto again;
    }
}

