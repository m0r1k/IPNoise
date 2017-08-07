/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <string>
#include <vector>

using namespace std;

class InterfaceInfo;
class LinkObject;

#ifndef LINK_OBJECT_H
#define LINK_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include "libnet.h"

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"
#include "objects/skBuffObject.h"
class DomDocument;

#define IPNOISE_UDP_PORT    2210
#define MAX_HUID_TEXT_LEN   128
#define MAX_PACKET_SIZE     65536

#define LO_DEV_NAME         "lo"        // virtual loopback
#define CLIENTS_DEV_NAME    "client"    // tcp 2210 hostos
#define IPNOISE_DEV_NAME    "ipnoise"   // udp/tcp 2210 ipv6 ipnoise

class InterfaceInfo
{
    public:
        InterfaceInfo(){
            if_name     = "is_not_set";
            if_index    = -1;
        };
        ~InterfaceInfo(){};

        string  if_name;
        int     if_index;
};

class LinkObject
    :   public Object
{
    public:
        LinkObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &_tagname = "ipn_link"
        );
        ~LinkObject();

        // internal
        virtual LinkObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_link"
        );
        virtual void setAttribute(
            const string    &ifname,
            const string    &value
        );

        // generic
        int sd;
        static vector <InterfaceInfo> getInterfacesInfo();

        virtual int setupLink(const string &ifname, int _ifindex);

        ssize_t                 linkSendPacketLo(SkBuffObject *skb);
        ssize_t                 linkSendPacketClient(SkBuffObject *skb);
        ssize_t                 linkSendPacketIPNoise(SkBuffObject *skb);
        ssize_t                 linkSendPacketOther(SkBuffObject *skb);
        virtual ssize_t         linkSendPacket(SkBuffObject *skb);
        virtual time_t          getLastCheck();
        virtual void            setLastCheck(time_t time);
        virtual int             getIfIndex();
        virtual string          getName();

        void setName(const string &name);
        void setIfIndex(int ifindex);

    private:
        struct event    *ev;
        libnet_ptag_t   t;
        libnet_t        *l;
        char errbuf[LIBNET_ERRBUF_SIZE];

        static void read_udp_cb(
            int socket,
            short event,
            void *arg
        );

        // signal|splots implementation
        virtual void registrySignals();
        virtual void registrySlots();
        virtual void slot(
            const string    &signal_name,
            DomElement      *object
        );
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

