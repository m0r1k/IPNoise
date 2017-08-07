using namespace std;

class ClientHandlerTelnetCommandUpdateGroup;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_UPDATEGROUP_H
#define CLIENT_HANDLER_TELNET_COMMAND_UPDATEGROUP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandUpdateGroup
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandUpdateGroup(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandUpdateGroup();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

