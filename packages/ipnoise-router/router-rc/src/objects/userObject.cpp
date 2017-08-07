/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

#include "libxml2/domDocument.h"
#include "objects/groupObject.h"
#include "objects/groupsObject.h"
#include "objects/groupItemObject.h"
#include "objects/neighsObject.h"
#include "objects/ipnoiseObject.h"

#include "objects/userObject.h"

UserObject * UserObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new UserObject(a_node, a_doc, a_tagname);
}

UserObject::UserObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

UserObject::~UserObject()
{
    string tag_name;
    string huid = getHuid();
    string std_out, std_err;

    if (not huid.empty()){
        // ok, trying to remove this user
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-user",
            "del",
            huid.c_str(),
            NULL
        };
        my_system(argv, std_out, std_err);
    }
};

void UserObject::registrySignals()
{
    getDocument()->registrySignal(
        "new_user_was_created",
        "ipn_user"
    );
    getDocument()->registrySignal(
        "new_item_was_created",
        "ipn_item"
    );
}

int32_t UserObject::autorun()
{
    int             err                 = 0;
    int             priveleges          = 0;
    ContactsObject  *contacts           = NULL;
    IPNoiseObject   *ipnoise            = NULL;

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());

    priveleges = getDocument()->getPriveleges();
    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

    // groups
    getCreateGroupsObject();

    // neighs object
    // ipnoise->getServerObject()->getCreateNeighsObject();

    // create "ipn_sessions" element if not exist
    getCreateSessionsObject();

    // create 'conferences' object
    getConferencesObject();

    // create 'items' object
    getCreateItemsObject();

    // create 'contacts' object
    contacts = getCreateContactsObject();
    contacts->getCreateContact(
        ContactObject::CONTACT_TYPE_UPDATER
    )->setAPIRealNickName(UPDATER_USER_REAL_NICKNAME);

    // create 'self contact'
    getCreateSelfContact();

    getDocument()->emitSignal(
        "new_user_was_created",
        this
    );

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
};

SessionsObject * UserObject::getCreateSessionsObject()
{
    return getCreateSelfItem()->getCreateSessionsObject();
}

string UserObject::getHuid()
{
    return getAttribute("huid");
};

ContactObject * UserObject::getSelfContact()
{
    ContactObject   *contact        = NULL;
    ContactsObject  *contacts_obj   = NULL;

    contacts_obj    = getCreateContactsObject();
    contact         = contacts_obj->getContact(
        ContactObject::CONTACT_TYPE_SELF
    );

    return contact;
}

ItemObject * UserObject::getCreateSelfItem()
{
    ItemObject  *item = NULL;
    item = getCreateSelfContact()->getSelfItem();
    return item;
}

ContactObject * UserObject::getCreateSelfContact()
{
    ContactObject *self_contact = NULL;

    self_contact = getSelfContact();
    if (self_contact){
        // already exist
        goto out;
    }

    self_contact = getCreateContact(
        ContactObject::CONTACT_TYPE_SELF
    );
    self_contact->getCreateItem(getHuid());
    addContactToMyselfGroup(self_contact);

out:
    return self_contact;
}

void UserObject::setHuid(
    const string &a_huid)
{
    string  old_huid;

    if (a_huid.empty()){
        PERROR_OBJ(this, "attempt to set empty huid for user\n");
        goto out;
    }

    // if old ourself exist, remove it
    old_huid = getHuid();
    if (not old_huid.empty()){
        delItem(old_huid);
    }

    setAttribute("huid", a_huid);

out:
    return;
};

/*
 *  parseFuseName - should return element tag name from fuse name
 *
 *  fuse name for user object is usr:huid,
 *  example: user:2210:0:7309:227d:3030:fb23:816a:cc5f
 *
 */
string UserObject::parseFuseName(const string &fuse_name)
{
    char buffer[1024] = { 0x00 };
    int i = 0;

    snprintf(buffer, sizeof(buffer), "%s", fuse_name.c_str());
    for (i = 0; i < (int)sizeof(buffer); i++){
        if (buffer[i] == '\0'){
            break;
        }
        if (buffer[i] == ':'){
            buffer[i] = '\0';
            break;
        }
    }
    return string(buffer);
}

string UserObject::getFuseName()
{
    char buffer[1024]   = { 0x00 };

    snprintf(buffer, sizeof(buffer), "%s:%s",
        getTagName().c_str(),
        getHuid().c_str()
    );

    return string(buffer);
}

