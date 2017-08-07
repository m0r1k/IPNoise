/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

class SessionIPNoiseObject;

using namespace std;

#ifndef SESSION_IPNOISE_OBJECT_H
#define SESSION_IPNOISE_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/sessionObject.h"
class DomDocument;

class SessionIPNoiseObject
    :   public SessionObject
{
    public:
        SessionIPNoiseObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_ipnoise"
        );
        virtual ~SessionIPNoiseObject();

        // internal
        virtual SessionIPNoiseObject *  create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_ipnoise"
        );
        virtual int32_t autorun();

        // interface
        virtual int32_t     connect();
        virtual void        process_packet(SkBuffObject *);
        virtual ssize_t     xmit(
            DomElement      *,
            const int32_t   &a_delete_after_send = 1
        );

        // generic
        ItemObject      *   getItem();
        bool                isConnected();
};

#endif

