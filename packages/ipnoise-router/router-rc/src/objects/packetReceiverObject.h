/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class PacketReceiverObject;

#ifndef PACKET_RECEIVER_OBJECT_H
#define PACKET_RECEIVER_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>

class DomDocument;

class PacketReceiverObject
    :   public Object
{
    public:
        PacketReceiverObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet_receiver"
        );
        virtual ~PacketReceiverObject();

        // internal
        virtual PacketReceiverObject *  create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet_receiver"
        );
        virtual void        shutdown();
        virtual int32_t     autorun();
        virtual void        timer();

        // generic
        string          getSessId();
        void            setSessId(const string &);
        string          getLastSendAttempt();
        void            setLastSendAttempt(const string &);

        // seq
        void            setSeq(const string &);
        void            setSeq(const uint32_t &);
        uint32_t        getSeqUint();
        string          getSeq();

        SessionObject * getSession();
        ssize_t         _xmit();
        ssize_t         xmit();
        void            wasDeliveredToAllClients();
        void            markAsDelivered();
        int32_t         isDelivered();
};

#endif

