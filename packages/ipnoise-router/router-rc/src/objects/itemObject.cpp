/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "libxml2/domElement.h"
#include "objects/contactObject.h"

#include "objects/itemObject.h"

ItemObject * ItemObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ItemObject(a_node, a_doc, a_tagname);
}

ItemObject::ItemObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   ContactAndItem(a_node, a_doc, a_tagname)
{
};

ItemObject::~ItemObject()
{
    string std_out, std_err;
    UserObject  *user       = NULL;
    string      item_huid;
    string      user_huid;

    // get item huid
    item_huid = getHuid();
    if (item_huid.empty()){
        goto out;
    }

    // get user
    user = getOwner();
    assert(user);

    // get user huid
    user_huid = user->getHuid();

    // ok, trying to remove this item
    {
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-item",
            "del",
            user_huid.c_str(),
            item_huid.c_str(),
            NULL
        };
        my_system(argv, std_out, std_err);
    }

out:
    return;
};

void ItemObject::shutdown()
{
    setOnline(false);
}

// called when element expired or deleted
void ItemObject::expired()
{
    removeFromGroups();
}

int32_t ItemObject::autorun()
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

    // create "ipn_sessions" element if not exist
    getCreateSessionsObject();

    // mark us offline
    setOnline(false);

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

SessionsObject * ItemObject::getCreateSessionsObject()
{
    SessionsObject *sessions = NULL;

    sessions = (SessionsObject*)getElementByTagName("ipn_sessions");
    if (!sessions){
        sessions = getDocument()
            ->createElement<SessionsObject>("ipn_sessions");
        appendChildSafe(sessions);
    }

    return sessions;
}

void ItemObject::removeFromGroups()
{
    int i;
    char        tmp[1024]   = { 0x00 };
    UserObject  *user       = NULL;
    vector <DomElement *> res;

    user = getOwner();
    if (!user){
        goto out;
    }

    // search this item in groups
    snprintf(tmp, sizeof(tmp),
        "//ipn_group_item[@huid=\"%s\"]",
        getHuid().c_str()
    );
    res.clear();
    user->getElementsByXpath(tmp, res);
    for (i = 0; i < (int)res.size(); i++){
        GroupItemObject *group_item = NULL;
        group_item = (GroupItemObject *)res[i];
        assert(group_item);
        DomDocument::deleteElement(group_item);
    }

out:
    return;
}

