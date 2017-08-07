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

class ApiEventClient;

#ifndef API_EVENT_CLIENT_HPP
#define API_EVENT_CLIENT_HPP

#include "objects/api/apiEvent.hpp"

class ApiEventClient
    :   public  ApiEvent
{
    public:
        ApiEventClient(const string &a_name);
        virtual ~ApiEventClient();
};

#endif

