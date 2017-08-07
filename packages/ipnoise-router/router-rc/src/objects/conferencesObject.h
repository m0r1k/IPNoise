/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>
#include "objects/object.h"

class ConferencesObject;

using namespace std;

#ifndef CONFERENCES_OBJECT_H
#define CONFERENCES_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceObject.h"

class ConferencesObject
    :   public  Object
{
    public:
        ConferencesObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conferences"
        );
        virtual ~ConferencesObject();

        // internal
        virtual ConferencesObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conferences"
        );
        virtual int32_t autorun();

        // generic
        UserObject          *   getUserObject();
        ConferenceObject    *   getConferenceById(const string &);
        string                  createConferenceId();
        ConferenceObject    *   createConference(
            const string &a_conf_flags,
            const string &a_conf_topic = ""
        );
        ConferenceObject    *   createConferenceById(
            const string &a_conf_id,
            const string &a_conf_flags,
            const string &a_conf_topic = ""
        );
};

#endif

