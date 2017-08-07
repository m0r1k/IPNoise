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

class ApiCommandIPNoiseGetUserInfo;

#define API_COMMAND_IPNOISE_GET_USER_INFO  "getUserInfo"

#ifndef API_COMMAND_IPNOISE_GET_USER_INFO_HPP
#define API_COMMAND_IPNOISE_GET_USER_INFO_HPP

#include "objects/api/command/apiCommandIPNoise.hpp"
#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

class ApiCommandIPNoiseGetUserInfo
    :   public  ApiCommandIPNoise
{
    public:
        ApiCommandIPNoiseGetUserInfo();
        virtual ~ApiCommandIPNoiseGetUserInfo();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        );
};

#endif

