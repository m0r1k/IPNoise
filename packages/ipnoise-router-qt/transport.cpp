#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "transport.h"
#include "main.h"

extern Instance *inst;

#define MAX_HUID_TEXT_LEN   128
#define MAX_PACKET_SIZE     65536

void Transport::readUdp(int socket)
{
    char buffer[MAX_PACKET_SIZE];
    char from[MAX_HUID_TEXT_LEN];
    char to[MAX_HUID_TEXT_LEN];
    char *ptr = buffer;
    int res;

    sk_buff         *skb    = NULL;
    struct ip6_hdr  *ipv6   = NULL;
    struct udphdr   *udp    = NULL;

    uint32_t udp_src_port   = 0;
    uint32_t udp_dst_port   = 0;
    int32_t  udp_data_len   = 0;
    uint32_t udp_check      = 0;

    uint32_t ip_ver         = 0;

    memset(buffer, 0x00, sizeof(buffer));
    res = (int)recvfrom(socket, buffer, sizeof(buffer), 0, NULL, NULL);
    if (res <= 0){
        goto ret;
    }
    
    // goto upper protocol
    ptr += IPNOISE_HLEN;
    res -= IPNOISE_HLEN;

    ip_ver = (ptr[0] & 0xf0) >> 4;

    // prepare ipv6
    ipv6 = (struct ip6_hdr *)ptr;

    // check ipv6
    if (ip_ver != 6){
        goto ret;
    }

    // check what is UDP
    if (ipv6->ip6_nxt != IPPROTO_UDP){
        // it's not udp protocol
        goto ret;
    }

    // goto upper protocol
    ptr += sizeof(struct ip6_hdr);
    res -= sizeof(struct ip6_hdr);
    if (res <= 0){
        // invalid packet size
        goto ret;
    }

    // prepare udp
    udp = (struct udphdr *)ptr;

    // move ptr to udp data
    ptr += sizeof(struct udphdr);
    res -= sizeof(struct udphdr);

    // good we have udp packet
    udp_src_port = htons(udp->source);
    udp_dst_port = htons(udp->dest);
    udp_data_len = htons(udp->len) - sizeof(struct udphdr);
    udp_check    = htons(udp->check);

    if (res > udp_data_len){
        // XXX TODO
        // looks like we have bug in kernel "udp" transport
        // some time we have recived buffer with spare data
        // (res > udp_data_len)
        // now in this case, we will remove garbage
        res = udp_data_len;
        ((char *)udp + sizeof(struct udphdr) + udp_data_len)[0] = 0x00;
    };

    // check errors
    if (res != udp_data_len){
        // invalid packet size
        goto ret;
    }

    if (udp_dst_port != IPNOISE_UDP_PORT){
        goto ret;
    }

    // hexdump((unsigned char*)ptr, res);

    PDEBUG(15, "Was received UDP packet from: '%s'\n", ifname.toAscii().data());
    PDEBUG(15, "UDP source port: '%d'\n", udp_src_port);
    PDEBUG(15, "UDP dest   port: '%d'\n", udp_dst_port);
    PDEBUG(15, "UDP len:         '%d'\n", udp_data_len);
    PDEBUG(15, "UDP check:       '%d'\n", udp_check);

    inet_ntop(AF_INET6, &ipv6->ip6_src, from, sizeof(from));
    inet_ntop(AF_INET6, &ipv6->ip6_dst, to,   sizeof(to));

    skb = new sk_buff();
    if (skb == NULL){
        PERROR("Cannot allocate new skb\n");
        goto fail;
    }
    
    // received packet
    skb->inbuffer       = ptr;
    skb->indev_name     = ifname;
    skb->indev_index    = ifindex;
    skb->from           = from;
    skb->to             = to;
    skb->from_ll        = buffer;

    inst->server->netif_rx(skb);

ret:
    return;
fail:
    goto ret;
}

