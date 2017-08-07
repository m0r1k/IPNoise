/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientConferenceMsg.hpp"

ApiCommandClientConferenceMsg::ApiCommandClientConferenceMsg()
    :   ApiCommandClient(API_COMMAND_CLIENT_CONFERENCE_MSG)
{
}

ApiCommandClientConferenceMsg::~ApiCommandClientConferenceMsg()
{
}

int ApiCommandClientConferenceMsg::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject          *user           = NULL;
    SessionObject       *session        = NULL;
    ConferenceObject    *conference     = NULL;
    DomElement          *message_el     = NULL;

    string    sessid;
    string    conf_id;
    string    answer;
    string    message;
    string    msg_id;

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

    // read command attributes
    conf_id = nc->getAttribute("conf_id");

    PDEBUG_OBJ(nc, 5, "ApiCommandClientConferenceMsg::process\n"
        " conf_id '%s'\n",
        conf_id.c_str()
    );

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }

    if (!conference){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"conferenceMsgFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"conferenceMsgFailed.1\"";
        answer +=         ">";
        answer +=           "conference not found";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    message_el = nc->getElementByTagName("message");
    if (!message_el){
        // there are no message
        goto out;
    }

    // generate message ID
    msg_id = conference->createMsgId();

    conference->getConferenceItems(conference_items);
    for (conference_items_it = conference_items.begin();
        conference_items_it != conference_items.end();
        conference_items_it++)
    {
        ConferenceItemObject    *conference_item = NULL;
        string                  conf_id;
        string                  conf_pwd1;

        conference_item = *conference_items_it;

        if (user->getHuid() == conference_item->getHuid()){
            // it is owner
            conf_id     = conference->getId();
            conf_pwd1   = conference_item->getPwd();
        } else {
            conf_id     = conference_item->getSrcConfId();
            conf_pwd1   = conference_item->getSrcPwd();
        }

        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=   "<commands>";
        answer +=       "<command";
        answer +=           " type=\"conferenceMsg\"";
        answer +=           " ver=\"0.01\"";
        answer +=           " conf_id=\""   + conf_id   + "\"";
        answer +=           " conf_pwd1=\"" + conf_pwd1 + "\"";
        answer +=       ">";
        answer +=           "<message";
        answer +=               " id=\"" + msg_id + "\"";
        answer +=               " type=\"outcoming\"";
        answer +=           ">";
        answer +=               "<![CDATA[";
        answer +=                   message_el->getData();
        answer +=               "]]>";
        answer +=           "</message>";
        answer +=       "</command>";
        answer +=   "</commands>";
        answer += "</ipnoise>";

        conference_item->queue(answer);
    }

out:
    return err;
}

