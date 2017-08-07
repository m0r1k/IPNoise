using namespace std;

class ClientHandlerTelnetCommandConferenceJoin;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_JOIN_H
#define CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_JOIN_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandConferenceJoin
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandConferenceJoin(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandConferenceJoin();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

