/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "libxml2/domElement.h"
#include "objects/contactItemsObject.h"
#include "objects/contactItemObject.h"

#include "objects/contactObject.h"

ContactObject * ContactObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ContactObject(a_node, a_doc, a_tagname);
}

ContactObject::ContactObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   ContactAndItem(a_node, a_doc, a_tagname)
{
};

ContactObject::~ContactObject()
{
};

void ContactObject::shutdown()
{
    setOnline(false);
}

// called when element expired or deleted
void ContactObject::expired()
{
    removeFromGroups();
}

int32_t ContactObject::autorun()
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

    // create ipn_contact_items object
    getCreateContactItemsObject();

    // update status
    updateOnline();

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

void ContactObject::registrySignals()
{
    getDocument()->registrySignal(
        "new_contact_was_created",
        "ipn_contact"
    );
}

void ContactObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "item_online",
        "ipn_item"
    );
    getDocument()->registrySlot(
        this,
        "item_offline",
        "ipn_item"
    );
}

void ContactObject::removeFromGroups()
{
    char        tmp[1024]   = { 0x00 };
    UserObject  *user       = NULL;

    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    user = getOwner();
    if (!user){
        goto out;
    }

    // search this contact in groups
    snprintf(tmp, sizeof(tmp),
        "//ipn_group_item[@contact_id=\"%s\"]",
        getContactIdStr().c_str()
    );
    res.clear();
    user->getElementsByXpath(tmp, res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        GroupItemObject *group_item = NULL;
        group_item = (GroupItemObject *)*res_it;
        DomDocument::deleteElement(group_item);
    }

out:
    return;
}

void ContactObject::changed()
{
    string answer;
    string event;

    event += "<event";
    event +=   " type=\"updateContact\"";
    event +=   " ver=\"0.01\"";
    event += ">";
    getAPIInfo(event);
    event += "</event>";

    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       event;
    answer +=   "</events>";
    answer += "</ipnoise>";

    // send event to all clients
    getOwner()->queue(answer);
}

ContactObject::Id ContactObject::getContactId()
{
    ContactObject::Id   contact_id;
    string              contact_id_str;

    contact_id_str  = getAttribute("contact_id");
    contact_id      = (ContactObject::Id)(
        atoi(contact_id_str.c_str())
    );

    return contact_id;
};

string ContactObject::getContactIdStr()
{
    char buffer[128] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", getContactId());
    return buffer;
}

void ContactObject::setContactId(
    const ContactObject::Id &a_contact_id)
{
    char buffer[128] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_contact_id);
    setAttribute("contact_id", buffer);
};

bool ContactObject::isSystem()
{
    bool                res         = false;
    ContactObject::Id   contact_id  = getContactId();
    if (ContactObject::CONTACT_TYPE_USER_DEFINED <= contact_id){
        res = true;
    }
    return res;
}

ContactItemsObject * ContactObject::getContactItemsObject()
{
    ContactItemsObject *items_obj = NULL;
    items_obj = (ContactItemsObject *)getElementByTagName(
        "ipn_contact_items"
    );
    return items_obj;
}

ContactItemsObject * ContactObject::getCreateContactItemsObject()
{
    ContactItemsObject *items_obj = NULL;

    items_obj = getContactItemsObject();
    if (items_obj){
        // already exist
        goto out;
    }

    // create new
    items_obj = getDocument()->createElement<ContactItemsObject>(
        "ipn_contact_items"
    );
    appendChildSafe(items_obj);

out:
    return items_obj;
}

ContactItemObject * ContactObject::getContactItem(
    const string &a_huid)
{
    ContactItemsObject *items_obj = NULL;
    items_obj = getCreateContactItemsObject();
    return items_obj->getContactItem(a_huid);
}

ContactItemObject * ContactObject::getCreateContactItem(
    const string &a_huid)
{
    ContactItemsObject  *contact_items_obj  = NULL;
    ContactItemObject   *contact_item       = NULL;

    contact_items_obj = getCreateContactItemsObject();
    contact_item = contact_items_obj->getCreateContactItem(a_huid);
    if (not contact_item){
        PERROR("Cannot create contact item\n");
        goto fail;
    }

    // all ok

out:
    return contact_item;

fail:
    goto out;
}

