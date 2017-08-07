using namespace std;

class ClientHandlerTelnetCommandAddItem;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_ADDITEM_H
#define CLIENT_HANDLER_TELNET_COMMAND_ADDITEM_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandAddItem
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandAddItem(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandAddItem();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