int Transport::sendPacket(sk_buff *skb)
{
    QString src_huid    = skb->from;
    QString dst_huid    = skb->to;
    uint8_t *dst_hw     = (uint8_t *)skb->to_ll.toAscii().data();

    PDEBUG(15, "-------------> OUTGOING FROM: '%s'\n", ifname.toAscii().data());
    PDEBUG(15, "-------------> TO:   '%s'\n", skb->to.toAscii().data());
    PDEBUG(15, "-------------> LL:   '%s'\n", dst_hw);
    PDEBUG(15, "-------------> FROM: '%s'\n", skb->from.toAscii().data());

    int res, err = 1;
    struct libnet_in6_addr dst_ip;
    struct libnet_in6_addr src_ip;
    char srcname[100],dstname[100];

    if (l == NULL){
        goto fail;
    }

    dst_ip = libnet_name2addr6(l, (char *)dst_huid.toAscii().data(), 1);
    res = strncmp((char*)&dst_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (res == 0){
        PERROR ("Bad IP6 dst address: %s\n", dst_huid.toAscii().data());
        goto fail;
    }

    src_ip = libnet_name2addr6(l, (char *)src_huid.toAscii().data(), 1);
    res = strncmp((char*)&src_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (res == 0){
        PERROR ("Bad IP6 src address: %s\n", src_huid.toAscii().data());
        err = 1;
        goto fail;
    }

    libnet_seed_prand(l);
    libnet_addr2name6_r(src_ip, 1, srcname,sizeof(srcname));
    libnet_addr2name6_r(dst_ip, 1, dstname,sizeof(dstname));

    t = LIBNET_PTAG_INITIALIZER;

    // clear packet
    libnet_clear_packet(l);

    libnet_build_udp(
            2210,                                           // src port
            2210,                                           // dst port
            sizeof(struct udphdr) + skb->outbuffer.size(),  // total len
            0,                                              // check sum
            (u_int8_t *)skb->outbuffer.toAscii().data(),    // payload
            skb->outbuffer.size(),                          // payload_s
            l,
            t);

    libnet_build_ipv6(
            0,                                              // flow label
            0,                                              // traffic class
            sizeof(struct udphdr) + skb->outbuffer.size(),  // size
            IPPROTO_UDP,                                    // next header
            64,                                             // hops limit
            src_ip,                                         // ip src
            dst_ip,                                         // id dst
            NULL,                                           // payload
            0,                                              // payload_s
            l,
            t);

    libnet_build_ipnoise(
            (u_int8_t*)(dst_hw),                            // dst
            NULL,                                           // payload
            0,                                              // payload size
            l,                                              // libnet handle
            t);

    // PDEBUG(5, "%15s/%d -> %15s/%d\n",
    //    srcname,
    //    128,        // source netmask
    //    dstname,
    //    128         // destination mask
    // );

    // send packet
    res = libnet_write(l);
    if (res == -1){
        PERROR("failed libnet_write: %s\n", libnet_geterror(l));
        err = 1;
        goto fail;
    }

    PDEBUG(15, "libnet_write, was wroted: '%d' bytes\n", res);
    err = 0;

ret:
    return err;

fail:
    err = 1;
    goto ret;
}

Transport::Transport(QString _ifname, int _ifindex)
{
    int res;
    ifname          = _ifname;
    ifindex         = _ifindex;
    udp             = NULL;
    sd              = -1;
    l               = NULL;

    struct sockaddr_ll s_ll;

    PDEBUG(5, "New interface was added for watching: '%s'\n",
         ifname.toAscii().data());

    sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    memset((void *)&s_ll, 0, sizeof(s_ll));
    s_ll.sll_family     = PF_PACKET;        // тип сокета
    s_ll.sll_protocol   = htons(ETH_P_ALL); // тип принимаемого протокола
    s_ll.sll_ifindex    = ifindex;          // индекс сетевого интерфейса

    res = bind(sd, (struct sockaddr *)&s_ll, sizeof(struct sockaddr_ll));
    if (res < 0){
        PERROR("bind() for device: '%s' (ifindex: '%d') failed\n",
            ifname.toAscii().data(),
            ifindex
        );
        goto fail;
    }

    res = fcntl(sd, F_SETFL, O_NONBLOCK);
    if (res == -1){
       PERROR("fcntl()\n");
       goto fail;
    }

    udp = new QSocketNotifier(sd, QSocketNotifier::Read, this);
    connect(udp, SIGNAL(activated(int)), this, SLOT(readUdp(int)));
    udp->setEnabled(true);

    // init libnet
    // Initialize the library.  Root priviledges are required.
    l = libnet_init(
        LIBNET_LINK,                        // injection type
        (char *)ifname.toAscii().data(),    // network interface
        errbuf                              // error buffer
    );
    if (l == NULL){
        PERROR("libnet_init() failed for dev: '%s' (%s)\n",
            ifname.toAscii().data(), errbuf);
        goto fail;
    }

fail:
    ;
};


Transport::~Transport()
{
    qDebug() << "Interface was removed from watching: "
             <<  ifname.toUtf8() << endl;
    
    disconnect(this, 0, 0, 0);

    if (udp != NULL){
        delete udp;
        udp = NULL;
    }
    if (sd >= 0){
        ::close(sd);
        sd  = -1;
    }
    if (l != NULL){
        libnet_destroy(l);
        l = NULL;
    }
};


