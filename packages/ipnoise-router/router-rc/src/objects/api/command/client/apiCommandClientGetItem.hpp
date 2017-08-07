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

class ApiCommandClientGetItem;

#define API_COMMAND_CLIENT_GETITEM  "getItem"

#ifndef API_COMMAND_CLIENT_GETITEM_HPP
#define API_COMMAND_CLIENT_GETITEM_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientGetItem
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientGetItem();
        virtual ~ApiCommandClientGetItem();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

