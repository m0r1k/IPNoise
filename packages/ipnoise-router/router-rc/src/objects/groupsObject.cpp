/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/groupObject.h"
#include "objects/itemObject.h"
#include "objects/contactObject.h"

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

#include "objects/groupsObject.h"

GroupsObject * GroupsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new GroupsObject(a_node, a_doc, a_tagname);
}

GroupsObject::GroupsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

GroupsObject::~GroupsObject()
{
};

int32_t GroupsObject::autorun()
{
    int err         = 0;
    int priveleges  = 0;

    // create generic groups
    GroupObject *root_group         = NULL;
    GroupObject *my_self_group      = NULL;
    GroupObject *services_group     = NULL;
    GroupObject *non_in_list_group  = NULL;
    GroupObject *all_users_group    = NULL;

    priveleges = getDocument()->getPriveleges();

    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

    root_group = getGroupById(GROUP_TYPE_ROOT);
    if (!root_group){
        root_group = createGroup(GROUP_TYPE_ROOT);
    }

    my_self_group = getGroupById(GROUP_TYPE_MY_SELF);
    if (!my_self_group){
        my_self_group = createGroup(GROUP_TYPE_MY_SELF);
    }

    all_users_group = getGroupById(GROUP_TYPE_ALL_ITEMS);
    if (!all_users_group){
        all_users_group = createGroup(GROUP_TYPE_ALL_ITEMS);
    }

    services_group = getGroupById(GROUP_TYPE_SERVICES);
    if (!services_group){
        services_group = createGroup(GROUP_TYPE_SERVICES);
    }

    non_in_list_group = getGroupById(GROUP_TYPE_NON_IN_LIST);
    if (!non_in_list_group){
        non_in_list_group = createGroup(GROUP_TYPE_NON_IN_LIST);
    }

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

GroupObject * GroupsObject::getRootGroup()
{
    GroupObject *group = NULL;
    group = getGroupById(GROUP_TYPE_ROOT);
    return group;
}

void GroupsObject::setAttribute(
    const string    &a_name,
    const string    &a_value)
{
    PERROR_OBJ(this, "Attempt to set attribute: '%s', value: '%s',"
        "use accessors instead\n",
        a_name.c_str(), a_value.c_str()
    );
    assert(0);
}

string GroupsObject::getAttribute(
    const string &a_name)
{
    PERROR_OBJ(this, "Attempt to read attribute: '%s',"
        " use accessors instead\n",
        a_name.c_str());
    assert(0);
}

bool GroupsObject::isStrategySupported(
    GroupDelStrategy    a_strategy)
{
    bool ret = false;

    if (    a_strategy >= GROUP_DEL_STRATEGY_0
        &&  a_strategy < GROUP_DEL_STRATEGY_MAX)
    {
        ret = true;
    }

    return ret;
}

int GroupsObject::delGroupStrategy_0(
    int a_group_id)
{
    int err = -1;

    // ok, our strategy is:
    // 1. Delete group recursive (don't delete ItemObject(s))

    GroupObject *group  = NULL;

    // search group
    group = getGroupById(a_group_id);
    if (!group){
        PERROR_OBJ(this, "Attempt to delete non-exist group"
            " with ID: '%d'\n",
            a_group_id
        );
        goto fail;
    }

    // delete group recursive
    DomDocument::deleteElement(group);

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void GroupsObject::getAllGroupsItems(
    vector<GroupItemObject *>     &a_out)
{
    vector <DomElement *>               res;
    vector <DomElement *>::iterator     res_it;

    // search all group items
    res.clear();
    getElementsByXpath("//ipn_group_item", res);

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        GroupItemObject *item = (GroupItemObject *)*res_it;
        a_out.push_back(item);
    }
}

int GroupsObject::delGroupStrategy_1(
    int a_group_id)
{
    // ok, our strategy are:
    // 1. Delete group recursive (don't delete ItemObject(s))
    // 2. Delete ItemObject(s)

    int err = -1;

    map<ContactObject::Id, int >            huids_for_del;
    map<ContactObject::Id, int >::iterator  huids_for_del_it;

    vector<GroupItemObject *>               items;
    vector<GroupItemObject *>::iterator     items_it;

    GroupObject *group  = NULL;
    UserObject  *user   = NULL;

    // search group
    group = getGroupById(a_group_id);
    if (!group){
        PERROR_OBJ(this, "Attempt to delete non-exist group"
            " with ID: '%d'\n",
            a_group_id
        );
        goto fail;
    }

    // search user
    user = getOwner();
    assert(user);

    // search all group items
    group->getAllGroupsItems(items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        GroupItemObject *group_item = *items_it;
        huids_for_del[group_item->getContactId()] = 1;
    }

    // delete group recursive
    DomDocument::deleteElement(group);

    for (huids_for_del_it = huids_for_del.begin();
        huids_for_del_it != huids_for_del.end();
        huids_for_del_it++)
    {
        user->delContact(huids_for_del_it->first);
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int GroupsObject::delGroup(
    int                 a_group_id,
    GroupDelStrategy    a_strategy)
{
    int res, err = -1;

    if (GROUP_DEL_STRATEGY_0 == a_strategy){
        res = GroupsObject::delGroupStrategy_0(a_group_id);
        if (res < 0){
            err = res;
            goto fail;
        }
    } else if (GROUP_DEL_STRATEGY_1 == a_strategy){
        res = GroupsObject::delGroupStrategy_1(a_group_id);
        if (res < 0){
            err = res;
            goto fail;
        }
    } else {
        PERROR_OBJ(this,
            "Unsupported group delete strategy: '%d'\n",
            a_strategy
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

GroupObject * GroupsObject::getGroupById(int a_group_id)
{
    char        tmp[1024]   = { 0x00 };
    GroupObject *group      = NULL;

    snprintf(tmp, sizeof(tmp),
        "//ipn_group[@id=\"%x\"]",
        a_group_id
    );
    group = (GroupObject *)getElementByXpath(tmp);

    return group;
}

GroupObject * GroupsObject::getGroupById(
    const string &a_group_id)
{
    return getGroupById(atoi(a_group_id.c_str()));
}

GroupItemObject * GroupsObject::getContactFromGroup(
    int                     a_group_id,
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *group_item = NULL;
    GroupObject     *group      = NULL;
    vector<DomElement *> res;

    group = getGroupById(a_group_id);
    if (!group){
        PERROR_OBJ(this,
            "Cannot getContactFromGroup: '%d',"
            " group not exist\n",
            a_group_id
        );
        goto out;
    }

    group_item = group->getGroupItem(a_contact_id);

out:
    return group_item;
}

GroupObject * GroupsObject::createGroup(
    const int32_t   &a_group_id,
    const int32_t   &a_parent_group_id)
{
    GroupObject *group          = NULL;
    GroupObject *parent_group   = NULL;
    string group_name           = DEFAULT_NEW_GROUP_NAME;

    if (a_group_id < 0){
        PERROR_OBJ(this, "Invalid group ID: '%d'\n",
            a_group_id);
        goto out;
    }

    // search group
    group = getGroupById(a_group_id);
    if (group){
        goto out;
    }

    // check parent group ID
    if (a_parent_group_id < 0){
        PERROR_OBJ(this, "Invalid parent group ID: '%d'\n",
            a_parent_group_id);
        goto out;
    }

    // search parent
    if (GROUP_TYPE_ROOT != a_group_id){
        parent_group = getGroupById(a_parent_group_id);
        if (!parent_group){
            PERROR_OBJ(this,
                "Parrent group with ID: '%d' not exist\n",
                a_parent_group_id
            );
            goto out;
        }
    }

    // ok, create new group
    group = getDocument()->createElement<GroupObject>("ipn_group");
    group->setGroupId(a_group_id);

    if (a_group_id == GROUP_TYPE_ROOT){
        group_name = GROUP_TYPE_NAME_ROOT;
    } else if (a_group_id == GROUP_TYPE_MY_SELF){
        group_name = GROUP_TYPE_NAME_MY_SELF;
    } else if (a_group_id == GROUP_TYPE_SERVICES){
        group_name = GROUP_TYPE_NAME_SERVICES;
    } else if (a_group_id == GROUP_TYPE_NON_IN_LIST){
        group_name = GROUP_TYPE_NAME_NON_IN_LIST;
    } else if (a_group_id == GROUP_TYPE_ALL_ITEMS){
        group_name = GROUP_TYPE_NAME_ALL_ITEMS;
    } else {
        group_name = DEFAULT_NEW_GROUP_NAME;
    }

    group->setGroupName(group_name);

    // append to parent (if exist)
    if (parent_group){
        parent_group->appendChild(group);
    } else {
        appendChild(group);
    }

out:
    return group;
}

GroupObject * GroupsObject::smartCreateGroup(
    int     a_group_id,
    int     a_parent_group_id)
{
    GroupObject *group = NULL;

    if (-1 == a_group_id){
        a_group_id = GROUP_TYPE_USER_DEFINED;
    }

    while (1) {
        // search free group number
        group = getGroupById(a_group_id);
        if (!group){
            break;
        }
        // check next id
        a_group_id++;
    }

    // ok, create new group
    group = createGroup(a_group_id, a_parent_group_id);

    return group;
}

void GroupsObject::delContactFromGroup(
    int                     a_group_id,
    const ContactObject::Id &a_contact_id)
{
    GroupObject *group  = NULL;

    group = getGroupById(a_group_id);
    if (!group){
        // not exist
        PERROR_OBJ(this, "Cannot delContactFromGroup,"
            " group with ID: '%d' not exist\n",
            a_group_id
        );
        goto fail;
    }

    // delete contact from group
    group->delGroupItem(a_contact_id);

out:
    return;

fail:
    goto out;
}

GroupItemObject * GroupsObject::addContactToGroup(
    int                        a_group_id,
    const ContactObject::Id    &a_contact_id)
{
    GroupObject     *group  = NULL;
    GroupItemObject *item   = NULL;

    // 1. add contact to 'all items' group
    {
        // search group
        group = getGroupById(GROUP_TYPE_ALL_ITEMS);
        if (!group){
            // not exist
            PERROR_OBJ(this, "Cannot addItemToGroup,"
                " group with ID: '%d' not exist\n",
                a_group_id
            );
            goto fail;
        }

        // search item
        if (not group->getGroupItem(a_contact_id)){
            // add contact in group
            group->addGroupItem(a_contact_id);
        }
    }

    // 2. add contact to requested group
    {
        // search group
        group = getGroupById(a_group_id);
        if (!group){
            // not exist
            PERROR_OBJ(this, "Cannot addItemToGroup,"
                " group with ID: '%d' not exist\n",
                a_group_id
            );
            goto fail;
        }

        // search item
        item = group->getGroupItem(a_contact_id);
        if (not item){
            // add contact in group
            item = group->addGroupItem(a_contact_id);
        }
    }

out:
    return item;
fail:
    goto out;
}

GroupItemObject * GroupsObject::addContactToGroup(
    int             a_group_id,
    ContactObject   *a_contact)
{
    const ContactObject::Id contact_id = a_contact->getContactId();
    return addContactToGroup(a_group_id, contact_id);
}

GroupItemObject * GroupsObject::addContactToMyselfGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *item   = NULL;
    int group_id            = GROUP_TYPE_MY_SELF;

    item = addContactToGroup(group_id, a_contact_id);
    if (!item){
        PERROR_OBJ(this,
            "Cannot add new contact with ID: '%d'"
            " to group with ID: '%d'",
            a_contact_id,
            group_id
        );
        goto fail;
    }

out:
    return item;
fail:
    goto out;
}

GroupItemObject * GroupsObject::addContactToMyselfGroup(
    ContactObject  *a_contact)
{
    GroupItemObject     *group_item = NULL;
    ContactObject::Id   contact_id;

    contact_id  = a_contact->getContactId();
    group_item  = addContactToMyselfGroup(contact_id);

    return group_item;
}

void GroupsObject::delContactFromAllItemsGroup(
    const ContactObject::Id &a_contact_id)
{
    int group_id = GROUP_TYPE_ALL_ITEMS;

    delContactFromGroup(group_id, a_contact_id);
}

void GroupsObject::delContactFromAllItemsGroup(
    ContactObject  *a_contact)
{
    ContactObject::Id contact_id;

    contact_id = a_contact->getContactId();
    delContactFromAllItemsGroup(contact_id);
}

GroupItemObject * GroupsObject::addContactToNonInListGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *item   = NULL;
    int group_id            = GROUP_TYPE_NON_IN_LIST;

    item = addContactToGroup(group_id, a_contact_id);
    if (!item){
        PERROR_OBJ(this,
            "Cannot add new contact with ID: '%d'"
            " to group with ID: '%d'",
            a_contact_id,
            group_id
        );
        goto fail;
    }

out:
    return item;
fail:
    goto out;
}

GroupItemObject * GroupsObject::addContactToNonInListGroup(
    ContactObject  *a_contact)
{
    GroupItemObject     *group_item = NULL;
    ContactObject::Id   contact_id;

    contact_id  = a_contact->getContactId();
    group_item  = addContactToNonInListGroup(contact_id);

    return group_item;
}

GroupItemObject * GroupsObject::addContactToRootGroup(
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *item   = NULL;
    int group_id            = GROUP_TYPE_ROOT;

    item = addContactToGroup(group_id, a_contact_id);
    if (!item){
        PERROR_OBJ(this,
            "Cannot add new contact with ID: '%d'"
            " to group with ID: '%d'",
            a_contact_id,
            group_id
        );
        goto fail;
    }

out:
    return item;
fail:
    goto out;
}

GroupItemObject * GroupsObject::addContactToRootGroup(
    ContactObject  *a_contact)
{
    GroupItemObject     *group_item = NULL;
    ContactObject::Id   contact_id;

    contact_id  = a_contact->getContactId();
    group_item  = addContactToRootGroup(contact_id);

    return group_item;
}

