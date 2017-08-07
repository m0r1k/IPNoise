/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/groupItemObject.h"
#include "objects/itemObject.h"

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

GroupItemObject * GroupItemObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new GroupItemObject(a_node, a_doc, a_tagname);
}

GroupItemObject::GroupItemObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

GroupItemObject::~GroupItemObject()
{
};

void GroupItemObject::setAttribute(
    const string    &a_name,
    const string    &a_value)
{
    PERROR_OBJ(this, "Attempt to set attribute: '%s', value: '%s',"
        " use accessors instead\n",
        a_name.c_str(), a_value.c_str());
    assert(0);
}

string GroupItemObject::getAttribute(const string &a_name)
{
    PERROR_OBJ(this, "Attempt to read attribute: '%s',"
        " use accessors instead\n",
        a_name.c_str());
    assert(0);
}

void GroupItemObject::setContactId(
    const ContactObject::Id &a_contact_id)
{
    Object::setAttribute("contact_id", a_contact_id);
}

ContactObject::Id GroupItemObject::getContactId()
{
    ContactObject::Id ret;
    ret = (ContactObject::Id)atoi(
        Object::getAttribute("contact_id").c_str()
    );
    return ret;
}

string GroupItemObject::getContactIdStr()
{
    char buffer[128] = { 0x00 };
    ContactObject::Id id = getContactId();
    snprintf(buffer, sizeof(buffer), "%d", id);
    return buffer;
}

