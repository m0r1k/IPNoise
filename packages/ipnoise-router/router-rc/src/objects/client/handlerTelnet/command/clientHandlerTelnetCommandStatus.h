using namespace std;

class ClientHandlerTelnetCommandStatus;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_STATUS_H
#define CLIENT_HANDLER_TELNET_COMMAND_STATUS_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandStatus
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandStatus(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandStatus();

        const char *getName();
        int run(int argc, char* const* argv);
};

#endif

