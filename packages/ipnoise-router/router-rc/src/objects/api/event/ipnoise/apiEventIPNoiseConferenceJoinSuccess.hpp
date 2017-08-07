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

class ApiEventIPNoiseConferenceJoinSuccess;

#define API_EVENT_IPNOISE_CONFERENCE_JOIN_SUCCESS  "conferenceJoinSuccess"

#ifndef API_EVENT_IPNOISE_CONFERENCE_JOIN_SUCCESS_HPP
#define API_EVENT_IPNOISE_CONFERENCE_JOIN_SUCCESS_HPP

#include "objects/api/event/apiEventIPNoise.hpp"

class ApiEventIPNoiseConferenceJoinSuccess
    :   public  ApiEventIPNoise
{
    public:
        ApiEventIPNoiseConferenceJoinSuccess();
        virtual ~ApiEventIPNoiseConferenceJoinSuccess();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif

