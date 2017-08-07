/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class ClientHandlerTelnetCommandDevelStop;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_DEVEL_STOP_H
#define CLIENT_HANDLER_TELNET_COMMAND_DEVEL_STOP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandDevelStop
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandDevelStop(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandDevelStop();

        const char *getName();
        int run(int argc, char* const* argv);
};

#endif

