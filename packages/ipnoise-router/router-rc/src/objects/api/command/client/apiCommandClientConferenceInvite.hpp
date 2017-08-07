/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandClientConferenceInvite;

#define API_COMMAND_CLIENT_CONFERENCE_INVITE  "conferenceInvite"

#ifndef API_COMMAND_CLIENT_CONFERENCE_INVITE_HPP
#define API_COMMAND_CLIENT_CONFERENCE_INVITE_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientConferenceInvite
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientConferenceInvite();
        virtual ~ApiCommandClientConferenceInvite();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

