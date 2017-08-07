/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class SessionsObject;

#ifndef SESSIONS_OBJECT_H
#define SESSIONS_OBJECT_H

#include <libxml/tree.h>
#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/sessionObject.h"
#include "objects/packetsObject.h"

class SessionsObject
    :   public Object
{
    public:
        SessionsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_sessions"
        );
        virtual ~SessionsObject();

        // internal
        virtual SessionsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_sessions"
        );
        virtual int32_t     autorun();
        virtual void        slot(
            const string    &signal_name,
            DomElement      *object
        );

        // generic methods
        PacketObject        *   queue(const string &a_packet_data);
        PacketObject        *   queue(DomElement *a_packet_data);
        ssize_t                 xmit(const string &a_packet_data);
        ssize_t                 xmit(DomElement *a_packet_data);
        ssize_t                 xmit(PacketObject *);
        SessionObject       *   getSession(const string &);
        SessionObject       *   createSession(
            const string &sess_type,
            const string &sessid = ""
        );
        SessionObject       *   getCreateSession(
            const string &sess_type,
            const string &sessid = ""
        );
        ItemObject          *   getItem();
        PacketsObject       *   getCreatePacketsObject();
        void                    ackPacket(
            const string    &a_sessid,
            const string    &a_packetid
        );
        SessionObject       *   getOutcomingSession();
        SessionObject       *   createIPNoiseOutcomingSession();

        void getSessions(vector<SessionObject *> &);
        void getOnlineSessions(vector<SessionObject *> &);

        static SessionObject * getSession(
            DomElement      *,
            const string    &
        );
};

#endif

