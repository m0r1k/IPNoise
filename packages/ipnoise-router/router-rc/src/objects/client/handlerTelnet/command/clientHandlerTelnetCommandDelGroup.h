using namespace std;

class ClientHandlerTelnetCommandDelGroup;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_DELGROUP_H
#define CLIENT_HANDLER_TELNET_COMMAND_DELGROUP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandDelGroup
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandDelGroup(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandDelGroup();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

