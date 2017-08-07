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

class ApiCommandClientLogin;

#define API_COMMAND_CLIENT_LOGIN  "login"

#ifndef API_COMMAND_CLIENT_LOGIN_HPP
#define API_COMMAND_CLIENT_LOGIN_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

class ApiCommandClientLogin
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientLogin();
        virtual ~ApiCommandClientLogin();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

