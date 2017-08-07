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

class ApiCommandClient;

#ifndef API_COMMAND_CLIENT_HPP
#define API_COMMAND_CLIENT_HPP

#include "objects/api/apiCommand.hpp"

class ApiCommandClient
    :   public  ApiCommand
{
    public:
        ApiCommandClient(const string &a_name);
        virtual ~ApiCommandClient();
};

#endif

