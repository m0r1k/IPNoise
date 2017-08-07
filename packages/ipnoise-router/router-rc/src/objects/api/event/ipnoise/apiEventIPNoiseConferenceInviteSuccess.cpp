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

#include "apiEventIPNoiseConferenceInviteSuccess.hpp"

ApiEventIPNoiseConferenceInviteSuccess::ApiEventIPNoiseConferenceInviteSuccess()
    :   ApiEventIPNoise(API_EVENT_IPNOISE_CONFERENCE_INVITE_SUCCESS)
{
}

ApiEventIPNoiseConferenceInviteSuccess::~ApiEventIPNoiseConferenceInviteSuccess()
{
}

int ApiEventIPNoiseConferenceInviteSuccess::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int             err         = -1;
    string          sessid      = "";
    SessionObject   *session    = NULL;

    // get session
    session = nc->getSession(skb);
    if (!session){
        goto out;
    }

    // all ok
    err = 0;

out:
    return err;
}

