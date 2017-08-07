/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class ClientHandlerTelnetCommandDevelDump;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_DEVEL_DUMP_H
#define CLIENT_HANDLER_TELNET_COMMAND_DEVEL_DUMP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandDevelDump
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandDevelDump(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandDevelDump();

        const char *getName();
        int run(int argc, char* const* argv);
};

#endif

