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

class ApiCommandClientConferenceMsg;

#define API_COMMAND_CLIENT_CONFERENCE_MSG  "conferenceMsg"

#ifndef API_COMMAND_CLIENT_CONFERENCE_MSG_HPP
#define API_COMMAND_CLIENT_CONFERENCE_MSG_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientConferenceMsg
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientConferenceMsg();
        virtual ~ApiCommandClientConferenceMsg();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

