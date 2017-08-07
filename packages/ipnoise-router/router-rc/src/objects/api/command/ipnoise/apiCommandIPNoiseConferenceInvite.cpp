/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandIPNoiseConferenceInvite.hpp"

ApiCommandIPNoiseConferenceInvite::ApiCommandIPNoiseConferenceInvite()
    :   ApiCommandIPNoise(API_COMMAND_IPNOISE_CONFERENCE_INVITE)
{
}

ApiCommandIPNoiseConferenceInvite::~ApiCommandIPNoiseConferenceInvite()
{
}

int ApiCommandIPNoiseConferenceInvite::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int                     err                 = -1;
    ConferenceObject        *conference         = NULL;
    string                  answer              = "";
    SessionObject           *session            = NULL;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    ConferenceItemObject    *conference_item    = NULL;
    string                  conf_id;
    string                  conf_topic;
    string                  conf_pwd0;

    // read attrs
    conf_id     = nc->getAttribute("conf_id");
    conf_topic  = nc->getAttribute("conf_topic");
    conf_pwd0   = nc->getAttribute("conf_pwd0");

    // search session
    session = nc->getSession(skb);
    if (!session){
        // session not found
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    user = session->getOwner();
    item = session->getItem();

    // request user info
    item->requestUserInfo();

    conference = user->createConference("private", conf_topic);
    if (!conference){
        PERROR_OBJ(nc, "Cannot create new conference\n");
        err = nc->errorInternalError(skb);
        goto out;
    }

    // set conference password
    conference->setPwd0(conf_pwd0);

    // add item to conference
    conference_item = conference->getCreateConferenceItem(
        item->getHuid(),
        "conferenceJoin"
    );
    conference_item->setSrcConfId(conf_id);

    // inform about invited
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event type=\"conferenceInviteSuccess\" ver=\"0.01\"/>";
    answer +=   "</events>";
    answer += "</ipnoise>";
    session->getItem()->queue(answer);

    // join to invited conference
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<commands>";
    answer +=       "<command type=\"conferenceJoin\" ver=\"0.01\""
        " conf_id=\""            + conference_item->getSrcConfId()   + "\""
        " conf_pwd0=\""          + conference->getPwd0()             + "\""
        " invite_conf_id=\""     + conference->getId()               + "\""
        " invite_conf_pwd1=\""   + conference_item->getPwd()         + "\""
        "/>";
    answer +=   "</commands>";
    answer += "</ipnoise>";
    session->getItem()->queue(answer);

    // all ok
    err = 0;

out:
    return err;
}

