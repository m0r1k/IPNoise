/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "apiCommandClientLogout.hpp"

ApiCommandClientLogout::ApiCommandClientLogout()
    :   ApiCommandClient(API_COMMAND_CLIENT_LOGOUT)
{
}

ApiCommandClientLogout::~ApiCommandClientLogout()
{
}

int ApiCommandClientLogout::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    SessionObject   *session    = NULL;
    string          sessid      = "";
    string          answer      = "";

    // search session
    session = nc->getSession(skb);
    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session ID
    sessid = session->getSessId();

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"logoutSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"logoutSuccess.1\"";
    answer +=         ">";
    answer +=           "logout success";
    answer +=         "</description>";
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer only to current client
    session->queue(answer);

    // delete session
    DomDocument::deleteElement(session);

out:
    return err;
}

