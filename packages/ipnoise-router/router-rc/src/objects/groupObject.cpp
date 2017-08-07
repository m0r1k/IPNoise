/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

#include "libxml2/domDocument.h"
#include "objects/groupItemObject.h"
#include "objects/itemObject.h"

#include "objects/groupObject.h"

GroupObject * GroupObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new GroupObject(a_node, a_doc, a_tagname);
}

GroupObject::GroupObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

GroupObject::~GroupObject()
{
};

void GroupObject::expired()
{
    // called when group will deleted from dom
}

bool GroupObject::isSystemGroup()
{
    bool ret        = false;
    int group_id    = getGroupIdInt();

    if (    group_id > 0
        &&  group_id < GroupsObject::GROUP_TYPE_USER_DEFINED)
    {
        ret = true;
    }

    return ret;
}

bool GroupObject::isRootGroup()
{
    bool ret        = false;
    int group_id    = getGroupIdInt();

    if (!group_id){
        ret = true;
    }

    return ret;
}

void GroupObject::setAttribute(const string &name, const string &value)
{
    PERROR_OBJ(this, "Attempt to set attribute: '%s', value: '%s',"
        " use accessors instead\n",
        name.c_str(), value.c_str()
    );
    assert(0);
}

string GroupObject::getAttribute(const string &name)
{
    PERROR_OBJ(this, "Attempt to read attribute: '%s', use accessors instead\n",
        name.c_str());
    assert(0);
}

void GroupObject::setGroupName(const string name)
{
    Object::setAttribute("name", name.c_str());
}

string GroupObject::getGroupName()
{
    return Object::getAttribute("name");
}

void GroupObject::setGroupId(const string id)
{
    setGroupId(atoi(id.c_str()));
}

void GroupObject::setGroupId(int id)
{
    char tmp[1024] = { 0x00 };
    snprintf(tmp, sizeof(tmp), "%x", id);
    Object::setAttribute("id", tmp);
}

string GroupObject::getGroupId()
{
    string ret = "";

    string id = Object::getAttribute("id");
    if (not id.empty()){
        int id_int = 0;
        char buffer[1024] = { 0x00 };
        sscanf(id.c_str(), "%x", &id_int);
        snprintf(buffer, sizeof(buffer), "%d", id_int);
        ret = buffer;
    }

    return ret;
}

int GroupObject::getGroupIdInt()
{
    int ret = 0;
    string id = getGroupId();
    if (id.size()){
        ret = atoi(id.c_str());
    }
    return ret;
}

void GroupObject::getGroupItems(
    vector<GroupItemObject *>   &a_groups_item)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    getElementsByTagName("ipn_group_item", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        GroupItemObject *group_item = (GroupItemObject *)*res_it;
        a_groups_item.push_back(group_item);
    }
}

void GroupObject::getSubGroups(
    vector<GroupObject *>   &a_groups)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    getElementsByTagName("ipn_group", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        GroupObject *group_object = (GroupObject *)*res_it;
        a_groups.push_back(group_object);
    }
}

GroupItemObject * GroupObject::getGroupItem(
    const ContactObject::Id &a_contact_id)
{
    int i = 0;
    GroupItemObject *group_item = NULL;
    vector<DomElement *> res;

    getElementsByTagName("ipn_group_item", res);
    for (i = 0; i < (int)res.size(); i++){
        GroupItemObject *cur_group_item = NULL;
        cur_group_item = (GroupItemObject *)res[i];
        if (cur_group_item->getContactId() == a_contact_id){
            group_item = cur_group_item;
            break;
        }
    }

    return group_item;
}

GroupItemObject * GroupObject::addGroupItem(
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *group_item = NULL;

    // search, may be already exist
    group_item = getGroupItem(a_contact_id);
    if (group_item){
        // already exist
        goto out;
    }

    // create item
    group_item = getDocument()
        ->createElement<GroupItemObject>("ipn_group_item");
    group_item->setContactId(a_contact_id);
    appendChild(group_item);

out:
    return group_item;
}

GroupItemObject * GroupObject::addGroupItem(
    ContactObject *a_contact)
{
    const ContactObject::Id contact_id = a_contact->getContactId();
    return addGroupItem(contact_id);
}

void GroupObject::delGroupItem(
    const ContactObject::Id &a_contact_id)
{
    GroupItemObject *group_item = NULL;

    group_item = getGroupItem(a_contact_id);
    if (group_item){
        DomDocument::deleteElement(group_item);
    }
}

void GroupObject::getAllGroupsItems(
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

