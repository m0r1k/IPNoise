/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/itemsObject.h"

ItemsObject * ItemsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ItemsObject(a_node, a_doc, a_tagname);
}

ItemsObject::ItemsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ItemsObject::~ItemsObject()
{
};

ItemObject * ItemsObject::getItem(const string &huid)
{
    char tmp[1024] = { 0x00 };
    ItemObject *item = NULL;

    // search item
    snprintf(tmp, sizeof(tmp),
        "//ipn_item[@huid=\"%s\"]",
        huid.c_str()
    );

    item = (ItemObject *)getElementByXpath(tmp);

    return item;
}

void ItemsObject::getItems(vector<ItemObject *> &a_res)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_i;

    getElementsByTagName("ipn_item", res);
    for (res_i = res.begin();
        res_i != res.end();
        res_i++)
    {
        ItemObject *item = (ItemObject *)*res_i;
        a_res.push_back(item);
    }
}

ItemObject * ItemsObject::getCreateItem(
    const string &a_huid)
{
    ItemObject *item = NULL;

    if (a_huid.empty()){
        PERROR_OBJ(this,
            "attempt to create item with empty a_huid\n");
        goto out;
    }

    item = getItem(a_huid);
    if (item){
        // already exist
        goto out;
    }

    // create item
    item = getDocument()->createElement<ItemObject>("ipn_item");
    if (!item){
        PERROR_OBJ(this, "Cannot create 'ipn_item' element\n");
        goto fail;
    }
    appendChild(item);

    // do setHuid after item will be attached in DOM
    item->setHuid(a_huid);

    // inform all what new item was created
    getDocument()->emitSignal(
        "new_item_was_created",
        (DomElement *)item
    );

out:
    return item;

fail:
    goto out;
}

void ItemsObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "session_opened",
        "ipn_session_item"
    );
    getDocument()->registrySlot(
        this,
        "session_closed",
        "ipn_session_item"
    );
}

void ItemsObject::slot(
    const string    &a_signal_name,
    DomElement      *a_object)
{
    string signal_name = a_signal_name;
    string object_type = "";

    object_type = a_object->getTagName();

    do {
        vector<SessionObject *>             sessions;
        vector<SessionObject *>::iterator   sessions_it;

        SessionObject   *session    = NULL;
        ItemObject      *item       = NULL;
        string           huid;

        if (    "session_opened" != signal_name
            &&  "session_closed" != signal_name)
        {
            break;
        }
        if (    "ipn_session_item"      != object_type
            &&  "ipn_session_client"    != object_type)
        {
            break;
        }
        session = (SessionObject *)a_object;
        item    = session->getItem();
        if (not item){
            break;
        }
        // collect online sessions
        item->getCreateSessionsObject()->getOnlineSessions(
            sessions
        );
        item->setOnline(sessions.size());
    } while (0);
}

