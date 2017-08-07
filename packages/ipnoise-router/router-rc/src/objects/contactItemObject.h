/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <assert.h>

// stl
#include <string>

class ContactItemObject;

using namespace std;

#ifndef CONTACT_ITEM_OBJECT_H
#define CONTACT_ITEM_OBJECT_H

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

class ContactItemsObject;
#include "objects/object.h"
#include "objects/itemObject.h"
#include "objects/contactObject.h"

class DomDocument;

class ContactItemObject
    :   public Object
{
    public:
        ContactItemObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact_item"
        );
        virtual ~ContactItemObject();

        // internal
        virtual ContactItemObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact_item"
        );
        virtual void setAttribute(
            const string    &a_name,
            const string    &a_value
        );
        virtual string getAttribute(const string &a_name);

        // generic
        void                    setHuid(const string &);
        string                  getHuid();

        ContactItemsObject  *   getContactItemsObject();
        ItemObject          *   getItem();
};

#endif

