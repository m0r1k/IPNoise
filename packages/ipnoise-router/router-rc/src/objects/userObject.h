/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

using namespace std;

class UserObject;

#ifndef USER_OBJECT_H
#define USER_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>

class DomDocument;

#include "objects/contactObject.h"
#include "objects/contactsObject.h"
#include "objects/itemObject.h"
#include "objects/itemsObject.h"
#include "objects/groupItemObject.h"
#include "objects/groupsObject.h"
#include "objects/groupObject.h"
#include "objects/sessionsObject.h"
#include "objects/conferencesObject.h"
#include "objects/conferenceObject.h"
#include "objects/packetObject.h"

#define UPDATER_USER_REAL_NICKNAME "updater"

class UserObject
    :   public Object
{
    public:
        UserObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_user"
        );
        virtual ~UserObject();

        // internal
        virtual UserObject *        create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_user"
        );
        virtual int32_t autorun();
        virtual void    registrySignals();

        // generic
        SessionsObject  *           getCreateSessionsObject();

        // fuse
        virtual string              getFuseName();
        virtual string              parseFuseName(const string &fuse_name);

        // contact list
        void    fillGroups(
            GroupObject     *a_group,
            string          &a_out
        );
        void    fillContacts(
            ContactsObject  *contacts_object,
            string          &a_out
        );
        void    fillItems(
            ItemsObject     *items_object,
            string          &a_out
        );
        void    getContactList(string &a_out);

        // huid
        string                      getHuid();
        void                        setHuid(const string &a_huid);

        // contacts
        ContactObject   *   getSelfContact();
        ContactsObject  *   getCreateContactsObject();
        ItemObject      *   getCreateSelfItem();
        ContactObject   *   getCreateSelfContact();
        ContactObject   *   getCreateContact(
            const ContactObject::Id &a_contact_id
                = ContactObject::CONTACT_TYPE_EMPTY
        );
        ContactObject   *   getContact(
            const ContactObject::Id &a_contact_id
        );
        void delContact(const ContactObject::Id &);

        // items
        ItemObject      *   getCreateItem(const string &huid);
        ItemObject      *   addCreateItem(const string &huid);
        void                delItem(const string &huid);
        ItemObject      *   getItem(const string &huid);
        ItemsObject     *   getCreateItemsObject();
        void                getItems(vector<ItemObject *> &a_out);
        GroupItemObject *   addContactToNonInListGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToNonInListGroup(
            ContactObject *
        );
        GroupItemObject *   addContactToRootGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToRootGroup(ContactObject *);
        GroupItemObject *   addContactToMyselfGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToMyselfGroup(ContactObject *);

        // groups
        GroupsObject    *       getCreateGroupsObject();
        GroupObject     *       getGroupById(string &group_id);
        GroupObject     *       getGroupById(int group_id);

        // conferences
        ConferencesObject   *   getConferencesObject();
        ConferenceObject    *   getConferenceById(const string &);
        ConferenceObject    *   createConference(
            const string    &a_conf_flags,
            const string    &a_conf_topic
        );

        ConferenceObject    *   createConferenceById(
            const string    &a_conf_id,
            const string    &a_conf_flags,
            const string    &a_conf_topic
        );

        void getAPIInfo(string &a_out);

        // queue
        PacketObject *  queue(const string &a_data);
        PacketObject *  queue(DomElement *a_data);
};

#endif

