/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/conferenceItemsObject.h"

ConferenceItemsObject * ConferenceItemsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferenceItemsObject(a_node, a_doc, a_tagname);
}

ConferenceItemsObject::ConferenceItemsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ConferenceItemsObject::~ConferenceItemsObject()
{
};

int32_t ConferenceItemsObject::autorun()
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

ConferenceObject * ConferenceItemsObject::getConferenceObject()
{
    ConferenceObject *conference = NULL;

    conference = (ConferenceObject *)getParentNode();
    assert ("ipn_conference" == conference->getTagName());

    return conference;
}

ConferenceItemObject * ConferenceItemsObject::getConferenceItemByHuid(
    const string    &huid)
{
    char tmp[1024] = { 0x00 };
    ConferenceItemObject *conference_item = NULL;

    snprintf(tmp, sizeof(tmp), "//ipn_conference_item[@huid=\"%s\"]",
        huid.c_str());

    conference_item = (ConferenceItemObject *)getElementByXpath(tmp);

    return conference_item;
}

/*
 * getCreateConferenceItem - add new item to conference
 *
 * a_huid        - huid for add to conference
 * a_inform_type - clients inform type:
 *                 conferenceJoin|conferenceInvited
 */
ConferenceItemObject * ConferenceItemsObject::getCreateConferenceItem(
    const string    &a_huid,
    const string    &a_inform_type)
{
    ConferenceItemObject *conference_item = NULL;

    string answer;

    // search conference item
    conference_item = getConferenceItemByHuid(a_huid);
    if (!conference_item){
        conference_item = getDocument()
            ->createElement<ConferenceItemObject>(
                "ipn_conference_item"
            );
        appendChild(conference_item);

        // setup huid
        conference_item->setHuid(a_huid);
    }

    if (a_inform_type.size()){
        // inform clients
        SkBuffObject    *skb    = NULL;
        UserObject      *owner  = NULL;

        DomElement *ipnoise     = NULL;
        DomElement *events      = NULL;
        DomElement *event       = NULL;
        DomElement *out_dom     = NULL;

        owner = conference_item->getOwner();

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

        event->setAttributeSafe("type",     a_inform_type);
        event->setAttributeSafe("ver",      "0.01");
        event->setAttributeSafe("conf_id",  conference_item
            ->getConferenceItemsObject()
            ->getConferenceObject()->getId()
        );
        event->setAttributeSafe("huid", conference_item->getHuid());

        getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
    }

    if (not getConferenceObject()->getTopic().size()){
        // conference have not topic, so will be used default topic,
        // we have added new member, so request conference
        // send 'conferenceUpdate' event to all client
        getConferenceObject()->changed();
    }

    return conference_item;
}

void ConferenceItemsObject::getConferenceItems(
    vector <ConferenceItemObject *> &a_out)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    getElementsByTagName("ipn_conference_item", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ConferenceItemObject *conference_item = NULL;
        conference_item = (ConferenceItemObject *)(*res_it);
        a_out.push_back(conference_item);
    }
}

void ConferenceItemsObject::getItems(
    vector <ItemObject *> &a_out)
{
    vector <ConferenceItemObject *>           res;
    vector <ConferenceItemObject *>::iterator res_it;

    getConferenceItems(res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ItemObject           *item            = NULL;
        ConferenceItemObject *conference_item = *res_it;
        item = conference_item->getItem();
        a_out.push_back(item);
    }
}

