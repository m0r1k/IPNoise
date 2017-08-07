/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandIPNoiseConferenceInvite;

#define API_COMMAND_IPNOISE_CONFERENCE_INVITE  "conferenceInvite"

#ifndef API_COMMAND_IPNOISE_CONFERENCE_INVITE_HPP
#define API_COMMAND_IPNOISE_CONFERENCE_INVITE_HPP

#include "objects/api/command/apiCommandIPNoise.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandIPNoiseConferenceInvite
    :   public  ApiCommandIPNoise
{
    public:
        ApiCommandIPNoiseConferenceInvite();
        virtual ~ApiCommandIPNoiseConferenceInvite();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

