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

class ConferenceMsgsObject;

using namespace std;

#ifndef CONFERENCE_MSGS_OBJECT_H
#define CONFERENCE_MSGS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceObject.h"
#include "objects/conferenceMsgObject.h"

class ConferenceMsgsObject
    :   public  Object
{
    public:
        ConferenceMsgsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_msgs"
        );
        virtual ~ConferenceMsgsObject();

        // internal
        virtual ConferenceMsgsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_msgs"
        );
        virtual int32_t autorun();

        // generic
        ConferenceObject    *   getConferenceObject();
        ConferenceMsgObject *   getConferenceMsgById(
            const string &msg_id
        );
        ConferenceMsgObject *   addMsg(
            const string    &a_type             = "",
            const string    &a_msg_data         = "",
            const string    &a_msg_id           = "",
            const string    &a_src_conf_huid    = "",
            const string    &a_src_conf_id      = ""
        );
        string                  createMsgId();
};

#endif

