/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientConferenceInvite.hpp"

ApiCommandClientConferenceInvite::ApiCommandClientConferenceInvite()
    :   ApiCommandClient(API_COMMAND_CLIENT_CONFERENCE_INVITE)
{
}

ApiCommandClientConferenceInvite::~ApiCommandClientConferenceInvite()
{
}

int ApiCommandClientConferenceInvite::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject          *user           = NULL;
    SessionObject       *session        = NULL;
    ConferenceObject    *conference     = NULL;
    string              sessid          = "";
    string              conf_id         = "";
    string              huid            = "";
    string              answer          = "";
    ItemObject          *item           = NULL;

    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

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
    huid    = nc->getAttribute("huid");

    PDEBUG_OBJ(nc, 5, "ApiCommandClientConferenceInvite::process\n"
        " invite '%s' at '%s'\n",
        huid.c_str(),
        conf_id.c_str()
    );

    if (not huid.empty()){
        item = user->getItem(huid);
    }

    if (!item){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"conferenceInviteFailed\"";
        answer +=         " ver=\"0.01\">";
        answer +=         " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"conferenceInviteFailed.20\"";
        answer +=         ">";
        answer +=           "no such item";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }

    if (!conference){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"conferenceInviteFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " conf_id=\"" + conf_id + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"conferenceInviteFailed.1\"";
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

    // add huid on conference
    {
        // create new item
        ConferenceItemObject *item = NULL;
        item = conference->getCreateConferenceItem(huid);
        // invite new item
        item->doInvite();
    }

out:
    return err;
}

