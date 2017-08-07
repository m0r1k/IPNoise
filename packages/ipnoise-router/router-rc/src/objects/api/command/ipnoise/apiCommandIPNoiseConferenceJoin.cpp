/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandIPNoiseConferenceJoin.hpp"

ApiCommandIPNoiseConferenceJoin::ApiCommandIPNoiseConferenceJoin()
    :   ApiCommandIPNoise(API_COMMAND_IPNOISE_CONFERENCE_JOIN)
{
}

ApiCommandIPNoiseConferenceJoin::~ApiCommandIPNoiseConferenceJoin()
{
}

int ApiCommandIPNoiseConferenceJoin::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int                     err                 = -1;
    ConferenceObject        *conference         = NULL;
    string                  answer              = "";
    SessionObject           *session            = NULL;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    string                  conf_id             = "";
    string                  conf_pwd0           = "";
    string                  invite_conf_id      = "";
    string                  invite_conf_pwd1    = "";
    ConferenceItemObject    *conference_item    = NULL;

    // read attrs
    conf_id             = nc->getAttribute("conf_id");
    conf_pwd0           = nc->getAttribute("conf_pwd0");
    invite_conf_id      = nc->getAttribute("invite_conf_id");
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
        answer +=       "<event";
        answer +=           " ver=\"0.01\"";
        answer +=           " type=\"conferenceJoinFailed\"";
        answer +=           " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=           "<description msg_id=\"conferenceJoinFailed.10\">";
        answer +=               "conference not found";
        answer +=           "</description>";
        answer +=       "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // return events
        session->getItem()->queue(answer);
        goto out;
    }

    // check conference password
    if (conf_pwd0 != conference->getPwd0()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=   "<events>";
        answer +=       "<event";
        answer +=           " ver=\"0.01\"";
        answer +=           " type=\"conferenceJoinFailed\"";
        answer +=           " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=           "<description msg_id=\"conferenceJoinFailed.20\">";
        answer +=               "conference password not valid";
        answer +=           "</description>";
        answer +=       "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // return events
        session->getItem()->queue(answer);
        goto out;
    }

    // add|get item to|from conference
    conference_item = conference->getCreateConferenceItem(
        item->getHuid(),
        "conferenceJoin"
    );
    conference_item->setSrcConfId(invite_conf_id);
    conference_item->setSrcPwd(invite_conf_pwd1);

    // prepare events
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"conferenceJoinSuccess\"";
    answer +=           " conf_id=\""           + conference_item->getSrcConfId()   + "\"";
    answer +=           " conf_pwd1=\""         + conference_item->getSrcPwd()      + "\"";
    answer +=           " invite_conf_pwd1=\""  + conference_item->getPwd()         + "\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"conferenceJoinSuccess.1\">";
    answer +=               "you are successful joined with conference";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // send events
    session->getItem()->queue(answer);

    // all ok
    err = 0;

out:
    return err;
}

