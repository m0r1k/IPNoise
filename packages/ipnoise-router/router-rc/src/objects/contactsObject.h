/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
#include <map>

class ContactsObject;

using namespace std;

#ifndef CONTACTS_OBJECT_H
#define CONTACTS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"
#include "objects/userObject.h"

class DomDocument;

#include "objects/contactObject.h"

class ContactsObject
    :   public  Object
{
    public:
        ContactsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contacts"
        );
        virtual ~ContactsObject();

        // internal
        virtual ContactsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contacts"
        );
        virtual int32_t autorun();

        // generic
        UserObject  *   getUser();
        void            getContacts(
            map<ContactObject::Id, ContactObject *> &a_res
        );
        ContactObject * getContact(
            const ContactObject::Id &
        );
        ContactObject * getCreateContact(
            const ContactObject::Id &a_contact_id
                = ContactObject::CONTACT_TYPE_EMPTY
        );
        ContactObject::Id generateUserContactId();
};

#endif

