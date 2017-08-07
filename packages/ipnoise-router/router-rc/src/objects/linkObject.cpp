/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "libxml2/domDocument.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

#include "main.h"

#include <ipnoise-common/hexdump.h>
#include <ipnoise-common/system.h>
#include <ipnoise-common/strings.h>

#include "objects/linkObject.h"

#define MAX_IFS 128

LinkObject * LinkObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new LinkObject(a_node, a_doc, a_tagname);
}

LinkObject::LinkObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    ev = NULL;

    // do remove us after exit
    setExpired("0");
};

LinkObject::~LinkObject()
{
    if (ev != NULL){
        event_free(ev);
        ev = NULL;
    }

    if (sd >= 0){
        ::close(sd);
        sd  = -1;
    }
/* TODO
    if (l != NULL){
        libnet_destroy(l);
        l = NULL;
    }
*/
};

void LinkObject::read_udp_cb(
    int     socket,
    short,          // event
    void    *arg)
{
    LinkObject *link = (LinkObject *)arg;
    assert(link != NULL);

    char buffer[MAX_PACKET_SIZE];
    char tmp[1024];
    char *ptr = buffer;
    int res, rd;

    vector <DomElement *>   users;
    IPNoiseObject           *ipnoise    = NULL;

    // get ipnoise
    ipnoise = (IPNoiseObject *)link->getDocument()
        ->getRootElement();
    assert(ipnoise);

    SkBuffObject        *skb    = NULL;
    struct ip6_hdr      *ipv6   = NULL;
    struct udphdr       *udp    = NULL;
    struct ipnoisehdr   *l2hdr  = NULL;

    uint32_t    udp_src_port    = 0;
    uint32_t    udp_dst_port    = 0;
    int32_t     udp_data_len    = 0;
    uint32_t    udp_check       = 0;

    uint32_t    ip_ver          = 0;
    string      ifname          = link->getName();
    int         ifindex         = link->getIfIndex();

    struct sockaddr_ll ll_hdr;
    int ll_hdr_len = sizeof(ll_hdr);

    memset(buffer, 0x00, sizeof(buffer));
    rd = (int)recvfrom(
        socket,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&ll_hdr,
        (socklen_t *)&ll_hdr_len
    );
    if (rd <= 0){
        goto ret;
    }

    l2hdr = (struct ipnoisehdr *) buffer;

    // check what it is ipnoise packet
    res = ipnoise_hdr_is_valid(l2hdr);
    if (!res){
        goto ret;
    }

    // goto upper protocol
    ptr += IPNOISE_HLEN;
    rd -= IPNOISE_HLEN;

    ip_ver = (ptr[0] & 0xf0) >> 4;

    // prepare ipv6
    ipv6 = (struct ip6_hdr *)ptr;

    // check ipv6
    if (ip_ver != 6){
        PDEBUG_OBJ(link, 25,
            "was received not IPV6 protocol (source: '%s')\n",
            l2hdr->h_source
        );
        ipnoise_hexdump((unsigned char*)ptr, rd);
        goto ret;
    }

    // check what is UDP
    if (ipv6->ip6_nxt != IPPROTO_UDP){
        // it's not udp protocol
        goto ret;
    }

    // goto upper protocol
    ptr += sizeof(struct ip6_hdr);
    rd  -= sizeof(struct ip6_hdr);
    if (rd <= 0){
        // invalid packet size
        PDEBUG_OBJ(link, 25, "was received IPV6 packet without payload"
            " (source: '%s')\n",
            l2hdr->h_source);
        goto ret;
    }

    // prepare udp
    udp = (struct udphdr *)ptr;

    // move ptr to udp data
    ptr += sizeof(struct udphdr);
    rd  -= sizeof(struct udphdr);

    // good we have udp packet
    udp_src_port = htons(udp->source);
    udp_dst_port = htons(udp->dest);
    udp_data_len = htons(udp->len) - sizeof(struct udphdr);
    udp_check    = htons(udp->check);

    if (rd > udp_data_len){
        // XXX TODO
        // looks like we have bug in kernel "udp" link
        // some time we have recived buffer with spare data
        // (rd > udp_data_len)
        // now in this case, we will remove garbage
        rd = udp_data_len;
        ((char *)udp + sizeof(struct udphdr) + udp_data_len)[0] = 0x00;
    };

    // check errors
    if (rd != udp_data_len){
        // invalid packet size
        PDEBUG_OBJ(link, 25, "was received IPV6 udp packet,"
            " but udp len is invalid (source: '%s')\n",
            l2hdr->h_source);
        goto ret;
    }

    if (udp_dst_port != IPNOISE_UDP_PORT){
        PDEBUG_OBJ(link, 25, "was received ipv6 udp packet,"
            " but udp dst port is invalid ('%d' but must be: '%d')"
            " (source: '%s')\n",
            udp_dst_port,
            IPNOISE_UDP_PORT,
            l2hdr->h_source
        );
        goto ret;
    }

    // ipnoise_hexdump((unsigned char*)ptr, rd);

    PDEBUG_OBJ(link, 25, "Was received UDP packet from: '%s'\n",
        ifname.c_str());

    PDEBUG_OBJ(link, 25, "UDP source port: '%d'\n", udp_src_port);
    PDEBUG_OBJ(link, 25, "UDP dest   port: '%d'\n", udp_dst_port);
    PDEBUG_OBJ(link, 25, "UDP len:         '%d'\n", udp_data_len);
    PDEBUG_OBJ(link, 25, "UDP check:       '%d'\n", udp_check);

    skb = ipnoise->getDocument()->createElement<SkBuffObject>("ipn_skbuff");
    if (skb == NULL){
        PERROR_OBJ(link, "Cannot allocate new skb\n");
        goto fail;
    }

    // ipnoise_hexdump((unsigned char*)ptr, rd);

    /*
     * Attention, HUID always must have full length, don't truncate it!
     * Example:
     *   will work      2210:0000:0000:0000:0000:0000:0001:0001
     *   will not work  2210::1
     *
     * Use in6addr2huid to convert in6addr to huid
     *
     * 20110816 morik@
     */

    // received packet
    skb->setInContent(ptr);
    skb->setAttributeSafe("indev_name",     ifname.c_str());
    snprintf(tmp, sizeof(tmp), "%d",        ifindex);
    skb->setAttributeSafe("indev_index",    tmp);
    skb->setAttributeSafe("from",           in6addr2huid(&ipv6->ip6_src));
    skb->setAttributeSafe("to",             in6addr2huid(&ipv6->ip6_dst));

/*
    if (!strncmp(ifname, "tcp", 3)){
        // TODO XXX FIXME
        // if interface name like tcp..
        // replace port on 2210, TODO remove hardcode
        // we should replace port on valid interface port
        // (given from interface address)
        char *ptr = (char *)l2hdr->h_source;
        char port_str[] = { '2', '2', '1', '0', '\0' };
        int state = 0;
        while (*ptr != '\0'){
            if (state){
                *ptr = port_str[state - 1];
                if (state >= sizeof (port_str)){
                    break;
                }
                state++;
            }
            if (*ptr == ':'){
                state++;
            }
            ptr++;
        }
    }
*/
    skb->setAttributeSafe("from_ll", (char *)l2hdr->h_source);

    if (MULTICAST_USER_HUID != skb->getAttribute("from")){
        // check ON martian source
        snprintf(tmp, sizeof(tmp),
            "//ipn_users/ipn_user[@huid=\"%s\"]",
            skb->getAttribute("from").c_str()
        );
        users.clear();
        ipnoise->getElementsByXpath(tmp, users);
        if (users.size()){
            // martian source
            // TODO send "IP duplicate event" to packet sender
            goto drop;
        }
    }

    // check what user exist
    snprintf(tmp, sizeof(tmp),
        "//ipn_users/ipn_user[@huid=\"%s\"]",
        skb->getAttribute("to").c_str()
    );
    users.clear();
    ipnoise->getElementsByXpath(tmp, users);
    if (!users.size()){
        // no such user
        goto drop;
    }

    // snprintf(tmp, sizeof(tmp),
    //    "packet was received, dump below\n"
    //);
    //skb->dump2stderr(link, tmp);
    //skb->dump2stderr(NULL, tmp);

    // rx packet, after deliver for all slots, skb will be removed
    skb->prepare();
    link->getDocument()->emitSignalAndDelObj("packet_incoming", skb);

ret:
    return;

drop:
fail:
    if (skb != NULL){
        DomDocument::deleteElement(skb);
        skb = NULL;
    }
    goto ret;
}

