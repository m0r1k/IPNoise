/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <assert.h>

// stl
#include <string>

class GroupItemObject;

using namespace std;

#ifndef GROUP_ITEM_OBJECT_H
#define GROUP_ITEM_OBJECT_H

#include <ipnoise-common/log.h>

#include "objects/object.h"
#include "objects/contactsObject.h"

class DomDocument;

class GroupItemObject
    :   public Object
{
    public:
        GroupItemObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_group_item"
        );
        virtual ~GroupItemObject();

        // internal
        virtual GroupItemObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_group_item"
        );
        virtual void setAttribute(
            const string    &a_name,
            const string    &a_value
        );
        virtual string getAttribute(const string &a_name);

        // generic
        void setContactId(
            const ContactObject::Id &
        );
        ContactObject::Id   getContactId();
        string              getContactIdStr();
};

#endif

