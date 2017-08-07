using namespace std;

class ClientHandlerTelnetCommandGetGroup;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_GETGROUP_H
#define CLIENT_HANDLER_TELNET_COMMAND_GETGROUP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandGetGroup
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandGetGroup(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandGetGroup();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

