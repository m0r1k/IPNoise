/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/conferenceObject.h"

ConferenceObject * ConferenceObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferenceObject(a_node, a_doc, a_tagname);
}

ConferenceObject::ConferenceObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    string pwd0 = getPwd0();
    if (not pwd0.size()){
        pwd0 = createPwd();
        setAttributeSafe("conf_pwd0", pwd0);
    }
};

ConferenceObject::~ConferenceObject()
{
};

int32_t ConferenceObject::autorun()
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

    // create conferenceItems element
    getCreateConferenceItemsObject();

    // create conferenceMsgs element
    getConferenceMsgsObject();


out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void ConferenceObject::registrySignals()
{
    getDocument()->registrySignal(
        "conferenceJoin",
        "ipn_conference_item"
    );
    getDocument()->registrySignal(
        "conferenceCreated",
        "ipn_conference"
    );
}

void ConferenceObject::shutdown()
{
    ConferenceMsgsObject *conference_msgs = NULL;
    conference_msgs = getConferenceMsgsObject();
    if (conference_msgs){
        vector<DomElement *> res;
        conference_msgs->getChilds(res);
        if (res.empty()){
            // conference have not messages, do remove
            setExpired("0");
        }
    }
}

ConferenceItemObject * ConferenceObject::getConferenceItemByHuid(
    const string    &huid)
{
    ConferenceItemObject *conference_item = NULL;

    conference_item = getCreateConferenceItemsObject()
        ->getConferenceItemByHuid(
            huid
        );

    return conference_item;
}

ConferenceItemObject * ConferenceObject::getCreateConferenceItem(
    const string    &a_huid,
    const string    &a_inform_type)
{
    ConferenceItemObject    *item = NULL;
    ConferenceItemsObject   *conference_items = NULL;

    conference_items = getCreateConferenceItemsObject();
    item = conference_items->getCreateConferenceItem(
        a_huid,
        a_inform_type
    );
    return item;
}

ConferenceItemsObject * ConferenceObject::getCreateConferenceItemsObject()
{
    ConferenceItemsObject *items = NULL;

    items = (ConferenceItemsObject *)getElementByTagName(
        "ipn_conference_items");
    if (!items){
        items = getDocument()
            ->createElement<ConferenceItemsObject>(
                "ipn_conference_items"
            );
        appendChildSafe(items);
    }

    return items;
}

ConferenceMsgsObject * ConferenceObject::getConferenceMsgsObject()
{
    ConferenceMsgsObject *msgs = NULL;

    msgs = (ConferenceMsgsObject *)getElementByTagName("ipn_conference_msgs");
    if (!msgs){
        msgs = getDocument()
            ->createElement<ConferenceMsgsObject>("ipn_conference_msgs");
        appendChildSafe(msgs);
    }

    return msgs;
}

ConferenceMsgObject * ConferenceObject::addMsg(
    const string    &a_type,
    const string    &a_msg_data,
    const string    &a_msg_id,
    const string    &a_src_conf_huid,
    const string    &a_src_conf_id)
{
    ConferenceMsgObject  *conference_msg  = NULL;
    ConferenceMsgsObject *conference_msgs = NULL;

    conference_msgs = getConferenceMsgsObject();
    conference_msg  = conference_msgs->addMsg(
        a_type,
        a_msg_data,
        a_msg_id,
        a_src_conf_huid,
        a_src_conf_id
    );
    return conference_msg;
}

ConferencesObject * ConferenceObject::getConferencesObject()
{
    ConferencesObject *conferences = NULL;

    conferences = (ConferencesObject *)getParentNode();
    assert ("ipn_conferences" == conferences->getTagName());

    return conferences;
}

string ConferenceObject::createPwd()
{
    return getRand(20);
}

string ConferenceObject::createMsgId()
{
    return getRand(10);
}

void ConferenceObject::getConferenceItems(
    vector <ConferenceItemObject *> &a_out)
{
    ConferenceItemsObject *conference_items_object = NULL;
    conference_items_object = getCreateConferenceItemsObject();
    conference_items_object->getConferenceItems(a_out);
}

// ---------------------------------------------------------

void ConferenceObject::setTopic(const string &a_conf_topic)
{
    if (getTopic() == a_conf_topic){
        // nothing changed
        goto out;
    }
    setAttribute("topic", a_conf_topic);
    changed();
out:
    return;
}

string ConferenceObject::getDefaultTopic()
{
    string      topic;
    ItemObject  *self_item = NULL;

    vector <ItemObject *>           items;
    vector <ItemObject *>::iterator items_it;

    getCreateConferenceItemsObject()->getItems(items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        ItemObject *item = *items_it;
        if (item->isSelfItem()){
            self_item = item;
            continue;
        }
        if (topic.size()){
            topic += ",";
        }
        topic += item->getAPINickName();
    }
    if (not topic.size() && self_item){
        topic = self_item->getAPINickName();
    }
    return topic;
}

string ConferenceObject::getTopic()
{
    return getAttribute("topic");
}

string ConferenceObject::getAnyTopic()
{
    string topic = getTopic();
    if (not topic.size()){
        topic = getDefaultTopic();
    }
    return topic;
}

void ConferenceObject::setId(const string &a_conf_id)
{
    if (getId() == a_conf_id){
        // nothing changed
        goto out;
    }
    setAttribute("id", a_conf_id);
    changed();
out:
    return;
}

string ConferenceObject::getId()
{
    return getAttribute("id");
}

void ConferenceObject::setFlags(const string &a_conf_flags)
{
    if (getFlags() == a_conf_flags){
        // nothing changed
        goto out;
    }
    setAttribute("flags", a_conf_flags);
out:
    return;
}

string ConferenceObject::getFlags()
{
    return getAttribute("flags");
}

void ConferenceObject::setPwd0(const string &a_conf_pwd0)
{
    if (getPwd0() == a_conf_pwd0){
        // nothing changed
        goto out;
    }
    setAttribute("conf_pwd0", a_conf_pwd0);
out:
    return;
}

string ConferenceObject::getPwd0()
{
    return getAttribute("conf_pwd0");
}

void ConferenceObject::changed()
{
    // inform clients
    SkBuffObject    *skb    = NULL;
    UserObject      *owner  = NULL;

    DomElement *ipnoise     = NULL;
    DomElement *events      = NULL;
    DomElement *event       = NULL;
    DomElement *out_dom     = NULL;

    owner = getOwner();

    skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
    skb->setAttributeSafe("to",             owner->getHuid());
    skb->setAttributeSafe("from",           owner->getHuid());
    skb->setAttributeSafe("outdev_name",    "client");

    out_dom = skb->getOutDom();

    ipnoise = skb->getDocument()->createElement("ipnoise");
    out_dom->appendChildSafe(ipnoise);

    events = skb->getDocument()->createElement("events");
    ipnoise->appendChildSafe(events);

    event = skb->getDocument()->createElement("event");
    events->appendChildSafe(event);

    event->setAttributeSafe("type",         "updateConference");
    event->setAttributeSafe("ver",          "0.01");
    event->setAttributeSafe("conf_id",      getId());
    event->setAttributeSafe("conf_topic",   getAnyTopic());
    event->setAttributeSafe("conf_flags",   getFlags());

    getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
}

// ---------------------------------------------------------

