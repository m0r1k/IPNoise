#include "class.hpp"
DEFINE_CLASS(ObjectIPNoiseLink);
DEFINE_CLASS(InterfaceInfo);

#ifndef OBJECT_IPNOISE_LINK_HPP
#define OBJECT_IPNOISE_LINK_HPP

#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

extern "C" {
#undef _GNU_SOURCE
#include <libnet.h>
}

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

#include <map>
#include <string>
#include <memory>

#include <mongo/client/dbclient.h>
#include <ipnoise-common/log_common.h>
#include <ipnoise-common/hexdump.h>
#include "utils.hpp"

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectTime);
DEFINE_CLASS(ObjectIPNoiseSkb);

#define IPNOISE_UDP_PORT    2210
#define MAX_HUID_TEXT_LEN   128
#define MAX_PACKET_SIZE     65536

#define LO_DEV_NAME         "lo"        // virtual loopback
#define CLIENTS_DEV_NAME    "client"    // tcp 2210 hostos
#define IPNOISE_DEV_NAME    "ipnoise"   // udp/tcp 2210 ipv6 ipnoise

using namespace std;

#define _OBJECT_IPNOISE_LINK(a_out, a_flags, ...)           \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectIPNoiseLink,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_IPNOISE_LINK(a_out, a_flags, ...)             \
    CREATE_PROP(                                            \
        a_out,                                              \
        ObjectIPNoiseLink,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_IPNOISE_LINK(a_out, ...)                     \
    CREATE_OBJECT(a_out, ObjectIPNoiseLink, ##__VA_ARGS__ )

#define PROP_IPNOISE_LINK(a_out, ...)                       \
    CREATE_PROP(a_out, ObjectIPNoiseLink, ##__VA_ARGS__ )

class InterfaceInfo
{
    public:
        InterfaceInfo(){
            m_name     = "is_not_set";
            m_index    = -1;
        };
        virtual ~InterfaceInfo(){};

        // name
        string getName(){
            return m_name;
        }
        void setName(
            const string &a_name)
        {
            m_name = a_name;
        }

        // index
        int32_t getIndex(){
            return m_index;
        }
        void setIndex(
            const int32_t &a_index)
        {
            m_index = a_index;
        }

    private:
        string  m_name;
        int32_t m_index;
};

class ObjectIPNoiseLink
    :   public  Object
{
    public:
        ObjectIPNoiseLink(const Object::CreateFlags &);
        virtual ~ObjectIPNoiseLink();

        DEFINE_PROP(IfIndex,    ObjectInt32Sptr);
        DEFINE_PROP(CheckTime,  ObjectTimeSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        int32_t setupLink(
            const string    &a_ifname,
            const int32_t   a_ifindex
        );
        void    resetCheckTime();
        void    parsePacket(
            const string &a_packet_raw
        );
        ssize_t send(
            ObjectIPNoiseSkbSptr a_skb
        );

        // static
        static int32_t getInterfacesInfo(
            vector<InterfaceInfoSptr> &a_out
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    //public slots:
        void readSlot(int);

    private:
        int32_t         m_sd;
        libnet_t        *m_libnet;
        char            m_libnet_errbuf[LIBNET_ERRBUF_SIZE];
        libnet_ptag_t   m_libnet_ptag;

        void    do_init(const Object::CreateFlags &);
};

/*
 * Attention, HUID always must have full length, don't truncate it!
 * Example:
 *   will work      2210:0000:0000:0000:0000:0000:0001:0001
 *   will not work  2210::1
 *
 * 20110816 morik@
 */
inline string in6addr2huid(struct in6_addr *a_in)
{
    int i;
    char    buffer[1024]    = { 0x00 };
    string  ret             = "";

    for (i = 0; i < 16; i++){
        snprintf(buffer, sizeof(buffer), "%2.2x",
            a_in->s6_addr[i]);
        if (i > 0 && 0 == i%2){
            ret += ":";
        }
        ret += buffer;
    }
    return ret;
}

#endif

