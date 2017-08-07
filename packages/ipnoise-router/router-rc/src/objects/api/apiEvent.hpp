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

class ApiEvent;

#ifndef API_EVENT_HPP
#define API_EVENT_HPP

#include "objects/netEventObject.h"

class ApiEvent
{
    public:
        ApiEvent(const string &a_name);
        virtual ~ApiEvent();

        virtual int process(
            NetEventObject  *nc,
            SkBuffObject    *skb
        ) = 0;

        const string getName();

    private:
        string  m_name;
};

#endif

