#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/ipnoiseSkb/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/ipnoiseLink/main.hpp"

ObjectIPNoiseLink::ObjectIPNoiseLink(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoiseLink::~ObjectIPNoiseLink()
{
    if (0 <= m_sd){
        ::close(m_sd);
        m_sd = -1;
    }
};

void ObjectIPNoiseLink::do_init(
    const Object::CreateFlags   &a_flags)
{
    m_sd                = -1;
    //m_socket_notifier   = NULL;
}

string ObjectIPNoiseLink::getType()
{
    return ObjectIPNoiseLink::_getType();
}

void ObjectIPNoiseLink::do_init_props()
{
    Object::do_init_props();

    ObjectInt32Sptr prop_if_index;
    ObjectTimeSptr  prop_check_time;

    PROP_INT32(prop_if_index, -1);
    PROP_TIME(prop_check_time);

    INIT_PROP(this, IfIndex,    prop_if_index);
    INIT_PROP(this, CheckTime,  prop_check_time);
}

void ObjectIPNoiseLink::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoiseLink::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseLink::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("if_index",   getIfIndex);
    SAVE_PROP("check_time", getCheckTime);

    Object::getAllProps(a_props);
}

int32_t ObjectIPNoiseLink::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err         = 0;
    int32_t if_index    = 0;
    double  check_time  = 0;

    ObjectInt32Sptr prop_if_index;
    ObjectTimeSptr  prop_check_time;

    PROP_INT32(prop_if_index,  if_index);
    PROP_TIME(prop_check_time, check_time);

    // dev index
    if (a_obj.hasField("if_index")){
        if_index = a_obj.getIntField("if_index");
        INIT_PROP(this, IfIndex, prop_if_index);
    }

    // check time
    if (a_obj.hasField("check_time")){
        check_time = a_obj.getField("check_time").Double();
        INIT_PROP(this, CheckTime, prop_check_time);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectIPNoiseLink::readSlot(
    int32_t a_fd)
{
    PWARN("was read new pascket"
        " from: '%s',"
        " a_fd: '%d',"
        " m_sd: '%d'\n",
        getName()->toString()->c_str(),
        a_fd,
        m_sd
    );

 //   char        buffer[65535] = { 0x00 };
 //   int32_t     res = 0;
 //   QByteArray  packet_raw;
 //
 //   do {
 //       res = read(m_sd, buffer, sizeof(buffer));
 //       if (!res){
 //           break;
 //       }
 //       packet_raw.append(buffer, res);
 //       continue;
 //   } while (res < 0 && -EINTR == errno);
 //
 //   if (packet_raw.size()){
 //       parsePacket(packet_raw);
 //   } else if (res < 0){
 //   } else {
 //       if (m_socket_notifier){
 //           delete m_socket_notifier;
 //           m_socket_notifier = NULL;
 //       }
 //   }

 //   return;

    char buffer[MAX_PACKET_SIZE];
//    char tmp[1024];
    char *ptr = buffer;
    int res, rd;

//    vector <DomElement *>   users;
    ObjectIPNoiseSkbSptr    skb;

    struct ip6_hdr      *ipv6   = NULL;
    struct udphdr       *udp    = NULL;
    struct ipnoisehdr   *l2hdr  = NULL;

    uint32_t    udp_src_port    = 0;
    uint32_t    udp_dst_port    = 0;
    int32_t     udp_data_len    = 0;
    uint32_t    udp_check       = 0;

    uint32_t    ip_ver          = 0;
    string      ifname          = getName()->toString();
    int32_t     ifindex         = *getIfIndex();

    ObjectStringSptr    prop_in_content;
    ObjectStringSptr    prop_in_link_name;
    ObjectInt32Sptr     prop_in_link_index;
    ObjectStringSptr    prop_from;
    ObjectStringSptr    prop_to;

    struct sockaddr_ll ll_hdr;
    int ll_hdr_len = sizeof(ll_hdr);

    // clear buffer
    memset(buffer, 0x00, sizeof(buffer));

    rd = (int)recvfrom(
        m_sd,
        buffer,
        sizeof(buffer) - 1,
        0,
        (struct sockaddr *)&ll_hdr,
        (socklen_t *)&ll_hdr_len
    );
    if (0 >= rd){
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
    rd  -= IPNOISE_HLEN;

    ip_ver = (ptr[0] & 0xf0) >> 4;

    // prepare ipv6
    ipv6 = (struct ip6_hdr *)ptr;

    // check ipv6
    if (ip_ver != 6){
        PDEBUG(25,
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
        PDEBUG(25, "was received IPV6 packet without payload"
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
        PDEBUG(25, "was received IPV6 udp packet,"
            " but udp len is invalid (source: '%s')\n",
            l2hdr->h_source
        );
        goto ret;
    }

    if (udp_dst_port != IPNOISE_UDP_PORT){
        PDEBUG(25, "was received ipv6 udp packet,"
            " but udp dst port is invalid"
            " ('%d' but must be: '%d')"
            " (source: '%s')\n",
            udp_dst_port,
            IPNOISE_UDP_PORT,
            l2hdr->h_source
        );
        goto ret;
    }

    // ipnoise_hexdump((unsigned char*)ptr, rd);

    PDEBUG(25, "Was received UDP packet from: '%s'\n",
        ifname.c_str());

    PDEBUG(25, "UDP source port: '%d'\n", udp_src_port);
    PDEBUG(25, "UDP dest   port: '%d'\n", udp_dst_port);
    PDEBUG(25, "UDP len:         '%d'\n", udp_data_len);
    PDEBUG(25, "UDP check:       '%d'\n", udp_check);

    skb = createNeigh<ObjectIPNoiseSkb>(
        "core.object.ipnoise.skb"
    );
    if (!skb){
        PERROR("Cannot allocate new skb\n");
        goto fail;
    }

    // ipnoise_hexdump((unsigned char*)ptr, rd);

    //
    // Attention, HUID always must have full length, don't truncate it!
    // Example:
    //   will work      2210:0000:0000:0000:0000:0000:0001:0001
    //   will not work  2210::1
    //
    // Use in6addr2huid to convert in6addr to huid
    //
    // 20110816 morik@
    //

    PROP_STRING(prop_in_content,    ptr);
    PROP_STRING(prop_in_link_name,  ifname);
    PROP_INT32(prop_in_link_index,  ifindex);
    PROP_STRING(prop_from,          in6addr2huid(&ipv6->ip6_src));
    PROP_STRING(prop_to,            in6addr2huid(&ipv6->ip6_dst));

    // received packet
    skb->setInContent(prop_in_content);
    skb->setInLinkName(prop_in_link_name);
    skb->setInLinkIndex(prop_in_link_index);
    skb->setFrom(prop_from);
    skb->setTo(prop_to);

 //   skb->setInContent(ptr);
 //   skb->setAttributeSafe("indev_name",     ifname.c_str());
 //   snprintf(tmp, sizeof(tmp), "%d",        ifindex);
 //   skb->setAttributeSafe("indev_index",    tmp);
 //   skb->setAttributeSafe("from",           in6addr2huid(&ipv6->ip6_src));
 //   skb->setAttributeSafe("to",             in6addr2huid(&ipv6->ip6_dst));
 //
 //   if (!strncmp(ifname, "tcp", 3)){
 //       // TODO XXX FIXME
 //       // if interface name like tcp..
 //       // replace port on 2210, TODO remove hardcode
 //       // we should replace port on valid interface port
 //       // (given from interface address)
 //       char *ptr = (char *)l2hdr->h_source;
 //       char port_str[] = { '2', '2', '1', '0', '\0' };
 //       int state = 0;
 //       while (*ptr != '\0'){
 //           if (state){
 //               *ptr = port_str[state - 1];
 //               if (state >= sizeof (port_str)){
 //                   break;
 //               }
 //               state++;
 //           }
 //           if (*ptr == ':'){
 //               state++;
 //           }
 //           ptr++;
 //       }
 //   }
 //
 //   skb->setAttributeSafe("from_ll", (char *)l2hdr->h_source);

 //   if (MULTICAST_USER_HUID != skb->getAttribute("from")){
 //       // check ON martian source
 //       snprintf(tmp, sizeof(tmp),
 //           "//ipn_users/ipn_user[@huid=\"%s\"]",
 //           skb->getAttribute("from").c_str()
 //       );
 //       users.clear();
 //       ipnoise->getElementsByXpath(tmp, users);
 //       if (users.size()){
 //           // martian source
 //           // TODO send "IP duplicate event" to packet sender
 //           goto drop;
 //       }
 //   }

    // check what user exist
    //snprintf(tmp, sizeof(tmp),
    //    "//ipn_users/ipn_user[@huid=\"%s\"]",
    //    skb->getAttribute("to").c_str()
    //);
    //users.clear();
    //ipnoise->getElementsByXpath(tmp, users);
    //if (!users.size()){
    //    // no such user
    //    goto drop;
    //}

    // snprintf(tmp, sizeof(tmp),
    //    "packet was received, dump below\n"
    //);
    //skb->dump2stderr(link, tmp);
    //skb->dump2stderr(NULL, tmp);

    // rx packet, after deliver for all slots, skb will be removed
    // skb->prepare();
    // link->getDocument()->emitSignalAndDelObj("packet_incoming", skb);

ret:
    return;

//drop:
fail:
    if (skb){
        skb->remove();
        skb.reset();
    }
    goto ret;

}

int32_t ObjectIPNoiseLink::setupLink(
    const string    &a_ifname,
    const int32_t   a_ifindex)
{
    int                 res, err = -1;
    struct sockaddr_ll  s_ll;
    ObjectTimeSptr      prop_time_now;
    ObjectStringSptr    prop_if_name;
    ObjectInt32Sptr     prop_if_index;

    PROP_TIME_NOW(prop_time_now);
    PROP_STRING(prop_if_name, a_ifname);
    PROP_INT32(prop_if_index, a_ifindex);

    if (0 < m_sd){
        PWARN("m_sd not closed\n");
        ::close(m_sd);
        m_sd = -1;
    }

    setName(      prop_if_name);
    setIfIndex(   prop_if_index);
    setCheckTime( prop_time_now);

    if (0 > a_ifindex){
        goto out;
    }

    // create raw socket
    m_sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (0 > m_sd){
        PERROR("cannot create socket:"
            " socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)\n"
        );
        goto fail;
    }

    memset((void *)&s_ll, 0, sizeof(s_ll));
    s_ll.sll_family     = PF_PACKET;
    s_ll.sll_protocol   = htons(ETH_P_ALL);
    s_ll.sll_ifindex    = a_ifindex;

    // bind to interface
    res = ::bind(
        m_sd,
        (struct sockaddr *)&s_ll,
        sizeof(struct sockaddr_ll)
    );
    if (0 > res){
        PERROR("bind() for device: '%s'"
            " (ifindex: '%d') failed\n",
            a_ifname.c_str(),
            a_ifindex
        );
        goto fail;
    }

    // set to non block
    res = fcntl(m_sd, F_SETFL, O_NONBLOCK);
    if (res){
        PERROR("cannot set socket: '%d'"
            " to non block mode\n",
            m_sd
       );
       goto fail;
    }

    // create notifier
    //m_socket_notifier = new QSocketNotifier(
    //    m_sd,
    //    QSocketNotifier::Read,
    //    this
    //);

    //CHECKED_CONNECT(
    //    m_socket_notifier,  SIGNAL(activated(int)),
    //    this,               SLOT(readSlot(int))
    //);

    // all ok
    err = 0;

 //   if (ev == NULL){
 //       ev = event_new(
 //           getDocument()->getEvBase(),
 //           sd,
 //           EV_READ | EV_PERSIST,
 //           read_udp_cb,
 //           (void *)this
 //       );
 //       if (ev == NULL){
 //           PERROR("Cannot create new event\n");
 //           goto fail;
 //       }
 //       event_add(ev, NULL);
 //   }

    // init libnet
    // Initialize the library.  Root priviledges are required.
    m_libnet = libnet_init(
        LIBNET_LINK,                    // injection type
        (char *)a_ifname.c_str(),       // network interface
        m_libnet_errbuf                 // error buffer
    );
    if (!m_libnet){
        PERROR("libnet_init() failed for dev: '%s' (%s)\n",
            a_ifname.c_str(),
            m_libnet_errbuf
        );
        goto fail;
    }

out:
    PDEBUG(10, "setupLink:"
        " ifname: '%s',"
        " ifindex: '%d',"
        " err: '%d'\n",
        a_ifname.c_str(),
        a_ifindex,
        err
    );
    return err;

fail:
    goto out;
}

int32_t ObjectIPNoiseLink::getInterfacesInfo(
    vector<InterfaceInfoSptr> &a_out)
{
    int res, i;
    int sd  = -1;
    int err = -1;

    // open socket
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > sd){
        PERROR("Cannot open"
            " socket(PF_INET, SOCK_STREAM, 0)\n"
        );
        goto fail;
    }

    // XXX TODO fucking idiots,
    // how can I known last device index?
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
            InterfaceInfoSptr interface_info(
                new InterfaceInfo
            );
            interface_info->setIndex(i);
            interface_info->setName(ifname);

            a_out.push_back(interface_info);
        }
    }

    // all ok
    err = 0;

out:
    if (0 <= sd){
        ::close(sd);
    }
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void ObjectIPNoiseLink::resetCheckTime()
{
    ObjectTimeSptr time_now;
    PROP_TIME_NOW(time_now);
    setCheckTime(time_now);
}

ssize_t ObjectIPNoiseLink::send(
    ObjectIPNoiseSkbSptr a_skb)
{
    ssize_t err = -1;
    int     res;
    struct  libnet_in6_addr dst_ip;
    struct  libnet_in6_addr src_ip;

    char    srcname[100], dstname[100];

    char buffer[1024]   = { 0x00 };
    string src_huid     = "2210:0000:0000:0000:0000:0000:0001:0001";
    string dst_huid     = "2210:0000:0000:0000:0000:0000:0001:0002";
    string dst_hw       = "192.168.1.1:1515";
    string outbuffer    = "tigra was here";
    string ifname       = getName()->toString();

    PDEBUG(40, "---> OUTGOING FROM INTERFACE: '%s'\n",
        ifname.c_str());
    PDEBUG(40, "---> TO:   '%s'\n",     dst_huid.c_str());
    PDEBUG(40, "---> LL:   '%s'\n",     dst_hw.c_str());
    PDEBUG(40, "---> FROM: '%s'\n",     src_huid.c_str());
    PDEBUG(40, "---> Send:\n%s\n",      outbuffer.c_str());

    if (!m_libnet){
        goto fail;
    }

    // get ipnoise
    //ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    //assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    //server = (ServerObject *)ipnoise->getServerObject();
    //assert ("ipn_server" == server->getTagName());

    dst_ip = libnet_name2addr6(
        m_libnet,
        (char *)dst_huid.c_str(),
        1
    );
    res = strncmp((char*)&dst_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (!res){
        PERROR("Bad IP6 dst address: %s\n", dst_huid.c_str());
        goto fail;
    }

    //if (src_huid.empty()){
    //    src_huid = server->getCreateUsersObject()->getAnyUserHuid();
    //}
    src_ip = libnet_name2addr6(
        m_libnet,
        (char *)src_huid.c_str(),
        1
    );
    res = strncmp((char*)&src_ip,
        (char*)&in6addr_error,
        sizeof(in6addr_error)
    );
    if (!res){
        PERROR("Bad IP6 src address: %s\n", src_huid.c_str());
        err = 1;
        goto fail;
    }

    libnet_seed_prand(m_libnet);
    libnet_addr2name6_r(src_ip, 1, srcname,sizeof(srcname));
    libnet_addr2name6_r(dst_ip, 1, dstname,sizeof(dstname));

    m_libnet_ptag = LIBNET_PTAG_INITIALIZER;

    // clear packet
    libnet_clear_packet(m_libnet);

    libnet_build_udp(
        2210,                                       // src port
        2210,                                       // dst port
        sizeof(struct udphdr) + outbuffer.size(),   // total len
        0,                                          // check sum
        (u_int8_t *)outbuffer.c_str(),              // payload
        outbuffer.size(),                           // payload_s
        m_libnet,
        m_libnet_ptag
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
        m_libnet,
        m_libnet_ptag
    );

    libnet_build_ipnoise(
        (u_int8_t*)(dst_hw.c_str()),                    // dst
        NULL,                                           // payload
        0,                                              // payload size
        m_libnet,                                       // libnet handle
        m_libnet_ptag
    );

    // send packet
    res = libnet_write(m_libnet);

    {
        snprintf(buffer, sizeof(buffer),
            "packet was delivered to libnet,"
            " %d byte(s) was wroted\n",
            res
        );

        //string dbg = skb->toString(buffer);
        //PDEBUG(25, "%s\n", dbg.c_str());
    }

    if (res == -1){
        PERROR("failed libnet_write: %s\n",
            libnet_geterror(m_libnet)
        );
        err = 1;
        goto fail;
    }

    PDEBUG(25, "libnet_write, was wroted: '%d' bytes\n", res);
    err = 0;

ret:
    return err;

fail:
    err = 1;
    goto ret;
}

// ---------------- module ----------------

string ObjectIPNoiseLink::_getType()
{
    return "core.object.ipnoise.link";
}

string ObjectIPNoiseLink::_getTypePerl()
{
    return "core::object::ipnoiseLink::main";
}

int32_t ObjectIPNoiseLink::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseLink::_destroy()
{
}

ObjectSptr ObjectIPNoiseLink::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE_LINK(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectIPNoiseLink::_getType,
    .type_perl      = ObjectIPNoiseLink::_getTypePerl,
    .init           = ObjectIPNoiseLink::_init,
    .destroy        = ObjectIPNoiseLink::_destroy,
    .object_create  = ObjectIPNoiseLink::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