// ------------------ contact list ----------------------------- {

void UserObject::fillGroups(
    GroupObject     *a_group,
    string          &a_out)
{
    vector<GroupObject *>               sub_groups;
    vector<GroupObject *>::iterator     sub_groups_it;

    vector<GroupItemObject *>           group_items;
    vector<GroupItemObject *>::iterator group_items_it;

    string is_system = a_group->isSystemGroup() ? "true" : "false";

    // get sub groups and group items
    a_group->getSubGroups(sub_groups);
    a_group->getGroupItems(group_items);

    a_out += "<group";
    a_out +=    " id=\""        + a_group->getGroupId()     + "\"";
    a_out +=    " name=\""      + a_group->getGroupName()   + "\"";
    a_out +=    " is_system=\"" + is_system                 + "\"";

    if (!sub_groups.size() && !group_items.size()){
        // we have not sub groups and group items
        a_out += "/>";
        goto out;
    }

    // we have childs
    a_out += ">";

    // get sub group
    for (sub_groups_it = sub_groups.begin();
        sub_groups_it != sub_groups.end();
        sub_groups_it++)
    {
        GroupObject *group = *sub_groups_it;
        fillGroups(group, a_out);
    }

    // get group items
    for (group_items_it = group_items.begin();
        group_items_it != group_items.end();
        group_items_it++)
    {
        GroupItemObject *group_item = *group_items_it;
        a_out += "<group_item"
            " contact_id=\"" + group_item->getContactIdStr() + "\""
            "/>";
    }

    a_out += "</group>";

out:
    return;
}

void UserObject::fillItems(
    ItemsObject     *items_object,
    string          &a_out)
{
    vector<ItemObject *>            items;
    vector<ItemObject *>::iterator  items_it;

    items_object->getItems(items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        ItemObject *item = *items_it;
        item->getAPIInfo(a_out);
    }
}

void UserObject::fillContacts(
    ContactsObject  *contacts_object,
    string          &a_out)
{
    map<ContactObject::Id, ContactObject *>            contacts;
    map<ContactObject::Id, ContactObject *>::iterator  contacts_it;

    contacts_object->getContacts(contacts);
    for (contacts_it = contacts.begin();
        contacts_it != contacts.end();
        contacts_it++)
    {
        ContactObject *contact = contacts_it->second;
        contact->getAPIInfo(a_out);
    }
}

void UserObject::getContactList(string &a_out)
{
    GroupsObject    *groups_object      = NULL;
    GroupObject     *root_group         = NULL;
    ContactsObject  *contacts_object    = NULL;
    ItemsObject     *items_object       = NULL;

    groups_object   = getCreateGroupsObject();
    root_group      = groups_object->getRootGroup();
    contacts_object = getCreateContactsObject();
    items_object    = getCreateItemsObject();

    // fill groups
    {
        a_out += "<groups";
        string groups_res;
        fillGroups(root_group, groups_res);
        if (groups_res.size()){
            a_out += ">";
            a_out += groups_res;
            a_out += "</groups>";
        } else {
            a_out += "/>";
        }
    }

    // fill contacts
    {
        a_out += "<contacts";
        string contacts_res;
        fillContacts(contacts_object, contacts_res);
        if (contacts_res.size()){
            a_out += ">";
            a_out += contacts_res;
            a_out += "</contacts>";
        } else {
            a_out += "/>";
        }
    }

    // fill items
    {
        a_out += "<items";
        string items_res;
        fillItems(items_object, items_res);
        if (items_res.size()){
            a_out += ">";
            a_out += items_res;
            a_out += "</items>";
        } else {
            a_out += "/>";
        }
    }
}

// ------------------ contact list ----------------------------- }

// ------------------ group and group's items ------------------ {

ItemsObject * UserObject::getCreateItemsObject()
{
    ItemsObject *items_obj = NULL;

    items_obj = (ItemsObject *)getElementByTagName("ipn_items");
    if (!items_obj){
        items_obj = getDocument()->createElement<ItemsObject>(
            "ipn_items"
        );
        appendChildSafe(items_obj);
    }

    return items_obj;
}

ContactsObject * UserObject::getCreateContactsObject()
{
    ContactsObject *contacts_obj = NULL;

    contacts_obj = (ContactsObject *)getElementByTagName(
        "ipn_contacts");
    if (!contacts_obj){
        contacts_obj = getDocument()->createElement<ContactsObject>(
            "ipn_contacts");
        appendChildSafe(contacts_obj);
    }

    return contacts_obj;
}

