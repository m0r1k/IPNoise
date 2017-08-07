/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "hmac-md5/gen-huid.h"

#include "apiCommandClientPing.hpp"

ApiCommandClientPing::ApiCommandClientPing()
    :   ApiCommandClient(API_COMMAND_CLIENT_PING)
{
}

ApiCommandClientPing::~ApiCommandClientPing()
{
}

int ApiCommandClientPing::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = -1;
    SessionObject *session = NULL;

    // search session in "unknown" sessions list
    session = nc->getSession(skb);

    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    //session->xmit();

    // all ok
    err = 0;

out:
    return err;
}

