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

class PacketsObject;

#ifndef PACKETS_OBJECT_H
#define PACKETS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>

class DomDocument;

#include "objects/packetObject.h"
#include "objects/packetReceiversObject.h"
#include "objects/packetReceiverObject.h"

class PacketsObject
    :   public Object
{
    public:
        PacketsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packets"
        );
        virtual ~PacketsObject();

        // internal
        virtual PacketsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packets"
        );
        virtual void        shutdown();
        virtual int32_t     autorun();
        virtual void        timer();

        // generic
        void            getPackets(vector<PacketObject *> &);
        PacketObject *  getFirstPacket();
        PacketObject *  getPacket(const string &);
        PacketObject *  getPacket(const uint32_t &);
        PacketObject *  createPacket(DomElement *);
        PacketObject *  queue(DomElement *);
        void            clear();
};

#endif

