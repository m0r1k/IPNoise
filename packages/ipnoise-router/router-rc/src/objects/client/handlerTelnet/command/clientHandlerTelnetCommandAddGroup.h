using namespace std;

class ClientHandlerTelnetCommandAddGroup;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_ADDGROUP_H
#define CLIENT_HANDLER_TELNET_COMMAND_ADDGROUP_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandAddGroup
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandAddGroup(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandAddGroup();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

