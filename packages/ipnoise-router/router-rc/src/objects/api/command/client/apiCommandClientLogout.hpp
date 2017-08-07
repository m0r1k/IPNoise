/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandClientLogout;

#define API_COMMAND_CLIENT_LOGOUT  "logout"

#ifndef API_COMMAND_CLIENT_LOGOUT_HPP
#define API_COMMAND_CLIENT_LOGOUT_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientLogout
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientLogout();
        virtual ~ApiCommandClientLogout();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

