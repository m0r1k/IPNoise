/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiEventLinkSearchRouteAnswer;

#define API_EVENT_LINK_SEARCHROUTE_ANSWER  "searchRouteAnswer"

#ifndef API_EVENT_LINK_SEARCH_ROUTE_ANSWER_HPP
#define API_EVENT_LINK_SEARCH_ROUTE_ANSWER_HPP

#include "objects/api/event/apiEventLink.hpp"

class ApiEventLinkSearchRouteAnswer
    :   public  ApiEventLink
{
    public:
        ApiEventLinkSearchRouteAnswer();
        virtual ~ApiEventLinkSearchRouteAnswer();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif
