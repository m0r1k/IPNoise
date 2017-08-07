#include "class.hpp"
DEFINE_CLASS(NeuronIPNoiseLink);
DEFINE_CLASS(InterfaceInfo);

#ifndef NEURON_IPNOISE_LINK_HPP
#define NEURON_IPNOISE_LINK_HPP

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

#include <QThread>
#include <QSocketNotifier>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include <ipnoise-common/ipnoise.h>
#include "utils.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/ipnoise/skb/main.hpp"

#include "core/neuron/neuron/main.hpp"

#define IPNOISE_UDP_PORT    2210
#define MAX_HUID_TEXT_LEN   128
#define MAX_PACKET_SIZE     65536

#define LO_DEV_NAME         "lo"        // virtual loopback
#define CLIENTS_DEV_NAME    "client"    // tcp 2210 hostos
#define IPNOISE_DEV_NAME    "ipnoise"   // udp/tcp 2210 ipv6 ipnoise

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class InterfaceInfo
{
    public:
        InterfaceInfo(){
            m_name     = "is_not_set";
            m_index    = -1;
        };
        ~InterfaceInfo(){};

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

class NeuronIPNoiseLink
    :   public  SUPER_CLASS
{
    Q_OBJECT

    public:
        NeuronIPNoiseLink();
        virtual ~NeuronIPNoiseLink();

        DEFINE_PROP(IfIndex,    PropIntSptr);
        DEFINE_PROP(CheckTime,  PropTimeSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_props
        );

        // generic
        virtual int32_t do_autorun();

        int32_t setupLink(
            const string    &a_ifname,
            const int32_t   a_ifindex
        );

        void resetCheckTime();
        void parsePacket(
            const QByteArray &a_packet_raw
        );
        ssize_t send(
            NeuronIPNoiseSkbSptr a_skb
        );

        // static
        static int32_t getInterfacesInfo(
            vector<InterfaceInfoSptr> &a_out
        );

        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    public slots:
        void readSlot(int);

    private:
        void            do_init();

        int32_t         m_sd;
        QSocketNotifier *m_socket_notifier;
        libnet_t        *m_libnet;
        char            m_libnet_errbuf[LIBNET_ERRBUF_SIZE];
        libnet_ptag_t   m_libnet_ptag;
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

