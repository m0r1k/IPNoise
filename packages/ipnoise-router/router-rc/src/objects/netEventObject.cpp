/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/linkObject.h"

// link events
#include "objects/api/event/link/apiEventLinkSearchRouteAnswer.hpp"
#include "objects/api/event/link/apiEventLinkSearchUsersAnswer.hpp"

// ipnoise events
#include "objects/api/event/ipnoise/apiEventIPNoiseConferenceInviteSuccess.hpp"
#include "objects/api/event/ipnoise/apiEventIPNoiseConferenceInvited.hpp"
#include "objects/api/event/ipnoise/apiEventIPNoiseConferenceJoinSuccess.hpp"
#include "objects/api/event/ipnoise/apiEventIPNoiseUpdateUser.hpp"
#include "objects/api/event/ipnoise/apiEventIPNoiseConferenceMsgDelivered.hpp"

#include "objects/netEventObject.h"

static NetEvents *s_events_client  = NULL;
static NetEvents *s_events_link    = NULL;
static NetEvents *s_events_ipnoise = NULL;

NetEventObject * NetEventObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NetEventObject(a_node, a_doc, a_tagname);
}

NetEventObject::NetEventObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   NetCommandAndEventObject(a_node, a_doc, a_tagname)
{
    // client events
    if (!s_events_client){
        s_events_client = new NetEvents();
        registerEventsClient();
    }

    // link events
    if (!s_events_link){
        s_events_link = new NetEvents();
        registerEventsLink();
    }

    // IPNoise events
    if (!s_events_ipnoise){
        s_events_ipnoise = new NetEvents();
        registerEventsIPNoise();
    }
};

NetEventObject::~NetEventObject()
{
    if (s_events_client){
        free(s_events_client);
    }
    if (s_events_link){
        free(s_events_link);
    }
    if (s_events_ipnoise){
        free(s_events_ipnoise);
    }
};

void NetEventObject::registerEventsClient()
{
}

void NetEventObject::registerEventsLink()
{
    ApiEvent *event = NULL;

    // searchRouteAnswer
    {
        event = new ApiEventLinkSearchRouteAnswer();
        (*s_events_link)[event->getName()] = event;
    }

    // searchUsersAnswer
    {
        event = new ApiEventLinkSearchUsersAnswer();
        (*s_events_link)[event->getName()] = event;
    }
}

void NetEventObject::registerEventsIPNoise()
{
    ApiEvent *event = NULL;

    // conferenceInviteSuccess
    {
        event = new ApiEventIPNoiseConferenceInviteSuccess();
        (*s_events_ipnoise)[event->getName()] = event;
    }

    // conferenceInvited
    {
        event = new ApiEventIPNoiseConferenceInvited();
        (*s_events_ipnoise)[event->getName()] = event;
    }

    // conferenceJoinSuccess
    {
        event = new ApiEventIPNoiseConferenceJoinSuccess();
        (*s_events_ipnoise)[event->getName()] = event;
    }

    // updateUser
    {
        event = new ApiEventIPNoiseUpdateUser();
        (*s_events_ipnoise)[event->getName()] = event;
    }

    // conferenceMsgDelivered
    {
        event = new ApiEventIPNoiseConferenceMsgDelivered();
        (*s_events_ipnoise)[event->getName()] = event;
    }
}

int NetEventObject::processEvent(SkBuffObject *skb)
{
    int err = -1;
    ApiEvent        *event = NULL;
    string          event_type = getAttribute("type");
    NetEventsIt     events_i;
    string          indev;

    PDEBUG_OBJ(this, 5, "NetEventObject::processEvent"
        " event_type: '%s', indev_name: '%s'\n",
        event_type.c_str(),
        skb->getAttribute("indev_name").c_str()
    );

    if (!event_type.size()){
        PERROR_OBJ(this, "Event without type was received\n");
        goto out;
    }

    // search handler
    indev = skb->getAttribute("indev_name");
    if (indev == CLIENTS_DEV_NAME){
        events_i = s_events_client->find(event_type);
        if (events_i != s_events_client->end()){
            event = events_i->second;
        }
    } else if ( (indev == IPNOISE_DEV_NAME)
        ||      (indev == LO_DEV_NAME))
    {
        events_i = s_events_ipnoise->find(event_type);
        if (events_i != s_events_ipnoise->end()){
            event = events_i->second;
        }
    } else {
        // may be link event?
        events_i = s_events_link->find(event_type);
        if (events_i != s_events_link->end()){
            event = events_i->second;
        }
    }

    if (!event){
        // handler not found
        PERROR_OBJ(this, "Unsupported event: '%s' was received from dev: '%s'\n",
            event_type.c_str(),
            indev.c_str()
        );
        goto out;
    }

    // call handler
    err = event->process(this, skb);

out:
    return err;
}

