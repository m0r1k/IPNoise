#ifndef ICMP_H
#define ICMP_H

#include <string>
#include "libnet.h"
#include "log.h"

#define IPNOISE_ICMP6_REPLY_ID 0x2210

using namespace std;

// NOTE, max - 56 bytes! maybe less :)
struct icmp_payload
{
    uint32_t    id;
};

class icmpClass
{
    public:
        icmpClass(string &dev_name);
        ~icmpClass();
 
        int ping(
            string src_huid, string dst_huid,
            uint8_t *dst_hw,
            uint32_t id
        );

        int error; 

    private:
        libnet_ptag_t   t;
        string          dev;
        char errbuf[LIBNET_ERRBUF_SIZE];
};

#endif