void UserObject::getItems(vector<ItemObject *> &a_out)
{
    ItemsObject *items = NULL;
    items = getCreateItemsObject();
    items->getItems(a_out);
}

ItemObject * UserObject::getItem(const string &a_huid)
{
    ItemObject  *item       = NULL;
    ItemsObject *items_obj  = NULL;

    items_obj   = getCreateItemsObject();
    item        = items_obj->getItem(a_huid);

    return item;
}

ContactObject * UserObject::getContact(
    const ContactObject::Id &a_contact_id)
{
    ContactObject  *contact       = NULL;
    ContactsObject *contacts_obj  = NULL;

    contacts_obj   = getCreateContactsObject();
    contact        = contacts_obj->getContact(a_contact_id);

    return contact;
}

ContactObject * UserObject::getCreateContact(
    const ContactObject::Id &a_contact_id)
{
    ContactObject   *contact        = NULL;
    ContactsObject  *contacts_obj   = NULL;

    // get contacts object
    contacts_obj = getCreateContactsObject();

    if (ContactObject::CONTACT_TYPE_EMPTY != a_contact_id){
        contact = contacts_obj->getContact(
            a_contact_id
        );
    }
    if (contact){
        // already exist
        goto out;
    }

    contact = contacts_obj->getCreateContact(a_contact_id);

out:
    return contact;
}

ItemObject * UserObject::getCreateItem(const string &a_huid)
{
    ItemObject  *item       = NULL;
    ItemsObject *items_obj  = NULL;

    if (a_huid.empty()){
        PERROR_OBJ(this, "attempt to create item with empty huid\n");
        goto out;
    }

    item = getItem(a_huid);
    if (item){
        // already exist
        goto out;
    }

    // get items object
    items_obj = getCreateItemsObject();

    // create item
    item = items_obj->getCreateItem(a_huid);
    if (!item){
        PERROR_OBJ(this, "Cannot create 'ipn_item' element\n");
        goto fail;
    }

out:
    return item;

fail:
    goto out;
}

GroupsObject * UserObject::getCreateGroupsObject()
{
    GroupsObject *groups = NULL;

    groups = (GroupsObject *)getElementByTagName("ipn_groups");
    if (!groups){
        groups = getDocument()->createElement<GroupsObject>("ipn_groups");
        appendChildSafe(groups);
    }

    return groups;
}

GroupObject * UserObject::getGroupById(string &group_id)
{
    GroupsObject *groups    = NULL;
    GroupObject  *group     = NULL;

    groups  = getCreateGroupsObject();
    group   = groups->getGroupById(group_id);

    return group;
}

GroupObject * UserObject::getGroupById(int group_id)
{
    GroupsObject *groups    = NULL;
    GroupObject  *group     = NULL;

    groups  = getCreateGroupsObject();
    group   = groups->getGroupById(group_id);

    return group;
}

GroupItemObject * UserObject::addContactToRootGroup(
    ContactObject *a_contact)
{
    GroupItemObject *ret = NULL;

    const ContactObject::Id contact_id = a_contact->getContactId();
    ret = addContactToRootGroup(contact_id);

    return ret;
}

GroupItemObject * UserObject::addContactToRootGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupsObject *groups = NULL;

    // get groups element
    groups = getCreateGroupsObject();

    return groups->addContactToRootGroup(a_contact_id);
}

GroupItemObject * UserObject::addContactToMyselfGroup(
    ContactObject *a_contact)
{
    GroupItemObject *ret = NULL;

    const ContactObject::Id contact_id = a_contact->getContactId();
    ret = addContactToMyselfGroup(contact_id);

    return ret;;
}

GroupItemObject * UserObject::addContactToMyselfGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupsObject *groups = NULL;

    // get groups element
    groups = getCreateGroupsObject();

    return groups->addContactToMyselfGroup(a_contact_id);
}

void UserObject::delContact(
    const ContactObject::Id &a_contact_id)
{
    GroupsObject    *groups     = NULL;
    ContactObject   *contact    = NULL;

    if (ContactObject::CONTACT_TYPE_SELF == a_contact_id){
        PERROR_OBJ(this,
            "Attempt to delete ourself from contact list"
            " (operation will be ignored)\n");
        goto out;
    }

    // get groups element
    groups = getCreateGroupsObject();

    // delete from "All items" group
    groups->delContactFromAllItemsGroup(a_contact_id);

    // search contact
    contact = getContact(a_contact_id);
    if (contact){
        DomDocument::deleteElement(contact);
    }

out:
    return;
}

