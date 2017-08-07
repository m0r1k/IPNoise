/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandIPNoiseConferenceMsg.hpp"

ApiCommandIPNoiseConferenceMsg::ApiCommandIPNoiseConferenceMsg()
    :   ApiCommandIPNoise(API_COMMAND_IPNOISE_CONFERENCE_MSG)
{
}

ApiCommandIPNoiseConferenceMsg::~ApiCommandIPNoiseConferenceMsg()
{
}

int ApiCommandIPNoiseConferenceMsg::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    SessionObject           *session            = NULL;
    ConferenceObject        *conference         = NULL;
    ConferenceItemObject    *conference_item    = NULL;
    DomElement              *message_el         = NULL;

    string  sessid;
    string  conf_id;
    string  conf_pwd1;
    string  answer;
    string  src_conf_huid;
    string  src_conf_id;
    string  msg_type;

    vector <ConferenceItemObject *>             conference_items;
    vector <ConferenceItemObject *>::iterator   conference_items_it;

    // search session
    session = nc->getSession(skb);


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

    // get src_conference huid
    src_conf_huid = item->getHuid();

    // read command attributes
    conf_id     = nc->getAttribute("conf_id");
    conf_pwd1   = nc->getAttribute("conf_pwd1");

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
        answer += ">";
        answer +=   "<events>";
        answer +=       "<event";
        answer +=           " ver=\"0.01\"";
        answer +=           " type=\"conferenceMsgFailed\"";
        answer +=           " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=           "<description msg_id=\"conferenceMsgFailed.10\">";
        answer +=               "conference not found";
        answer +=           "</description>";
        answer +=       "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // return events
        session->getItem()->queue(answer);
        goto out;
    }

    conference_item = conference->getConferenceItemByHuid(
        item->getHuid());
    if (    !conference_item
        ||  (conf_pwd1 != conference_item->getPwd()))
    {
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=   "<events>";
        answer +=       "<event";
        answer +=           " ver=\"0.01\"";
        answer +=           " type=\"conferenceMsgFailed\"";
        answer +=           " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=           "<description msg_id=\"conferenceMsgFailed.20\">";
        answer +=               "auth failed";
        answer +=           "</description>";
        answer +=       "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // return events
        session->getItem()->queue(answer);
        goto out;
    }

    // get src_conf_id
    src_conf_id = conference_item->getSrcConfId();

    // ok add message on conference
    message_el = nc->getElementByTagName("message");
    if (!message_el){
        goto out;
    }

    if (LO_DEV_NAME == skb->getInDevName()){
        // only for loopback (security)
        msg_type = message_el->getAttribute("type");
    }

    conference->addMsg(
        msg_type,
        message_el->getData(),
        message_el->getAttribute("id"),
        src_conf_huid,  // src huid (for deliver event)
        src_conf_id     // src conf ID (for deliver event)
    );

    // all ok
    err = 0;

out:
    return err;

}

