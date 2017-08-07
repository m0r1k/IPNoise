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

class ApiEventIPNoiseConferenceInviteSuccess;

#define API_EVENT_IPNOISE_CONFERENCE_INVITE_SUCCESS  "conferenceInviteSuccess"

#ifndef API_EVENT_IPNOISE_CONFERENCE_INVITE_SUCCESS_HPP
#define API_EVENT_IPNOISE_CONFERENCE_INVITE_SUCCESS_HPP

#include "objects/api/event/apiEventIPNoise.hpp"

class ApiEventIPNoiseConferenceInviteSuccess
    :   public  ApiEventIPNoise
{
    public:
        ApiEventIPNoiseConferenceInviteSuccess();
        virtual ~ApiEventIPNoiseConferenceInviteSuccess();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif

