#include <getopt.h>

#include <ipnoise-common/log.h>
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandStatus.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandStatus::ClientHandlerTelnetCommandStatus(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
};

ClientHandlerTelnetCommandStatus::~ClientHandlerTelnetCommandStatus()
{
};

const char *ClientHandlerTelnetCommandStatus::getName()
{
    return "status";
};

int ClientHandlerTelnetCommandStatus::run(int argc, char* const* argv)
{
    PDEBUG(5, "status command received\n");
    char buffer[1024]       = { 0x00 };
    int show_usage          = 0;
    int err                 = 0;
    string sessid           = "";
    string handler_status   = "";

    // init command prototype
    do_init(argv[0], "");

    // get args
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", 0, 0, 'h'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv,
            "h",
            long_options, &option_index
        );

        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                show_usage = 1;
                break;
        }
    }

    // check another args
    if (optind < argc){
        snprintf(buffer, sizeof(buffer),
            "Warning: argument(s) will be ignored: ");
        _handler->write(buffer);
        while (optind < argc){
            snprintf(buffer, sizeof(buffer), "'%s' ", argv[optind++]);
            _handler->write(buffer);
        }
        snprintf(buffer, sizeof(buffer), "\n");
        _handler->write(buffer);
    }

    if (show_usage){
        snprintf(buffer, sizeof(buffer),
            "Usage:     %s [-options]                     \n"
            "options:                                     \n"
            "         -h, --status        This status     \n"
            "\n",
            argv[0]
        );
        _handler->write(buffer);
        goto out;
    }

    // dump handler status
    _handler->write("IPNoise client status:  '"+handler_status+"'\n");
    _handler->write("IPNoise Client ID:     '"+sessid+"'\n");


out:
    return err;
};

