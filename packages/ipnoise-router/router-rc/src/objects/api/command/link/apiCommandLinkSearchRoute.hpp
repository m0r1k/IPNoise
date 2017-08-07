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

class ApiCommandLinkSearchRoute;

#define API_COMMAND_LINK_SEARCHROUTE  "searchRoute"

#ifndef API_COMMAND_LINK_SEARCHROUTE_HPP
#define API_COMMAND_LINK_SEARCHROUTE_HPP

#include "objects/api/command/apiCommandLink.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"
#include "objects/ipnoiseObject.h"

class ApiCommandLinkSearchRoute
    :   public  ApiCommandLink
{
    public:
        ApiCommandLinkSearchRoute();
        virtual ~ApiCommandLinkSearchRoute();

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

