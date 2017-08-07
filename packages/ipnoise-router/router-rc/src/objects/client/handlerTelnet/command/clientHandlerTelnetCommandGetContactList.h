using namespace std;

class ClientHandlerTelnetCommandGetContactList;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_GETCONTACTLIST_H
#define CLIENT_HANDLER_TELNET_COMMAND_GETCONTACTLIST_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandGetContactList
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandGetContactList(
            ClientHandlerTelnetObject *handler
        );
        ~ClientHandlerTelnetCommandGetContactList();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

