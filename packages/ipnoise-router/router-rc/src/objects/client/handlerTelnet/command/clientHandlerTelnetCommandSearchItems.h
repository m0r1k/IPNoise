using namespace std;

class ClientHandlerTelnetCommandSearchItems;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_SEARCHITEMS_H
#define CLIENT_HANDLER_TELNET_COMMAND_SEARCHITEMS_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandSearchItems
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandSearchItems(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandSearchItems();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