ssize_t LinkObject::linkSendPacketOther(SkBuffObject *skb)
{
    ssize_t err = -1;
    int     res;
    struct  libnet_in6_addr dst_ip;
    struct  libnet_in6_addr src_ip;
    char    srcname[100], dstname[100];

    ServerObject    *server     = NULL;
    IPNoiseObject   *ipnoise    = NULL;

    char buffer[1024]       = { 0x00 };
    string src_huid         = skb->getAttribute("from");
    string dst_huid         = skb->getAttribute("to");
    string dst_hw           = skb->getAttribute("to_ll");
    string outbuffer        = skb->getOutDom()->getFirstChild()->serialize(0);
    string ifname           = getName();

    PDEBUG_OBJ(this, 10, "---> OUTGOING FROM INTERFACE: '%s'\n",
        ifname.c_str());
    PDEBUG_OBJ(this, 10, "---> TO:   '%s'\n",     dst_huid.c_str());
    PDEBUG_OBJ(this, 10, "---> LL:   '%s'\n",     dst_hw.c_str());
    PDEBUG_OBJ(this, 10, "---> FROM: '%s'\n",     src_huid.c_str());
    PDEBUG_OBJ(this, 10, "---> Send:\n%s\n",      outbuffer.c_str());

    if (!l){
        goto fail;
    }

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert ("ipn_server" == server->getTagName());

    dst_ip = libnet_name2addr6(l, (char *)dst_huid.c_str(), 1);
    res = strncmp((char*)&dst_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (!res){
        PERROR_OBJ(this, "Bad IP6 dst address: %s\n", dst_huid.c_str());
        goto fail;
    }

    if (src_huid.empty()){
        src_huid = server->getCreateUsersObject()->getAnyUserHuid();
    }
    src_ip = libnet_name2addr6(l, (char *)src_huid.c_str(), 1);
    res = strncmp((char*)&src_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (!res){
        PERROR_OBJ(this, "Bad IP6 src address: %s\n", src_huid.c_str());
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
        2210,                                       // src port
        2210,                                       // dst port
        sizeof(struct udphdr) + outbuffer.size(),   // total len
        0,                                          // check sum
        (u_int8_t *)outbuffer.c_str(),              // payload
        outbuffer.size(),                           // payload_s
        l,
        t
    );

    libnet_build_ipv6(
        0,                                         // flow label
        0,                                         // traffic class
        sizeof(struct udphdr) + outbuffer.size(),  // size
        IPPROTO_UDP,                               // next header
        64,                                        // hops limit
        src_ip,                                    // ip src
        dst_ip,                                    // id dst
        NULL,                                      // payload
        0,                                         // payload_s
        l,
        t
    );

    libnet_build_ipnoise(
        (u_int8_t*)(dst_hw.c_str()),                    // dst
        NULL,                                           // payload
        0,                                              // payload size
        l,                                              // libnet handle
        t
    );

    // send packet
    res = libnet_write(l);

    {
        snprintf(buffer, sizeof(buffer),
            "packet was delivered to libnet,"
            " %d byte(s) was wroted\n",
            res
        );

        string dbg = skb->toString(buffer);
        PDEBUG_OBJ(this, 25, "%s\n", dbg.c_str());
    }

    if (res == -1){
        PERROR_OBJ(this, "failed libnet_write: %s\n", libnet_geterror(l));
        err = 1;
        goto fail;
    }

    PDEBUG_OBJ(this, 25, "libnet_write, was wroted: '%d' bytes\n", res);
    err = 0;

ret:
    return err;

fail:
    err = 1;
    goto ret;
}

ssize_t LinkObject::linkSendPacketClient(SkBuffObject *skb)
{
    int err = -1;

    string outbuffer    = skb->getOutDom()->getFirstChild()->serialize(0);
    string dst_huid     = skb->getAttribute("to");
    string ifname       = getName();

    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    UserObject      *user       = NULL;

    PDEBUG_OBJ(this, 10, "-------------> OUTGOING FROM INTERFACE: '%s'\n",
        ifname.c_str());
    PDEBUG_OBJ(this, 10, "-------------> Send:\n%s\n", outbuffer.c_str());
    PDEBUG_OBJ(this, 10, "-------------> TO: '%s'\n",  dst_huid.c_str());

    // get IPNoise element
    ipnoise = (IPNoiseObject *)skb->getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();

    // get user
    user = server->getUser(dst_huid);
    if (!user){
        PERROR_OBJ(this, "Attempt send packet"
            " to not exist client with HUID: '%s'"
            " dump below:\n"
            "'%s'\n",
            dst_huid.c_str(),
            skb->toString().c_str()
        );
        goto out;
    }

    // send
    user->queue(skb->getOutDom()->getFirstChild());

    // all ok
    err = 0;

out:
    return err;
}

ssize_t LinkObject::linkSendPacketIPNoise(SkBuffObject *skb)
{
    ssize_t err = -1;

    string outbuffer    = skb->getOutDom()->getFirstChild()->serialize(0);
    string src_huid     = skb->getAttribute("from");
    string dst_huid     = skb->getAttribute("to");
    string ifname       = getName();

    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    UserObject      *user       = NULL;
    ItemObject      *item       = NULL;

    PDEBUG_OBJ(this, 15, "---> OUTGOING FROM INTERFACE: '%s'\n", ifname.c_str());
    PDEBUG_OBJ(this, 15, "---> Send:\n%s\n", outbuffer.c_str());
    PDEBUG_OBJ(this, 15, "---> TO: '%s'\n",  dst_huid.c_str());

    // get IPNoise element
    ipnoise = (IPNoiseObject *)skb->getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();
    if (!server){
        goto drop;
    }

    // get user
    user = server->getUser(src_huid);
    if (!user){
        goto drop;
    }

    // get item
    item = user->getCreateItem(dst_huid);
    if (!item){
        goto drop;
    }

    // send
    item->queue(skb->getOutDom()->getFirstChild());

    // all ok
    err = 0;

out:
    return err;

drop:
    if (err >= 0){
        err = -1;
    }
    PERROR("Drop packet: '%s', destination not found\n",
        skb->serialize(0).c_str());
    goto out;
}

int LinkObject::linkSendPacketLo(SkBuffObject *skb)
{
    ssize_t err         = -1;
    string outbuffer    = skb->getOutDom()->getFirstChild()->serialize(0);
    string src_huid     = skb->getAttribute("from");
    string dst_huid     = skb->getAttribute("to");
    string ifname       = getName();

    IPNoiseObject   *ipnoise = NULL;
    ServerObject    *server  = NULL;

    PDEBUG_OBJ(this, 15, "---> OUTGOING FROM INTERFACE: '%s'\n", ifname.c_str());
    PDEBUG_OBJ(this, 15, "---> Send:\n%s\n", outbuffer.c_str());
    PDEBUG_OBJ(this, 15, "---> TO: '%s'\n",  dst_huid.c_str());

    // prepare in
    {
        DomElement *el = skb->getOutDom()->getFirstChild();
        DomDocument::deleteChildsSafe(skb->getInDom());
        if (el){
            skb->setInContent(el->serializeForPublic(0));
            DomDocument::deleteChildsSafe(skb->getOutDom());
        };
    }

    // get IPNoise element
    ipnoise = (IPNoiseObject *)skb->getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();
    if (!server){
        goto drop;
    }

    // do receive
    skb->setInDevName(LO_DEV_NAME);
    server->netif_rx(skb);

    // all ok
    err = 0;

out:
    return err;

drop:
    if (err >= 0){
        err = -1;
    }
    PERROR("Drop packet: '%s', destination not found\n",
        skb->serialize(0).c_str());
    goto out;
}

int LinkObject::linkSendPacket(SkBuffObject *skb)
{
    string ifname = getName();
    int err = -1;

    if (CLIENTS_DEV_NAME == ifname){
        err = linkSendPacketClient(skb);
    } else if (LO_DEV_NAME == ifname){
        err = linkSendPacketLo(skb);
    } else if (IPNOISE_DEV_NAME == ifname){
        err = linkSendPacketIPNoise(skb);
    } else {
        err = linkSendPacketOther(skb);
    }

    return err;
}

/*
int32_t LinkObject::autorun()
{
    int err = 0;
    const char *ifname  = NULL;
    int ifindex         = NULL;
    ifname  = getName();
    ifindex = getIfIndex();

    if (ifname != NULL && ifindex >= 0){
        setupLink(ifname, ifindex);
    } else {
        // request to delete us
        setEexpired("-1");
    }
    return err;
}
*/

int LinkObject::setupLink(const string &ifname, int ifindex)
{
    int res, err = 0;
    struct sockaddr_ll  s_ll;
    sd = -1;
    l  = NULL;

    setName(ifname);
    setIfIndex(ifindex);

    PDEBUG_OBJ(this,
        5,
        "New interface was added for watching: '%s'\n",
        ifname.c_str());

    if (ifindex < 0){
        goto out;
    }

    // create raw socket
    sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sd < 0){
        PERROR_OBJ(this, "%s\n", "Cannot create"
            " socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)");
        goto fail;
    }

    memset((void *)&s_ll, 0, sizeof(s_ll));
    s_ll.sll_family     = PF_PACKET;        // тип сокета
    s_ll.sll_protocol   = htons(ETH_P_ALL); // тип принимаемого протокола
    s_ll.sll_ifindex    = ifindex;          // индекс сетевого интерфейса

    res = ::bind(sd, (struct sockaddr *)&s_ll, sizeof(struct sockaddr_ll));
    if (res < 0){
        PERROR_OBJ(this, "bind() for device: '%s' (ifindex: '%d') failed\n",
            ifname.c_str(),
            ifindex
        );
        goto fail;
    }

    res = fcntl(sd, F_SETFL, O_NONBLOCK);
    if (res == -1){
       PERROR_OBJ(this, "%s\n", "fcntl()");
       goto fail;
    }

    if (ev == NULL){
        ev = event_new(
            getDocument()->getEvBase(),
            sd,
            EV_READ | EV_PERSIST,
            read_udp_cb,
            (void *)this
        );
        if (ev == NULL){
            PERROR_OBJ(this, "%s\n", "Cannot create new event");
            goto fail;
        }
        event_add(ev, NULL);
    }

    // init libnet
    // Initialize the library.  Root priviledges are required.
    l = libnet_init(
        LIBNET_LINK,                        // injection type
        (char *)ifname.c_str(),             // network interface
        errbuf                              // error buffer
    );
    if (l == NULL){
        PERROR_OBJ(this, "libnet_init() failed for dev: '%s' (%s)\n",
            ifname.c_str(), errbuf);
        goto fail;
    }

out:
    return err;

fail:
    goto out;
};

time_t LinkObject::getLastCheck()
{
    time_t ret = 0;
    string last_check_str = Object::getAttribute("last_check");
    if (not last_check_str.empty()){
        ret = atoi(last_check_str.c_str());
    }
    return ret;
}

void LinkObject::setLastCheck(time_t time)
{
    char buffer[1024] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", (int)time);
    Object::setAttributeSafe("last_check", buffer);
}

string LinkObject::getName()
{
    return getAttribute("name");
}

vector <InterfaceInfo> LinkObject::getInterfacesInfo()
{
    vector <InterfaceInfo> ifs_info;

    int res, i;
    int sd = -1;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (sd < 0){
        PERROR("Cannot open socket(PF_INET, SOCK_STREAM, 0)\n");
        goto fail;
    }

    // XXX TODO fucking idiots, how can I known last device index?
    // now hardcode 1024 max
    for (i = 1; i < 1024; i++){
        struct ifreq    ifr;
        string          ifname = "";

        memset(&ifr, 0x00, sizeof(ifr));
        ifr.ifr_ifindex = i;

        res = ioctl(sd, SIOCGIFNAME, &ifr);
        if (res < 0){
            continue;
        }

        // store name
        ifname = ifr.ifr_name;

        // get ll addr
        res = ioctl(sd, SIOCGIFHWADDR, &ifr);
        if (res < 0){
            break;
        }

        if (ARPHRD_IPNOISE == ifr.ifr_hwaddr.sa_family){
            InterfaceInfo interface_info;
            interface_info.if_index = i;
            interface_info.if_name  = ifname;

            ifs_info.push_back(interface_info);
        }
    }

    close(sd);

out:
    return ifs_info;

fail:
    goto out;
}

int LinkObject::getIfIndex()
{
    int ret = -1;
    string ifindex_str = getAttribute("ifindex");
    if (not ifindex_str.empty()){
        ret = atoi(ifindex_str.c_str());
    }
    return ret;
}

void LinkObject::setName(const string &ifname)
{
    setAttributeSafe("name", ifname);
}

void LinkObject::setIfIndex(int ifindex)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%d", ifindex);
    setAttributeSafe("ifindex", buffer);
}

void LinkObject::setAttribute(const string &name, const string &value)
{
    // original "setAttribute" will call markAsDirty() everytime
    // we - never

    setAttributeSafe(name, value);
}

void LinkObject::registrySignals()
{
    getDocument()->registrySignal(
        "packet_incoming",
        "ipn_skbuff"
    );
    getDocument()->registrySignal(
        "packet_outcoming",
        "ipn_skbuff"
    );
}

void LinkObject::registrySlots()
{
}

void LinkObject::slot(
    const string    &,  // signal_name,
    DomElement      *)  // object
{
}

