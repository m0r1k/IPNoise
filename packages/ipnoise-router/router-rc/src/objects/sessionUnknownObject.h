/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

class SessionUnknownObject;

using namespace std;

#ifndef SESSION_UNKNOWN_OBJECT_H
#define SESSION_UNKNOWN_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/sessionObject.h"

class DomDocument;

class SessionUnknownObject
    :   public SessionObject
{
    public:
        SessionUnknownObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_unknown"
        );
        virtual ~SessionUnknownObject();

        // internal
        virtual SessionUnknownObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_unknown"
        );

        // generic
};

#endif

