/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandIPNoiseConferenceJoin;

#define API_COMMAND_IPNOISE_CONFERENCE_JOIN  "conferenceJoin"

#ifndef API_COMMAND_IPNOISE_CONFERENCE_JOIN_HPP
#define API_COMMAND_IPNOISE_CONFERENCE_JOIN_HPP

#include "objects/api/command/apiCommandIPNoise.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandIPNoiseConferenceJoin
    :   public  ApiCommandIPNoise
{
    public:
        ApiCommandIPNoiseConferenceJoin();
        virtual ~ApiCommandIPNoiseConferenceJoin();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