ItemObject * ContactObject::getCreateItem(
    const string &a_huid)
{
    ItemObject *item = NULL;
    if (not a_huid.size()){
        goto out;
    }

    item = getOwner()->getCreateItem(a_huid);
    getCreateContactItem(a_huid);

out:
    return item;
}

void ContactObject::delContactItem(
    const string &a_huid)
{
    ContactItemsObject *items_obj = NULL;
    items_obj = getCreateContactItemsObject();
    items_obj->delContactItem(a_huid);
}

void ContactObject::getItems(
    map<string, ItemObject *>    &a_out)
{
    ContactItemsObject *items_obj = NULL;
    items_obj = getCreateContactItemsObject();
    items_obj->getItems(a_out);
}

void ContactObject::getContactItems(
    map<string, ContactItemObject *>    &a_out)
{
    ContactItemsObject *items_obj = NULL;
    items_obj = getCreateContactItemsObject();
    items_obj->getContactItems(a_out);
}

ItemObject * ContactObject::getSelfItem()
{
    ItemObject  *item = NULL;
    UserObject  *user = getOwner();

    map<string, ItemObject *>            res;
    map<string, ItemObject *>::iterator  res_it;

    if (not user){
        goto out;
    }

    getItems(res);
    res_it = res.find(user->getHuid());
    if (res.end() != res_it){
        item = res_it->second;
    }

out:
    return item;
}

void ContactObject::updateOnline()
{
    bool new_state = false;
    map<string, ItemObject *>            items;
    map<string, ItemObject *>::iterator  items_it;

    getItems(items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        if (items_it->second->isOnline()){
            new_state = true;
            break;
        }
    }

    setOnline(new_state);
}

void ContactObject::setOnline(bool a_val)
{
    if (isOnline() != a_val){
        setAttributeSafe(
            "is_online",
            a_val ? "true" : "false"
        );
        changed();
    }
}

bool ContactObject::isOnline()
{
    return "true" == getAttribute("is_online");
}

string ContactObject::isOnlineStr()
{
    return isOnline() ? "true" : "false";
}

void ContactObject::slot(
    const string    &a_signal_name,
    DomElement      *a_object)
{
    string signal_name = a_signal_name;
    string object_type = "";

    object_type = a_object->getTagName();

    do {
        map<string, ItemObject *>             items;
        map<string, ItemObject *>::iterator   items_it;

        bool        is_online   = false;
        ItemObject  *item       = NULL;
        string      huid;

        if (    "item_online"  != signal_name
            &&  "item_offline" != signal_name)
        {
            break;
        }
        if ("ipn_item" != object_type){
            break;
        }
        item = (ItemObject *)a_object;

        // get our items
        getItems(items);
        items_it = items.find(item->getHuid());
        if (items.end() == items_it){
            // this contact cannot contain such item, skip
            break;
        }
        // check items state
        for (items_it = items.begin();
            items_it != items.end();
            items_it++)
        {
            if (items_it->second->isOnline()){
                // we have online item
                is_online = true;
                break;
            }
        }

        setOnline(is_online);
    } while (0);
}

// ----------------------- API ----------------------- {

string ContactObject::getAPIStatus()
{
    return getAttribute("status");
};

void ContactObject::setAPIStatus(
    const string &a_status)
{
    ItemObject *item = getSelfItem();

    if (getAPIStatus() == a_status){
        // nothing to change
        goto out;
    }

    setAttribute("status", a_status);
    changed();

    // update self item if exist
    if (item){
        item->setAPIStatus(a_status);
    }

out:
    return;
}

string ContactObject::getAPINickName()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("ipn_nickname");
    if (element){
        ret = element->getData();
    } else {
        // give a chance not be empty
        ret = getAPIRealNickName();
    }

    return ret;
};

