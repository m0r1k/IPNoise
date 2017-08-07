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

class ApiCommand;

#ifndef API_COMMAND_HPP
#define API_COMMAND_HPP

#include "objects/netCommandObject.h"
#include "objects/skBuffObject.h"

#include "objects/sessionsObject.h"
#include "objects/sessionObject.h"

class ApiCommand
{
    public:
        ApiCommand(const string &a_name);
        virtual ~ApiCommand();

        virtual int process(
            NetCommandObject    *nc,
            SkBuffObject        *skb
        ) = 0;

        const string getName();

    private:
        string  m_name;
};

#endif

