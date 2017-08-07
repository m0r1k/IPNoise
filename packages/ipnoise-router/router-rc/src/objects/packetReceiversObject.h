/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>
#include <vector>

using namespace std;

class PacketReceiversObject;

#ifndef PACKETS_RECEIVERS_OBJECT_H
#define PACKETS_RECEIVERS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>

class DomDocument;

#include "objects/packetReceiverObject.h"

class PacketReceiversObject
    :   public Object
{
    public:
        PacketReceiversObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet_receivers"
        );
        virtual ~PacketReceiversObject();

        // internal
        virtual PacketReceiversObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet_receivers"
        );
        virtual void        shutdown();
        virtual int32_t     autorun();
        virtual void        timer();

        // generic
        PacketReceiverObject *  getReceiver(const string &a_sessid);
        void                    getReceivers(
            vector<PacketReceiverObject *>  &
        );
        PacketReceiverObject *  getCreateReceiver(const string &);

};

#endif

