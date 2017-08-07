#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"

ClientHandlerTelnetCommand::ClientHandlerTelnetCommand(ClientHandlerTelnetObject *handler)
{
    _handler            = handler;
    _command            = "";
}

ClientHandlerTelnetCommand::~ClientHandlerTelnetCommand()
{
}

int ClientHandlerTelnetCommand::write(const string &buffer)
{
    return _handler->write(buffer);
}

string ClientHandlerTelnetCommand::getCommand(PLACEHOLDERS &placeholders)
{
    string ret = _command;
    map <string, string>::iterator placeholders_i;

    if (!placeholders.size()){
        goto out;
    }

    for (placeholders_i = placeholders.begin();
        placeholders_i != placeholders.end();
        placeholders_i++)
    {
        string::size_type start_pos;
        string key = placeholders_i->first;
        string val = placeholders_i->second;
again:
        start_pos = ret.find("[["+key+"]]");
        if (start_pos != string::npos){
            ret.replace(start_pos, (key.size() + 4), val);
            goto again;
        }
    }

out:
    return ret;
}

string ClientHandlerTelnetCommand::getCommand()
{
    PLACEHOLDERS placeholders;
    return getCommand(placeholders);
}

int ClientHandlerTelnetCommand::do_init(
    char            *_optarg,
    const char      *command)
{
    int err = 0;

    // init getopt
    optarg = _optarg;
    optind = 0;
    opterr = 0;
    optopt = 0;

    _command = command;

    return err;
}

