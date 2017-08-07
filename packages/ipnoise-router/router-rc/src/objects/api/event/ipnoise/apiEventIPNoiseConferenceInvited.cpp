/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netEventObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventIPNoiseConferenceInvited.hpp"

ApiEventIPNoiseConferenceInvited::ApiEventIPNoiseConferenceInvited()
    :   ApiEventIPNoise(API_EVENT_IPNOISE_CONFERENCE_INVITED)
{
}

ApiEventIPNoiseConferenceInvited::~ApiEventIPNoiseConferenceInvited()
{
}

int ApiEventIPNoiseConferenceInvited::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int err = 0;
    UserObject              *user               = NULL;
    ItemObject              *item               = NULL;
    SessionObject           *session            = NULL;
    ConferenceObject        *conference         = NULL;
    ConferenceItemObject    *conference_item    = NULL;
    string                  sessid              = "";
    string                  conf_id             = "";
    string                  conf_pwd1           = "";
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
    conf_id     = nc->getAttribute("conf_id");
    conf_pwd1   = nc->getAttribute("conf_pwd1");
    huid        = nc->getAttribute("huid");

    PDEBUG_OBJ(nc, 5, "ApiEventIPNoiseConferenceInvited::process\n"
        " conf_id '%s'\n",
        conf_id.c_str()
    );

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }

    if (!conference){
        goto out;
    }

    conference_item = conference->getConferenceItemByHuid(
        item->getHuid()
    );
    if (    !conference_item
        ||  (conf_pwd1 != conference_item->getPwd()))
    {
        goto out;
    }

    // add invited user in contact list
    user->addCreateItem(huid);

    // add invited user on conference
    conference->getCreateConferenceItem(
        huid,
        "conferenceInvited"
    );

    // all ok
    err = 0;

out:
    return err;
}

