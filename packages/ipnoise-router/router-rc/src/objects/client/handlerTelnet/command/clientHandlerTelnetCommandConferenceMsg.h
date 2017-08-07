using namespace std;

class ClientHandlerTelnetCommandConferenceMsg;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_MSG_H
#define CLIENT_HANDLER_TELNET_COMMAND_CONFERENCE_MSG_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommandConferenceMsg
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandConferenceMsg(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandConferenceMsg();

        const char *getName();
        int run(int argc, char* const* argv);

    private:
        ClientHandlerTelnetObject *_handler;
};

#endif

