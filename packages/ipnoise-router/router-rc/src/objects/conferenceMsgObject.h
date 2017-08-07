/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>
#include "objects/object.h"

class ConferenceMsgObject;

using namespace std;

#ifndef CONFERENCE_MSG_OBJECT_H
#define CONFERENCE_MSG_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceMsgsObject.h"

class ConferenceMsgObject
    :   public  Object
{
    public:
        ConferenceMsgObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_msg"
        );
        virtual ~ConferenceMsgObject();

        // internal
        virtual ConferenceMsgObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_msg"
        );
        virtual int32_t autorun();

        // generic
        ConferenceMsgsObject    *   getConferenceMsgsObject();
        string                      getId();
        void                        setId(const string &a_id);
};

#endif

