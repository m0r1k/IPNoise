using namespace std;

class ClientHandlerTelnetCommandGetItem;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_GETITEM_H
#define CLIENT_HANDLER_TELNET_COMMAND_GETITEM_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandGetItem
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandGetItem(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandGetItem();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

