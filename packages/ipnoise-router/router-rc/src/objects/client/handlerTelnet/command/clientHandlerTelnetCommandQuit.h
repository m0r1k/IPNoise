using namespace std;

class ClientHandlerTelnetCommandQuit;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_QUIT_H
#define CLIENT_HANDLER_TELNET_COMMAND_QUIT_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandQuit
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandQuit(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandQuit();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

