/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/conferenceMsgsObject.h"

ConferenceMsgsObject * ConferenceMsgsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferenceMsgsObject(a_node, a_doc, a_tagname);
}

ConferenceMsgsObject::ConferenceMsgsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ConferenceMsgsObject::~ConferenceMsgsObject()
{
};

int32_t ConferenceMsgsObject::autorun()
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

ConferenceObject * ConferenceMsgsObject::getConferenceObject()
{
    ConferenceObject *conference = NULL;

    conference = (ConferenceObject *)getParentNode();
    assert ("ipn_conference" == conference->getTagName());

    return conference;
}

ConferenceMsgObject * ConferenceMsgsObject::getConferenceMsgById(
    const string    &msg_id)
{
    char tmp[1024] = { 0x00 };
    ConferenceMsgObject *conference_msg = NULL;

    snprintf(tmp, sizeof(tmp), "//ipn_conference_msg[@id=\"%s\"]", msg_id.c_str());
    conference_msg = (ConferenceMsgObject *)getElementByXpath(tmp);

    return conference_msg;
}

ConferenceMsgObject * ConferenceMsgsObject::addMsg(
    const string    &a_msg_type,
    const string    &a_msg_data,
    const string    &a_msg_id,
    const string    &a_src_conf_huid,
    const string    &a_src_conf_id)
{
    UserObject          *owner          = getOwner();
    ConferenceMsgObject *conference_msg = NULL;

    string  msg_type = "incoming";
    string  msg_id;
    string  src_huid = owner->getHuid();

    if (not a_msg_type.empty()){
        msg_type = a_msg_type;
    }
    if (not a_src_conf_huid.empty()){
        src_huid = a_src_conf_huid;
    }

    if (not a_msg_id.empty()){
        msg_id = a_msg_id;
    } else {
        msg_id = getConferenceObject()->createMsgId();
    }

    conference_msg = getDocument()
        ->createElement<ConferenceMsgObject>(
            "ipn_conference_msg"
        );

    conference_msg->setId(msg_id);
    conference_msg->setData(a_msg_data);
    appendChild(conference_msg);

    // TODO inform clients
    {
        SkBuffObject    *skb        = NULL;
        DomElement      *ipnoise    = NULL;
        DomElement      *events     = NULL;
        DomElement      *event      = NULL;
        DomElement      *message    = NULL;
        DomElement      *out_dom    = NULL;

        skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
        skb->setAttributeSafe("to",             owner->getHuid());
        skb->setAttributeSafe("from",           src_huid);
        skb->setAttributeSafe("outdev_name",    "client");

        out_dom = skb->getOutDom();

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        events = skb->getDocument()->createElement("events");
        ipnoise->appendChildSafe(events);

        event = skb->getDocument()->createElement("event");
        events->appendChildSafe(event);

        event->setAttributeSafe("type",     "conferenceMsg");
        event->setAttributeSafe("ver",      "0.01");
        event->setAttributeSafe("conf_id",  getConferenceObject()->getId());

        message = skb->getDocument()->createElement("message");
        event->appendChildSafe(message);

        message->setAttributeSafe("id",                 a_msg_id);
        message->setAttributeSafe("type",               msg_type);
        message->setAttributeSafe("src_conf_huid",      a_src_conf_huid);
        message->setAttributeSafe("src_conf_id",        a_src_conf_id);
        message->appendCDataBlockSafe(a_msg_data);

        getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
    }

    return conference_msg;
}

