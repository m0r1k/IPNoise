using namespace std;

class ClientHandlerTelnetCommandLogout;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_LOGOUT_H
#define CLIENT_HANDLER_TELNET_COMMAND_LOGOUT_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandLogout
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandLogout(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandLogout();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

