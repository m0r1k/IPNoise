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

class ApiEventIPNoiseUpdateUser;

#define API_EVENT_IPNOISE_UPDATE_USER  "updateUser"

#ifndef API_EVENT_IPNOISE_UPDATE_USER_HPP
#define API_EVENT_IPNOISE_UPDATE_USER_HPP

#include "objects/api/event/apiEventIPNoise.hpp"

class ApiEventIPNoiseUpdateUser
    :   public  ApiEventIPNoise
{
    public:
        ApiEventIPNoiseUpdateUser();
        virtual ~ApiEventIPNoiseUpdateUser();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif

