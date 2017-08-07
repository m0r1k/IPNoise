using namespace std;

class ClientHandlerTelnetCommandConferenceInvite;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_INVITE_H
#define CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_INVITE_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandConferenceInvite
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandConferenceInvite(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandConferenceInvite();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

