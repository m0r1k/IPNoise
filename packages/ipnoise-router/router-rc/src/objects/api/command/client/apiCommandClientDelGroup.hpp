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

class ApiCommandClientDelGroup;

#define API_COMMAND_CLIENT_DELGROUP  "delGroup"

#ifndef API_COMMAND_CLIENT_DELGROUP_HPP
#define API_COMMAND_CLIENT_DELGROUP_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientDelGroup
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientDelGroup();
        virtual ~ApiCommandClientDelGroup();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

