/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiCommandClientUpdateContact;

#define API_COMMAND_CLIENT_UPDATE_CONTACT  "updateContact"

#ifndef API_COMMAND_CLIENT_UPDATE_CONTACT_HPP
#define API_COMMAND_CLIENT_UPDATE_CONTACT_HPP

#include "objects/api/command/apiCommandClient.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandClientUpdateContact
    :   public  ApiCommandClient
{
    public:
        ApiCommandClientUpdateContact();
        virtual ~ApiCommandClientUpdateContact();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

