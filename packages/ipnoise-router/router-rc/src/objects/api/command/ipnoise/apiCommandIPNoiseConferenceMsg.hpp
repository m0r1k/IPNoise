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

class ApiCommandIPNoiseConferenceMsg;

#define API_COMMAND_IPNOISE_CONFERENCE_MSG  "conferenceMsg"

#ifndef API_COMMAND_IPNOISE_CONFERENCE_MSG_HPP
#define API_COMMAND_IPNOISE_CONFERENCE_MSG_HPP

#include "objects/api/command/apiCommandIPNoise.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandIPNoiseConferenceMsg
    :   public  ApiCommandIPNoise
{
    public:
        ApiCommandIPNoiseConferenceMsg();
        virtual ~ApiCommandIPNoiseConferenceMsg();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