void ContactObject::setAPINickName(
    const string &a_nickname)
{
    DomElement  *element    = NULL;
    ItemObject  *item       = getSelfItem();

    if (getAPINickName() == a_nickname){
        // nothing to change
        goto out;
    }

    element = getElementByTagName("ipn_nickname");
    if (not element){
        element = getDocument()->createElement("ipn_nickname");
        appendChildSafe(element);
    }

    element->setData(a_nickname);
    changed();

    // update self item if exist
    if (item){
        item->setAPINickName(a_nickname);
    }

out:
    return;
}

string ContactObject::getAPIIcon()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("ipn_icon");
    if (element){
        ret = element->getData();
    } else {
        ret = getAPIRealIcon();
    }

    return ret;
};

void ContactObject::setAPIIcon(
    const string &a_icon)
{
    DomElement  *element    = NULL;
    ItemObject  *item       = getSelfItem();

    if (getAPIIcon() == a_icon){
        // nothing to change
        goto out;
    }

    element = getElementByTagName("ipn_icon");
    if (!element){
        element = getDocument()->createElement("ipn_icon");
        appendChildSafe(element);
    }

    element->setData(a_icon);
    changed();

    // update self item if exist
    if (item){
        item->setAPIIcon(a_icon);
    }

out:
    return;
}

// ----------------------- API -----------------------

string ContactObject::getAPIRealNickName()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("ipn_real_nickname");
    if (element){
        ret = element->getData();
    }

    return ret;
};

void ContactObject::setAPIRealNickName(
    const string &a_real_nickname)
{
    DomElement  *element    = NULL;
    ItemObject  *item       = getSelfItem();

    if (getAPIRealNickName() == a_real_nickname){
        // nothing to change
        goto out;
    }

    element = getElementByTagName("ipn_real_nickname");
    if (!element){
        element = getDocument()->createElement("ipn_real_nickname");
        appendChildSafe(element);
    }

    element->setData(a_real_nickname);
    changed();

    // update self item if exist
    if (item){
        item->setAPIRealNickName(a_real_nickname);
    }

out:
    return;
}

string ContactObject::getAPIRealIcon()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("real_icon");
    if (element){
        ret = element->getData();
    }

    return ret;
};

void ContactObject::setAPIRealIcon(
    const string &a_real_icon)
{
    DomElement  *element    = NULL;
    ItemObject  *item       = getSelfItem();

    if (getAPIRealIcon() == a_real_icon){
        // nothing to change
        goto out;
    }

    element = getElementByTagName("real_icon");
    if (!element){
        element = getDocument()->createElement("real_icon");
        appendChildSafe(element);
    }

    element->setData(a_real_icon);
    changed();

    // update self item if exist
    if (item){
        item->setAPIRealIcon(a_real_icon);
    }

out:
    return;
}

void ContactObject::getAPIInfo(
    string  &a_out)
{
    map<string, ContactItemObject *>            res;
    map<string, ContactItemObject *>::iterator  res_it;

    getContactItems(res);

    a_out += "<contact";
    a_out +=   " contact_id=\"" + getContactIdStr() + "\"";
    a_out +=   " is_online=\""  + isOnlineStr()     + "\"";
    a_out += ">";
    a_out +=   "<status><![CDATA[";
    a_out +=       getAPIStatus();
    a_out +=   "]]></status>";
    a_out +=   "<nickname><![CDATA[";
    a_out +=       getAPINickName();
    a_out +=   "]]></nickname>";
    a_out +=   "<icon><![CDATA[";
    a_out +=       getAPIIcon();
    a_out +=   "]]></icon>";
    a_out +=   "<real_nickname><![CDATA[";
    a_out +=       getAPIRealNickName();
    a_out +=   "]]></real_nickname>";
    a_out +=   "<real_icon><![CDATA[";
    a_out +=       getAPIRealIcon();
    a_out +=   "]]></real_icon>";

    // add contact items
    a_out += "<contact_items>";
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ContactItemObject *contact_item = res_it->second;
        a_out += "<contact_item";
        a_out +=   " huid=\"" + contact_item->getHuid() + "\"";
        a_out += "/>";
    }
    a_out += "</contact_items>";

    a_out += "</contact>";
};

// ----------------------- API ----------------------- }


