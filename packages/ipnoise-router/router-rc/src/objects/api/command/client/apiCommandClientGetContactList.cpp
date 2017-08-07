/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientGetContactList.hpp"

ApiCommandClientGetContactList::ApiCommandClientGetContactList()
    :   ApiCommandClient(API_COMMAND_CLIENT_GETCONTACTLIST)
{
}

ApiCommandClientGetContactList::~ApiCommandClientGetContactList()
{
}

int ApiCommandClientGetContactList::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user           = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = "";
    string          answer          = "";

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

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"getContactListSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    user->getContactList(answer);
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise";

    // send answer only to current client
    session->queue(answer);

out:
    return err;
}

