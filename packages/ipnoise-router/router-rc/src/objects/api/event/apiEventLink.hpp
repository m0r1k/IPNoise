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

class ApiEventLink;

#ifndef API_EVENT_LINK_HPP
#define API_EVENT_LINK_HPP

#include "objects/api/apiEvent.hpp"

class ApiEventLink
    :   public  ApiEvent
{
    public:
        ApiEventLink(const string &a_name);
        virtual ~ApiEventLink();
};

#endif

