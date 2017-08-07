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

class ApiCommandClientUpdateItem;

#define API_COMMAND_CLIENT_UPDATEITEM  "updateItem"

#ifndef API_COMMAND_CLIENT_UPDATEITEM_HPP
#define API_COMMAND_CLIENT_UPDATEITEM_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientUpdateItem
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientUpdateItem();
        virtual ~ApiCommandClientUpdateItem();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

