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

class ApiCommandLinkSearchUsers;

#define API_COMMAND_LINK_SEARCH_USERS  "searchUsers"

#ifndef API_COMMAND_LINK_SEARCH_USERS_HPP
#define API_COMMAND_LINK_SEARCH_USERS_HPP

#include "objects/api/command/apiCommandLink.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

class ApiCommandLinkSearchUsers
    :   public  ApiCommandLink
{
    public:
        ApiCommandLinkSearchUsers();
        virtual ~ApiCommandLinkSearchUsers();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );

        void processNeigh(
            NetCommandObject    *nc,
            SkBuffObject        *skb,
            NeighObject         *neigh
        );
};

#endif

