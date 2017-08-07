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

class ConferenceItemObject;

using namespace std;

#ifndef CONFERENCE_ITEM_OBJECT_H
#define CONFERENCE_ITEM_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceItemsObject.h"
#include "objects/conferenceObject.h"

class ConferenceItemObject
    :   public  Object
{
    public:
        ConferenceItemObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_item"
        );
        virtual ~ConferenceItemObject();

        // internal
        virtual ConferenceItemObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_item"
        );
        virtual int32_t autorun();

        // generic
        ConferenceObject        * getConferenceObject();
        ConferenceItemsObject   * getConferenceItemsObject();
        ConferenceItemObject    * getConferenceItemByHuid(const string&);

        ItemObject *    getItem();
        string          createPwd();
        void            setPwd(const string &huid);
        string          getPwd();

        void            setHuid(const string &huid);
        string          getHuid();

        void            setSrcConfId(const string &src_conf_id);
        string          getSrcConfId();

        void            setSrcPwd(const string &src_pwd);
        string          getSrcPwd();

        ssize_t         queue(const string &a_data);
        ssize_t         queue(DomElement *);

        void            addLinks(
            const string    &a_huid,
            DomElement      *a_out
        );
        int             doInvite();
};

#endif

