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

class PacketObject;

#ifndef PACKET_OBJECT_H
#define PACKET_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>

#include "objects/packetReceiversObject.h"

class DomDocument;

#define PACKET_RETRANSMIT_TIME "+30"

class PacketObject
    :   public Object
{
    public:
        PacketObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet"
        );
        virtual ~PacketObject();

        // internal
        virtual PacketObject *  create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_packet"
        );
        virtual void            shutdown();
        virtual int32_t         autorun();
        virtual void            timer();

        // generic
        ssize_t         xmit();

        PacketReceiversObject   *   getReceiversObject();
        PacketReceiversObject   *   getCreateReceiversObject();
        PacketReceiverObject    *   createReceiver();
        void                        getReceivers(
            vector<PacketReceiverObject *> &
        );
        int32_t isDelivered(const string &a_sessid = "");

        DomElement *    getIPNoiseElement();
        void            setIPNoiseElement(DomElement *);
        DomElement *    getCreateIPNoiseElement();

        DomElement *    getEventsElement();
        void            setEventsElement(DomElement *);
        DomElement *    getCreateEventsElement();

        DomElement *    getCommandsElement();
        void            setCommandsElement(DomElement *);
        DomElement *    getCreateCommandsElement();

        PacketReceiverObject * getReceiver(
            const string &a_sessid
        );
        PacketReceiverObject * getCreateReceiver(
            const string &a_sessid
        );

};

#endif

