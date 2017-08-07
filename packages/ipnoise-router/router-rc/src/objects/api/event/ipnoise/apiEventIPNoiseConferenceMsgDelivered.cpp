/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netEventObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventIPNoiseConferenceMsgDelivered.hpp"

ApiEventIPNoiseConferenceMsgDelivered::ApiEventIPNoiseConferenceMsgDelivered()
    :   ApiEventIPNoise(API_EVENT_IPNOISE_CONFERENCE_MSG_DELIVERED)
{
}

ApiEventIPNoiseConferenceMsgDelivered::~ApiEventIPNoiseConferenceMsgDelivered()
{
}

int ApiEventIPNoiseConferenceMsgDelivered::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int err = 0;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    SessionObject           *session            = NULL;
    ConferenceObject        *conference         = NULL;
    string                  sessid              = "";
    string                  conf_id             = "";
    string                  msg_id              = "";
    string                  huid                = "";
    string                  answer              = "";

    vector <DomElement *>           msgs;
    vector <DomElement *>::iterator msgs_it;

    vector <ConferenceItemObject *>             conference_items;
    vector <ConferenceItemObject *>::iterator   conference_items_it;

    // search session
    session = nc->getSession(skb);

    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session ID
    sessid = session->getSessId();

    // search user for this session
    user = session->getOwner();
    if (!user){
        err = nc->errorNotLogged(skb);
        goto out;
    }

    // search item for this session
    item = session->getItem();
    if (!item){
        err = nc->errorInternalError(skb);
        goto out;
    }

    // read event attributes
    conf_id = nc->getAttribute("conf_id");
    msg_id  = nc->getAttribute("msg_id");

    PDEBUG_OBJ(nc, 5, "ApiEventIPNoiseConferenceMsgDelivered::process\n"
        " conf_id '%s', msg_id: '%s'\n",
        conf_id.c_str(),
        msg_id.c_str()
    );

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }

    if (!conference){
        goto out;
    }

    // inform clients
    {
        SkBuffObject    *skb    = NULL;
        UserObject      *owner  = NULL;

        DomElement *ipnoise     = NULL;
        DomElement *events      = NULL;
        DomElement *event       = NULL;
        DomElement *out_dom     = NULL;

        owner = conference->getOwner();

        skb = nc->getDocument()->createElement<SkBuffObject>("ipn_skbuff");
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

        event->setAttributeSafe("ver",          "0.01");
        event->setAttributeSafe("type",         "conferenceMsgDelivered");
        event->setAttributeSafe("conf_id",      conf_id);
        event->setAttributeSafe("msg_id",       msg_id);

        nc->getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
    }

    // all ok
    err = 0;

out:
    return err;
}