void ItemObject::changed()
{
    string answer;
    string event;

    event += "<event";
    event +=   " type=\"updateItem\"";
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

string ItemObject::getHuid()
{
    return getAttribute("huid");
};

void ItemObject::setHuid(const string &new_huid)
{
    setAttribute("huid", new_huid);
};

bool ItemObject::isSelfItem()
{
    bool        res     = false;
    UserObject  *user   = getOwner();

    if (not user){
        goto out;
    }

    if (getHuid() == user->getHuid()){
        res = true;
    }

out:
    return res;
}

ContactObject * ItemObject::getSelfContact()
{
    ContactObject   *self_contact   = NULL;
    UserObject      *user           = getOwner();

    if (not user){
        goto out;
    }

    self_contact = user->getSelfContact();

out:
    return self_contact;
}

bool ItemObject::isOnline()
{
    return "true" == getAttribute("is_online");
}

string ItemObject::isOnlineStr()
{
    bool is_online = isOnline();
    return is_online ? "true" : "false";
}

void ItemObject::registrySignals()
{
    getDocument()->registrySignal(
        "new_item_was_created",
        "ipn_item"
    );
    getDocument()->registrySignal(
        "item_online",
        "ipn_item"
    );
    getDocument()->registrySignal(
        "item_offline",
        "ipn_item"
    );
    getDocument()->registrySignal(
        "item_online",
        "ipn_item"
    );
    getDocument()->registrySignal(
        "item_offline",
        "ipn_item"
    );
}

void ItemObject::setOnline(bool a_val)
{
    if (isOnline() != a_val){
        setAttributeSafe(
            "is_online",
            a_val ? "true" : "false"
        );
        changed();
        // send signal
        getDocument()->emitSignal(
            a_val ? "item_online" : "item_offline",
            this
        );
    }
}

void ItemObject::requestUserInfo()
{
    string answer;
    string cmd;

    if (isSelfItem()){
        goto out;
    }

    cmd  = "<command";
    cmd +=   " type=\"getUserInfo\"";
    cmd +=   " ver=\"0.01\"";
    cmd += "/>";

    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<commands>";
    answer +=       cmd;
    answer +=   "</commands>";
    answer += "</ipnoise>";

    queue(answer);

out:
    return;
}

PacketObject * ItemObject::queue(const string &a_data)
{
    PacketObject *packet = NULL;
    packet = getCreateSessionsObject()->queue(a_data);
    return packet;
}

PacketObject * ItemObject::queue(DomElement *a_data)
{
    PacketObject *packet = NULL;
    packet = getCreateSessionsObject()->queue(a_data);
    return packet;
}

// ----------------------- API ----------------------- {

string ItemObject::getAPIStatus()
{
    return getAttribute("status");
};

void ItemObject::setAPIStatus(
    const string &a_status)
{
    ContactObject *self_contact = getSelfContact();

    if (getAPIStatus() == a_status){
        // nothing to change
        goto out;
    }

    setAttribute("status", a_status);
    changed();

    // update self contact if available
    if (isSelfItem() && self_contact){
        self_contact->setAPIStatus(a_status);
    }

out:
    return;
}

string ItemObject::getAPINickName()
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

void ItemObject::setAPINickName(
    const string &a_nickname)
{
    DomElement      *element        = NULL;
    ContactObject   *self_contact   = getSelfContact();

    if (getAPINickName() == a_nickname){
        // nothing to change
        goto out;
    }

    element = getElementByTagName("ipn_nickname");
    if (!element){
        element = getDocument()->createElement("ipn_nickname");
        appendChildSafe(element);
    }

    element->setData(a_nickname);
    changed();

    // update self contact if available
    if (isSelfItem() && self_contact){
        self_contact->setAPINickName(a_nickname);
    }

out:
    return;
}

string ItemObject::getAPIIcon()
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

void ItemObject::setAPIIcon(
    const string &a_icon)
{
    DomElement      *element        = NULL;
    ContactObject   *self_contact   = getSelfContact();

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

    // update self contact if available
    if (isSelfItem() && self_contact){
        self_contact->setAPIIcon(a_icon);
    }

out:
    return;
}

// ----------------------- API -----------------------

string ItemObject::getAPIRealNickName()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("ipn_real_nickname");
    if (element){
        ret = element->getData();
    }

    return ret;
};

void ItemObject::setAPIRealNickName(
    const string &a_real_nickname)
{
    DomElement      *element        = NULL;
    ContactObject   *self_contact   = getSelfContact();

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

    // update self contact if available
    if (isSelfItem() && self_contact){
        self_contact->setAPIRealNickName(a_real_nickname);
    }

out:
    return;
}

string ItemObject::getAPIRealIcon()
{
    string      ret         = "";
    DomElement  *element    = NULL;

    element = getElementByTagName("real_icon");
    if (element){
        ret = element->getData();
    }

    return ret;
};

void ItemObject::setAPIRealIcon(
    const string &a_real_icon)
{
    DomElement      *element        = NULL;
    ContactObject   *self_contact   = getSelfContact();

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

    // update self contact if available
    if (isSelfItem() && self_contact){
        self_contact->setAPIRealIcon(a_real_icon);
    }

out:
    return;
}

void ItemObject::getAPIInfo(
    string  &a_out)
{
    a_out += "<item";
    a_out +=   " huid=\""       + getHuid()         + "\"";
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
    a_out += "</item>";
};

// ----------------------- API ----------------------- }

NeighObject * ItemObject::addNeigh(
    const string &a_dev,
    const string &a_lladdr,
    const string &a_metric)
{
    ServerObject    *server  = getServer();
    NeighsObject    *neighs  = server->getCreateNeighsObject();
    NeighObject     *neigh   = NULL;
    string          dst_huid = getHuid();

    neigh = neighs->getNeigh(
        dst_huid,
        a_dev,
        a_lladdr
    );
    if (neigh){
        // already exist
        goto out;
    }

    neigh = neighs->addNeigh(
        dst_huid,
        a_dev,
        a_lladdr,
        a_metric
    );

out:
    return neigh;
}

void ItemObject::getNeighs(
    vector<NeighObject *> &a_neighs)
{
    ServerObject *server = getServer();
    server->getCreateNeighsObject()->getNeighs(
        a_neighs,
        getHuid()
    );
}

void ItemObject::getOnlineNeighs(
    vector<NeighObject *> &a_neighs)
{
    ServerObject *server = getServer();
    server->getCreateNeighsObject()->getOnlineNeighs(
        a_neighs,
        getHuid()
    );
}

