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

class ApiCommandClientAddItem;

#define API_COMMAND_CLIENT_ADDITEM  "addItem"

#ifndef API_COMMAND_CLIENT_ADDITEM_HPP
#define API_COMMAND_CLIENT_ADDITEM_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientAddItem
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientAddItem();
        virtual ~ApiCommandClientAddItem();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

