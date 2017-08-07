using namespace std;

class ClientHandlerTelnetCommandDelItem;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_DELITEM_H
#define CLIENT_HANDLER_TELNET_COMMAND_DELITEM_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandDelItem
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandDelItem(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandDelItem();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

