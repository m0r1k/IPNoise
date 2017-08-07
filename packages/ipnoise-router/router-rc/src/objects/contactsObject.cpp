/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/contactsObject.h"

ContactsObject * ContactsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ContactsObject(a_node, a_doc, a_tagname);
}

ContactsObject::ContactsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ContactsObject::~ContactsObject()
{
};

int32_t ContactsObject::autorun()
{
    return 0;
}

UserObject * ContactsObject::getUser()
{
    UserObject *user = NULL;

    user = (UserObject *)getParentNodeByTagName(
        "ipn_user"
    );

    return user;
}

void ContactsObject::getContacts(
    map<ContactObject::Id, ContactObject *> &a_res)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_i;

    getElementsByTagName("ipn_contact", res);
    for (res_i = res.begin();
        res_i != res.end();
        res_i++)
    {
        ContactObject *contact = (ContactObject *)*res_i;
        a_res[contact->getContactId()] = contact;
    }
}

ContactObject * ContactsObject::getContact(
    const ContactObject::Id &a_contact_id)
{
    char tmp[1024] = { 0x00 };
    ContactObject *contact = NULL;

    // search contact
    snprintf(tmp, sizeof(tmp),
        "//ipn_contact[@contact_id=\"%d\"]",
        a_contact_id
    );
    contact = (ContactObject *)getElementByXpath(tmp);

    return contact;
}

ContactObject::Id ContactsObject::generateUserContactId()
{
    ContactObject::Id id = ContactObject::CONTACT_TYPE_USER_DEFINED;

    map<ContactObject::Id, ContactObject *>             res;
    map<ContactObject::Id, ContactObject *>::iterator   res_it;

    getContacts(res);
    do {
        res_it = res.find(id);
        if (res.end() == res_it){
            break;
        }
        id = ContactObject::Id(int(id) + 1);
    } while (1);

    return id;
}

ContactObject * ContactsObject::getCreateContact(
    const ContactObject::Id &a_contact_id)
{
    ContactObject::Id   contact_id  = a_contact_id;
    ContactObject       *contact    = NULL;

    if (ContactObject::CONTACT_TYPE_EMPTY != contact_id){
        contact = getContact(contact_id);
    }
    if (contact){
        // already exist
        goto out;
    }

    // create contact
    contact = getDocument()->createElement<ContactObject>(
        "ipn_contact");
    if (not contact){
        PERROR_OBJ(this, "Cannot create 'ipn_contact' element\n");
        goto fail;
    }
    appendChild(contact);

    if (ContactObject::CONTACT_TYPE_EMPTY == contact_id){
        // generate contact ID
        contact_id = generateUserContactId();
    }

    // do setContactId after item will be attached in DOM
    contact->setContactId(contact_id);

    // add to all items
    getUser()
        ->getCreateGroupsObject()
        ->addContactToGroup(
        GroupsObject::GROUP_TYPE_ALL_ITEMS,
        contact
    );

    // inform all what new item was created
    getDocument()->emitSignal(
        "new_contact_was_created",
        (DomElement *)contact
    );

out:
    return contact;

fail:
    goto out;
}

    /*
     * Attention, HUID always must have full length, don't truncate it!
     * Example:
     *   will work      2210:0000:0000:0000:0000:0000:0001:0001
     *   will not work  2210::1
     *
     * Use in6addr2huid to convert in6addr to huid
     *
     * 20110816 morik@
     */

/*
    // updater huid
    updater_huid = "2210:0000:0000:0000:0000:0000:0001:0001";

    updater = getItem(updater_huid);
    if (!updater){
        // Create "updater" item
        updater = addCreateItem(updater_huid);
        updater->setAPIRealNickName("updater");
    }

    // add in group
    groups->addItemToGroup(
        GroupsObject::GROUP_TYPE_SERVICES,
        updater
    );

    neighs->addNeigh(updater_huid, "tcp_v4_0", "192.168.226.2:22102");
    neighs->addNeigh(updater_huid, "udp_v4_0", "192.168.226.2:22102");
    //neighs->addNeigh(updater_huid, "tcp_v4_0", "95.31.26.54:22102");
    //neighs->addNeigh(updater_huid, "udp_v4_0", "95.31.26.54:22102");

*/ 

