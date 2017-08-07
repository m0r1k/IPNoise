/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netCommandObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventIPNoiseConferenceJoinSuccess.hpp"

ApiEventIPNoiseConferenceJoinSuccess::ApiEventIPNoiseConferenceJoinSuccess()
    :   ApiEventIPNoise(API_EVENT_IPNOISE_CONFERENCE_JOIN_SUCCESS)
{
}

ApiEventIPNoiseConferenceJoinSuccess::~ApiEventIPNoiseConferenceJoinSuccess()
{
}

int ApiEventIPNoiseConferenceJoinSuccess::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int                     err                 = -1;
    ConferenceObject        *conference         = NULL;
    string                  answer              = "";
    SessionObject           *session            = NULL;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    string                  conf_id             = "";
    string                  conf_pwd1           = "";
    string                  invite_conf_pwd1    = "";
    ConferenceItemObject    *conference_item    = NULL;

    // read attrs
    conf_id             = nc->getAttribute("conf_id");
    conf_pwd1           = nc->getAttribute("conf_pwd1");
    invite_conf_pwd1    = nc->getAttribute("invite_conf_pwd1");

    // search session
    session = nc->getSession(skb);
    if (!session){
        // session not found
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    user = session->getOwner();
    item = session->getItem();

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }
    if (!conference){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=   "<events>";
        answer +=       "<event type=\"conferenceJoinFailed\" ver=\"0.01\"";
        answer +=           " conf_id=\"" + conf_id + "\">";
        answer +=           "<description msg_id=\"conferenceJoinFailed.10\">";
        answer +=               "conference not found";
        answer +=           "</description>";
        answer +=       "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // return events
        item->queue(answer);
        goto out;
    }

    // add|get item to|from conference
    conference_item = conference->getConferenceItemByHuid(item->getHuid());
    if (!conference_item){
        // no such item
        PERROR_OBJ(nc, "no such item: '%s' in conference: '%s'\n",
            item->getHuid().c_str(),
            conference->getId().c_str()
        );
        goto out;
    }

    if (conf_pwd1 != conference_item->getPwd()){
        // passwords not equal
        PERROR_OBJ(nc, "invalid password: '%s'"
            " for item: '%s' in conference: '%s'\n",
            conf_pwd1.c_str(),
            item->getHuid().c_str(),
            conference->getId().c_str()
        );
        goto out;
    }

    // all ok
    conference_item->setSrcPwd(invite_conf_pwd1);

out:
    return err;

}

