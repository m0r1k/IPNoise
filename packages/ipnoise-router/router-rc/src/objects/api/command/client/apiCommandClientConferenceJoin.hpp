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

class ApiCommandClientConferenceJoin;

#define API_COMMAND_CLIENT_CONFERENCE_JOIN  "conferenceJoin"

#ifndef API_COMMAND_CLIENT_CONFERENCE_JOIN_HPP
#define API_COMMAND_CLIENT_CONFERENCE_JOIN_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientConferenceJoin
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientConferenceJoin();
        virtual ~ApiCommandClientConferenceJoin();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

