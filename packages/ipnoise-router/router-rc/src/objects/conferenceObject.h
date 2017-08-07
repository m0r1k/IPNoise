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
#include "objects/conferencesObject.h"
#include "objects/conferenceItemObject.h"

class ConferenceObject;

using namespace std;

#ifndef CONFERENCE_OBJECT_H
#define CONFERENCE_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceMsgsObject.h"
#include "objects/conferenceItemsObject.h"
#include "objects/conferenceMsgObject.h"

class ConferenceObject
    :   public  Object
{
    public:
        ConferenceObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference"
        );
        virtual ~ConferenceObject();

        // internal
        virtual ConferenceObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference"
        );
        virtual int32_t     autorun();
        virtual void        shutdown();
        virtual void        registrySignals();

        // generic
        ConferenceItemObject    * getCreateConferenceItem(
            const string    &huid,
            const string    &a_inform_type = ""
        );

        ConferenceItemObject    * getConferenceItemByHuid(
            const string &);
        ConferenceItemsObject   * getCreateConferenceItemsObject();
        ConferenceMsgsObject    * getConferenceMsgsObject();
        ConferencesObject       * getConferencesObject();
        ConferenceMsgObject     * addMsg(
            const string    &a_type,
            const string    &a_msg_data,
            const string    &a_msg_id,
            const string    &a_src_conf_huid,
            const string    &a_src_conf_id
        );

        string  createPwd();
        string  createMsgId();

        void getConferenceItems(vector <ConferenceItemObject *> &);

        string  getDefaultTopic();
        void    setTopic(const string &a_conf_topic);
        string  getTopic();
        string  getAnyTopic();

        void    setId(const string &a_conf_id);
        string  getId();

        void    setFlags(const string &a_conf_flags);
        string  getFlags();

        void    setPwd0(const string &a_conf_pwd0);
        string  getPwd0();

        void    changed();
};

#endif

