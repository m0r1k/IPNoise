/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandIPNoiseGetUserInfo.hpp"

ApiCommandIPNoiseGetUserInfo::ApiCommandIPNoiseGetUserInfo()
    :   ApiCommandIPNoise(API_COMMAND_IPNOISE_GET_USER_INFO)
{
}

ApiCommandIPNoiseGetUserInfo::~ApiCommandIPNoiseGetUserInfo()
{
}

int ApiCommandIPNoiseGetUserInfo::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int           err      = -1;
    string        answer   = "";
    SessionObject *session = NULL;
    UserObject    *user    = NULL;

    // search session
    session = nc->getSession(skb);
    if (!session){
        // session not found
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    user = session->getOwner();

    // prepare events
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"updateUser\"";
    answer +=       ">";
    user->getAPIInfo(answer);
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

