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

class ApiEventIPNoise;

#ifndef API_EVENT_IPNOISE_HPP
#define API_EVENT_IPNOISE_HPP

#include "objects/api/apiEvent.hpp"

class ApiEventIPNoise
    :   public  ApiEvent
{
    public:
        ApiEventIPNoise(const string &a_name);
        virtual ~ApiEventIPNoise();
};

#endif

