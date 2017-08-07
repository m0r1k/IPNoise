/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

class SessionLoObject;

using namespace std;

#ifndef SESSION_LO_OBJECT_H
#define SESSION_LO_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/sessionObject.h"
class DomDocument;

class SessionLoObject
    :   public SessionObject
{
    public:
        SessionLoObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_lo"
        );
        virtual ~SessionLoObject();

        // internal
        virtual SessionLoObject *   create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_lo"
        );
        virtual int32_t autorun();

        // interface
        virtual ssize_t xmit(
            DomElement      *,
            const int32_t   &a_delete_after_send = 1
        );
};

#endif

