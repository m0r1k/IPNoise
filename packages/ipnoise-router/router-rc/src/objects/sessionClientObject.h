/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

class SessionClientObject;

using namespace std;

#ifndef SESSION_CLIENT_OBJECT_H
#define SESSION_CLIENT_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/sessionObject.h"

class DomDocument;

class SessionClientObject
    :   public SessionObject
{
    public:
        SessionClientObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_client"
        );
        virtual ~SessionClientObject();

        // internal
        virtual SessionClientObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_client"
        );
        virtual void        registrySlots();
        virtual void        slot(
            const string    &a_signal_name,
            DomElement      *a_object
        );

        // generic
        virtual void    process_packet(SkBuffObject *);
        virtual ssize_t xmit(
            DomElement      *,
            const int32_t   &a_delete_after_send = 1
        );
};

#endif

