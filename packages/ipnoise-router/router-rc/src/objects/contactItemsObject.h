/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
#include <map>

class ContactItemsObject;

using namespace std;

#ifndef CONTACT_ITEMS_OBJECT_H
#define CONTACT_ITEMS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;
class UserObject;
class ContactObject;

#include "objects/contactItemObject.h"

class ContactItemsObject
    :   public  Object
{
    public:
        ContactItemsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact_items"
        );
        virtual ~ContactItemsObject();

        // internal
        virtual ContactItemsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact_items"
        );
        virtual int32_t     autorun();
        virtual void        setAttribute(
            const string    &a_name,
            const string    &a_value
        );
        virtual string      getAttribute(const string &a_name);

        // generic
        UserObject          *   getUserObject();
        ContactObject       *   getContactObject();
        ContactItemObject   *   getContactItem(const string &);
        ContactItemObject   *   getCreateContactItem(const string &);
        void getItems(map<string, ItemObject *> &);
        void getContactItems(map<string, ContactItemObject *> &);
        void delContactItem(const string &a_huid);

};

#endif

