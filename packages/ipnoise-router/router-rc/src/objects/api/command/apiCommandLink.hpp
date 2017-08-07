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

class ApiCommandLink;

#ifndef API_COMMAND_LINK_HPP
#define API_COMMAND_LINK_HPP

#include "objects/api/apiCommand.hpp"

class ApiCommandLink
    :   public  ApiCommand
{
    public:
        ApiCommandLink(const string &a_name);
        virtual ~ApiCommandLink();
};

#endif

