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

class ConferenceItemsObject;

using namespace std;

#ifndef CONFERENCE_ITEMS_OBJECT_H
#define CONFERENCE_ITEMS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/conferenceObject.h"
#include "objects/conferenceItemObject.h"

class ConferenceItemsObject
    :   public  Object
{
    public:
        ConferenceItemsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_items"
        );
        virtual ~ConferenceItemsObject();

        // internal
        virtual ConferenceItemsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_conference_items"
        );
        virtual int32_t autorun();

        // generic
        ConferenceObject     *  getConferenceObject();
        ConferenceItemObject *  getCreateConferenceItem(
            const string    &huid,
            const string    &a_inform_type = ""
        );
        ConferenceItemObject *  getConferenceItemByHuid(
            const string &
        );
        void getConferenceItems(vector <ConferenceItemObject *> &);
        void getItems(vector <ItemObject *> &);

};

#endif

