#include <map>
#include <string>

using namespace std;

class ClientHandlerTelnetCommand;

typedef map <string, string> PLACEHOLDERS;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_ABSTRACT_H
#define CLIENT_HANDLER_TELNET_COMMAND_ABSTRACT_H

#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommand(ClientHandlerTelnetObject *handler);
        virtual ~ClientHandlerTelnetCommand();

        virtual const char *getName() = 0;
        virtual int run(int argc, char* const* argv) = 0;

        int write(const string &buffer);
        string getCommand(PLACEHOLDERS &placeholders);
        string getCommand();

    protected:
        int do_init(
            char            *_optarg,
            const char      *command
        );

        ClientHandlerTelnetObject    *_handler;

    private:
        string  _command;
};

#endif

