/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

using namespace std;

class ApiEventLinkSearchUsersAnswer;

#define API_EVENT_LINK_SEARCH_USERS_ANSWER  "searchUsersAnswer"

#ifndef API_EVENT_LINK_SEARCH_USERS_ANSWER_HPP
#define API_EVENT_LINK_SEARCH_USERS_ANSWER_HPP

#include "objects/api/event/apiEventLink.hpp"

class ApiEventLinkSearchUsersAnswer
    :   public  ApiEventLink
{
    public:
        ApiEventLinkSearchUsersAnswer();
        virtual ~ApiEventLinkSearchUsersAnswer();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        );
};

#endif
