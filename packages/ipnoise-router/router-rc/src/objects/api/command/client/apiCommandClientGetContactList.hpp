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

class ApiCommandClientGetContactList;

#define API_COMMAND_CLIENT_GETCONTACTLIST  "getContactList"

#ifndef API_COMMAND_CLIENT_GETCONTACTLIST_HPP
#define API_COMMAND_CLIENT_GETCONTACTLIST_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientGetContactList
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientGetContactList();
        virtual ~ApiCommandClientGetContactList();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

