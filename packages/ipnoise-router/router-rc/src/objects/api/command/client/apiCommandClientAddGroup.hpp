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

class ApiCommandClientAddGroup;

#define API_COMMAND_CLIENT_ADDGROUP  "addGroup"

#ifndef API_COMMAND_CLIENT_ADDGROUP_HPP
#define API_COMMAND_CLIENT_ADDGROUP_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientAddGroup
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientAddGroup();
        virtual ~ApiCommandClientAddGroup();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

