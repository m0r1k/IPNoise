using namespace std;

class ClientHandlerTelnetCommandLogin;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_LOGIN_H
#define CLIENT_HANDLER_TELNET_COMMAND_LOGIN_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandLogin
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandLogin(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandLogin();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

