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
#include "objects/conferencesObject.h"

ConferencesObject * ConferencesObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferencesObject(a_node, a_doc, a_tagname);
}

ConferencesObject::ConferencesObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ConferencesObject::~ConferencesObject()
{
};

int32_t ConferencesObject::autorun()
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

UserObject * ConferencesObject::getUserObject()
{
    UserObject *user = NULL;

    user = (UserObject *)getParentNode();
    assert ("ipn_user" == user->getTagName());

    return user;
}

ConferenceObject * ConferencesObject::getConferenceById(
    const string    &conf_id)
{
    char tmp[1024] = { 0x00 };
    ConferenceObject *conference = NULL;

    snprintf(tmp, sizeof(tmp),
        "//ipn_conference[@id=\"%s\"]",
        conf_id.c_str());
    conference = (ConferenceObject *)getElementByXpath(tmp);

    return conference;
}

string ConferencesObject::createConferenceId()
{
    ConferenceObject    *conference = NULL;
    string              conf_id;

again:

    conf_id     = getRand(20);
    conference  = getConferenceById(conf_id);
    if (conference){
        // hmm.. name already exist, go again
        goto again;
    }

    return conf_id;
}

ConferenceObject * ConferencesObject::createConference(
    const string    &a_conf_flags,
    const string    &a_conf_topic)
{
    ConferenceObject *conference = NULL;
    conference = createConferenceById(
        "",
        a_conf_flags,
        a_conf_topic
    );
    return conference;
}

ConferenceObject * ConferencesObject::createConferenceById(
    const string    &a_conf_id,
    const string    &a_conf_flags,
    const string    &a_conf_topic)
{
    string              conf_id         = a_conf_id;
    ConferenceObject    *conference     = NULL;
    UserObject          *owner          = NULL;

    if (not conf_id.empty()){
        // search conference
        conference = getConferenceById(conf_id);
    } else {
        // create new conference ID
        conf_id = createConferenceId();
    }

    if (conference){
        goto out;
    }

    // search owner
    owner = getOwner();

    // create conference
    conference = getDocument()
        ->createElement<ConferenceObject>("ipn_conference");
    appendChild(conference);
    // set all props after element will be attached in DOM
    conference->setId(conf_id);
    conference->setTopic(a_conf_topic);
    conference->setFlags(a_conf_flags);

    // emit signal
    getDocument()->emitSignal("conferenceCreated", conference);

    {
        SkBuffObject    *skb        = NULL;
        DomElement      *ipnoise    = NULL;
        DomElement      *events     = NULL;
        DomElement      *event      = NULL;
        DomElement      *out_dom    = NULL;

        skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
        skb->setAttributeSafe("to",             owner->getHuid());
        skb->setAttributeSafe("from",           owner->getHuid());
        skb->setAttributeSafe("outdev_name",    "client");

        out_dom = skb->getOutDom(1);

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        events = skb->getDocument()->createElement("events");
        ipnoise->appendChildSafe(events);

        event = skb->getDocument()->createElement("event");
        events->appendChildSafe(event);

        event->setAttributeSafe("type",         "conferenceCreated");
        event->setAttributeSafe("ver",          "0.01");
        event->setAttributeSafe("conf_id",      conference->getId());
        event->setAttributeSafe("conf_topic",   conference->getAnyTopic());
        event->setAttributeSafe("conf_flags",   conference->getFlags());

        getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
    }

    // join ourself with conference
    conference->getCreateConferenceItem(
        owner->getHuid(),
        "conferenceJoin"
    );

out:
    return conference;
}

