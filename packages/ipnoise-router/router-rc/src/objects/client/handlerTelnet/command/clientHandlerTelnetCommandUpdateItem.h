class ClientHandlerTelnetCommandUpdateItem;

using namespace std;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_UPDATEITEM_H
#define CLIENT_HANDLER_TELNET_COMMAND_UPDATEITEM_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandUpdateItem
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandUpdateItem(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandUpdateItem();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

