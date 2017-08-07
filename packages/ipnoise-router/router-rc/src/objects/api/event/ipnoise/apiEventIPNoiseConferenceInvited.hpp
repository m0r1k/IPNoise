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

class ApiEventIPNoiseConferenceInvited;

#define API_EVENT_IPNOISE_CONFERENCE_INVITED  "conferenceInvited"

#ifndef API_EVENT_IPNOISE_CONFERENCE_INVITED_HPP
#define API_EVENT_IPNOISE_CONFERENCE_INVITED_HPP

#include "objects/api/event/apiEventIPNoise.hpp"

class ApiEventIPNoiseConferenceInvited
    :   public  ApiEventIPNoise
{
    public:
        ApiEventIPNoiseConferenceInvited();
        virtual ~ApiEventIPNoiseConferenceInvited();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif

