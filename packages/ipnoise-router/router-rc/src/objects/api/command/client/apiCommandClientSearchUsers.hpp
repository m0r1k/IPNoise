/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandClientSearchUsers;

#define API_COMMAND_CLIENT_SEARCH_USERS  "searchUsers"

#ifndef API_COMMAND_CLIENT_SEARCH_USERS_HPP
#define API_COMMAND_CLIENT_SEARCH_USERS_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

class ApiCommandClientSearchUsers
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientSearchUsers();
        virtual ~ApiCommandClientSearchUsers();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