void UserObject::delItem(const string &huid)
{
    ItemObject *item = NULL;

    if (huid.empty()){
        PERROR_OBJ(this, "Attempt to delete item with empty huid\n");
        goto out;
    }

    if (huid == getHuid()){
        PERROR_OBJ(this, "Attempt to delete ourself from contact list"
            " (operation will be ignored)\n");
        goto out;
    }

    // search item
    item = getItem(huid);
    if (item){
        DomDocument::deleteElement(item);
    }

out:
    return;
}

ItemObject * UserObject::addCreateItem(const string &huid)
{
    ItemObject * item = NULL;

    if (huid.empty()){
        PERROR_OBJ(this, "attempt to add item with empty huid\n");
        goto out;
    }

    item = getCreateItem(huid);

out:
    return item;
}

GroupItemObject * UserObject::addContactToNonInListGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupsObject    *groups     = NULL;
    GroupItemObject *group_item = NULL;

    // get groups element
    groups      = getCreateGroupsObject();
    group_item  = groups->addContactToNonInListGroup(a_contact_id);

    return group_item;
}

GroupItemObject * UserObject::addContactToNonInListGroup(
    ContactObject *a_contact)
{
    GroupItemObject *ret = NULL;

    ContactObject::Id id = a_contact->getContactId();
    ret = addContactToNonInListGroup(id);

    return ret;
}

// ------------------ group and group's items ------------------ }

// ------------------ conferences ------------------ {
ConferencesObject * UserObject::getConferencesObject()
{
    ConferencesObject *conferences = NULL;

    conferences = (ConferencesObject *)getElementByTagName("ipn_conferences");
    if (!conferences){
        conferences = getDocument()
            ->createElement<ConferencesObject>("ipn_conferences");
        appendChildSafe(conferences);
    }

    return conferences;
}

ConferenceObject * UserObject::getConferenceById(const string &conf_id)
{
    ConferencesObject *conferences    = NULL;
    ConferenceObject  *conference     = NULL;

    conferences = getConferencesObject();
    conference  = conferences->getConferenceById(conf_id);

    return conference;
}

ConferenceObject * UserObject::createConference(
    const string    &a_conf_flags,
    const string    &a_conf_topic)
{
    ConferencesObject   *conferences    =   NULL;
    ConferenceObject    *conference     =   NULL;

    conferences = getConferencesObject();
    conference  = conferences->createConference(
        a_conf_flags,
        a_conf_topic
    );

    return conference;
}

ConferenceObject * UserObject::createConferenceById(
    const string    &a_conf_id,
    const string    &a_conf_flags,
    const string    &a_conf_topic)
{
    ConferencesObject   *conferences    =   NULL;
    ConferenceObject    *conference     =   NULL;

    conferences = getConferencesObject();
    conference  = conferences->createConferenceById(
        a_conf_id,
        a_conf_flags,
        a_conf_topic
    );

    return conference;
}

// ------------------ conferences ------------------ }

void UserObject::getAPIInfo(
    string  &a_out)
{
    ItemObject *self_item = getCreateSelfItem();

    a_out += "<user>";
    a_out +=   "<status><![CDATA[";
    a_out +=       self_item->getAPIStatus();
    a_out +=   "]]></status>";
    a_out +=   "<nickname><![CDATA[";
    a_out +=       self_item->getAPINickName();
    a_out +=   "]]></nickname>";
    a_out +=   "<icon><![CDATA[";
    a_out +=       self_item->getAPIIcon();
    a_out +=   "]]></icon>";
    a_out +=   "<real_nickname><![CDATA[";
    a_out +=       self_item->getAPIRealNickName();
    a_out +=   "]]></real_nickname>";
    a_out +=   "<real_icon><![CDATA[";
    a_out +=       self_item->getAPIRealIcon();
    a_out +=   "]]></real_icon>";
    a_out += "</user>";
};

PacketObject * UserObject::queue(const string &a_data)
{
    PacketObject *receiver = NULL;
    receiver = getCreateSelfItem()
        ->getCreateSessionsObject()
        ->queue(a_data);
    return receiver;
}

PacketObject * UserObject::queue(DomElement *a_data)
{
    PacketObject *receiver = NULL;
    receiver = getCreateSelfItem()
        ->getCreateSessionsObject()
        ->queue(a_data);
    return receiver;
}

