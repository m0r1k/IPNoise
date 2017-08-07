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

class ApiCommandClientDelItem;

#define API_COMMAND_CLIENT_DELITEM  "delItem"

#ifndef API_COMMAND_CLIENT_DELITEM_HPP
#define API_COMMAND_CLIENT_DELITEM_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientDelItem
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientDelItem();
        virtual ~ApiCommandClientDelItem();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

