/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/contactItemsObject.h"
#include "objects/contactItemObject.h"

ContactItemObject * ContactItemObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ContactItemObject(a_node, a_doc, a_tagname);
}

ContactItemObject::ContactItemObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ContactItemObject::~ContactItemObject()
{
};

void ContactItemObject::setAttribute(
    const string    &a_name,
    const string    &a_value)
{
    PERROR_OBJ(this, "Attempt to set attribute: '%s', value: '%s',"
        " use accessors instead\n",
        a_name.c_str(), a_value.c_str());
    assert(0);
}

string ContactItemObject::getAttribute(const string &a_name)
{
    PERROR_OBJ(this, "Attempt to read attribute: '%s',"
        " use accessors instead\n",
        a_name.c_str());
    assert(0);
}

void ContactItemObject::setHuid(
    const string &a_huid)
{
    Object::setAttribute("huid", a_huid);
}

string ContactItemObject::getHuid()
{
    return Object::getAttribute("huid");
}

ContactItemsObject * ContactItemObject::getContactItemsObject()
{
    ContactItemsObject *contact_items_obj = NULL;

    contact_items_obj = (ContactItemsObject *)getParentNodeByTagName(
        "ipn_contact_items"
    );

    return contact_items_obj;
}

ItemObject * ContactItemObject::getItem()
{
    ItemObject          *item               = NULL;
    ContactItemsObject  *contact_items_obj  = NULL;
    string              huid                = getHuid();

    contact_items_obj = getContactItemsObject();
    if (not contact_items_obj){
        goto out;
    }

    item = contact_items_obj->getUserObject()->getItem(huid);

out:
    return item;
}


