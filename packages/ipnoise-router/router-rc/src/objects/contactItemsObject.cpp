/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/contactObject.h"
#include "objects/contactItemObject.h"

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

#include "objects/contactItemsObject.h"

ContactItemsObject * ContactItemsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ContactItemsObject(a_node, a_doc, a_tagname);
}

ContactItemsObject::ContactItemsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ContactItemsObject::~ContactItemsObject()
{
};

int32_t ContactItemsObject::autorun()
{
    int err         = 0;
    int priveleges  = 0;

    priveleges = getDocument()->getPriveleges();

    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void ContactItemsObject::setAttribute(
    const string    &a_name,
    const string    &a_value)
{
    PERROR_OBJ(this, "Attempt to set attribute: '%s', value: '%s',"
        "use accessors instead\n",
        a_name.c_str(), a_value.c_str()
    );
    assert(0);
}

string ContactItemsObject::getAttribute(
    const string &a_name)
{
    PERROR_OBJ(this, "Attempt to read attribute: '%s',"
        " use accessors instead\n",
        a_name.c_str());
    assert(0);
}

ContactObject * ContactItemsObject::getContactObject()
{
    ContactObject *contact = NULL;

    contact = (ContactObject *)getParentNode();
    if (not contact){
        goto out;
    }
    assert ("ipn_contact" == contact->getTagName());

out:
    return contact;
}

UserObject * ContactItemsObject::getUserObject()
{
    UserObject      *user       = NULL;
    ContactObject   *contact    = NULL;

    contact = getContactObject();
    if (not contact){
        goto out;
    }

    user = contact->getOwner();

out:
    return user;
}

ContactItemObject * ContactItemsObject::getContactItem(
    const string    &a_huid)
{
    char                buffer[512]     = { 0x00 };
    ContactItemObject   *contact_item   = NULL;

    snprintf(buffer, sizeof(buffer),
        "//ipn_contact_item[@huid=\"%s\"]",
        a_huid.c_str()
    );
    contact_item = (ContactItemObject *)getElementByXpath(buffer);
    return contact_item;
}

ContactItemObject * ContactItemsObject::getCreateContactItem(
    const string    &a_huid)
{
    ContactItemObject *contact_item = NULL;

    contact_item = getContactItem(a_huid);
    if (contact_item){
        // already exist
        goto out;
    }

    contact_item = getDocument()->createElement<ContactItemObject>(
        "ipn_contact_item"
    );
    if (not contact_item){
        PWARN_OBJ(this, "Cannot create ipn_contact_item element\n");
        goto fail;
    }
    appendChild(contact_item);

    // do set huid after add to DOM
    contact_item->setHuid(a_huid);

out:
    return contact_item;
fail:
    goto out;
}

void ContactItemsObject::delContactItem(
    const string &a_huid)
{
    ContactItemObject *contact_item = NULL;
    if (a_huid.empty()){
        goto out;
    }
    contact_item = getContactItem(a_huid);
    if (not contact_item){
        goto out;
    }

    DomDocument::deleteElement(contact_item);

out:
    return;
}

void ContactItemsObject::getContactItems(
    map<string, ContactItemObject *>    &a_out)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    getElementsByTagName("ipn_contact_item", res);

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ContactItemObject *contact_item = NULL;
        contact_item = (ContactItemObject *)(*res_it);
        a_out[contact_item->getHuid()] = contact_item;
    }
}

void ContactItemsObject::getItems(
    map<string, ItemObject *>    &a_out)
{
    map<string, ContactItemObject *>               res;
    map<string, ContactItemObject *>::iterator     res_it;

    getContactItems(res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ContactItemObject *contact_item = res_it->second;
        ItemObject        *item         = contact_item->getItem();
        if (item){
            a_out[item->getHuid()] = item;
        }
    }
}


