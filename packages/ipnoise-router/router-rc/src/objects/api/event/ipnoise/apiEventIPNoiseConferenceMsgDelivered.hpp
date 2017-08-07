/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiEventIPNoiseConferenceMsgDelivered;

#define API_EVENT_IPNOISE_CONFERENCE_MSG_DELIVERED  "conferenceMsgDelivered"

#ifndef API_EVENT_IPNOISE_CONFERENCE_MSG_DELIVERED_HPP
#define API_EVENT_IPNOISE_CONFERENCE_MSG_DELIVERED_HPP

#include "objects/api/event/apiEventIPNoise.hpp"

class ApiEventIPNoiseConferenceMsgDelivered
    :   public  ApiEventIPNoise
{
    public:
        ApiEventIPNoiseConferenceMsgDelivered();
        virtual ~ApiEventIPNoiseConferenceMsgDelivered();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif

