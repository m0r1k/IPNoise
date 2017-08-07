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

using namespace std;

class ContactObject;

#ifndef CONTACT_OBJECT_H
#define CONTACT_OBJECT_H

#include <libxml/tree.h>
#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

class DomDocument;
class ContactsObject;
class ContactItemsObject;
class ContactItemObject;

#include "objects/itemObject.h"
#include "objects/contactAndItem.h"

class ContactObject
    :   public ContactAndItem
{
    public:
        ContactObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact"
        );
        virtual ~ContactObject();

        // internal
        virtual ContactObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_contact"
        );
        virtual void            shutdown();
        virtual void            expired();
        virtual int32_t         autorun();
        virtual void            registrySignals();
        virtual void            registrySlots();
        virtual void            slot(
            const string    &a_signal_name,
            DomElement      *object
        );

        // generic
        enum Id {
            CONTACT_TYPE_EMPTY          = 0,
            CONTACT_TYPE_SELF,
            CONTACT_TYPE_UPDATER,
            // users contacts start from here
            CONTACT_TYPE_USER_DEFINED   = 100
        };

        virtual void            changed();
        bool                    isSystem();
        ContactItemsObject *    getContactItemsObject();
        ContactItemsObject *    getCreateContactItemsObject();
        ContactItemObject  *    getContactItem(const string &a_huid);
        ContactItemObject  *    getCreateContactItem(
            const string &a_huid
        );
        ItemObject *            getCreateItem(
            const string &a_huid
        );
        void                    delContactItem(const string &a_huid);
        void                    removeFromGroups();
        ContactObject::Id       getContactId();
        string                  getContactIdStr();
        void                    setContactId(
            const ContactObject::Id &
        );

        void    getItems(map<string, ItemObject *> &);
        void    getContactItems(map<string, ContactItemObject *> &);

        ItemObject * getSelfItem();

        void    updateOnline();
        void    setOnline(bool a_val);
        bool    isOnline();
        string  isOnlineStr();

        // api
        virtual string  getAPIStatus();
        virtual void    setAPIStatus(const string &);

        virtual string  getAPINickName();
        virtual void    setAPINickName(const string &);

        virtual string  getAPIIcon();
        virtual void    setAPIIcon(const string &);

        virtual string  getAPIRealNickName();
        virtual void    setAPIRealNickName(const string &);

        virtual string  getAPIRealIcon();
        virtual void    setAPIRealIcon(const string &);

        virtual void    getAPIInfo(string  &);
};

#endif

