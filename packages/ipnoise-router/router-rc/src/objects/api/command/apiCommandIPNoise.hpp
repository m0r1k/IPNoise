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

class ApiCommandIPNoise;

#ifndef API_COMMAND_IPNOISE_HPP
#define API_COMMAND_IPNOISE_HPP

#include "objects/api/apiCommand.hpp"

class ApiCommandIPNoise
    :   public  ApiCommand
{
    public:
        ApiCommandIPNoise(const string &a_name);
        virtual ~ApiCommandIPNoise();
};

#endif

