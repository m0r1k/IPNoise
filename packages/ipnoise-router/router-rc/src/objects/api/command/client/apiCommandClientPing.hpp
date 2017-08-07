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

class ApiCommandClientPing;

#define API_COMMAND_CLIENT_PING  "ping"

#ifndef API_COMMAND_CLIENT_PING_HPP
#define API_COMMAND_CLIENT_PING_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

class ApiCommandClientPing
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientPing();
        virtual ~ApiCommandClientPing();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

