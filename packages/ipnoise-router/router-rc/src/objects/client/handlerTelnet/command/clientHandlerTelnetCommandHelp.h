using namespace std;

class ClientHandlerTelnetCommandHelp;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_HELP_H
#define CLIENT_HANDLER_TELNET_COMMAND_HELP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandHelp
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandHelp(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandHelp();

        const char *getName();
        int run(int argc, char* const* argv);
};

#endif

