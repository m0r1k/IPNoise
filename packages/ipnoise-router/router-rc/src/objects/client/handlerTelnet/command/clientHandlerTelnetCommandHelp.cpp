#include <getopt.h>

#include <ipnoise-common/log.h>
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandHelp.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandHelp::ClientHandlerTelnetCommandHelp(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
};

ClientHandlerTelnetCommandHelp::~ClientHandlerTelnetCommandHelp()
{
};

const char *ClientHandlerTelnetCommandHelp::getName()
{
    return "help";
};

int ClientHandlerTelnetCommandHelp::run(int argc, char* const* argv)
{
    PDEBUG(5, "help command received\n");
    int err = 0;
    char buffer[1024] = { 0x00 };

    // init optind
    optind = 1;

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
                snprintf(buffer, sizeof(buffer),
                    "Usage:     %s [-options]                     \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "\n",
                    argv[0]
                );
                write(buffer);
                break;
        }
    }

    // check another args
    if (optind < argc){
        snprintf(buffer, sizeof(buffer),
            "Warning: argument(s) will be ignored: ");
        write(buffer);
        while (optind < argc){
            snprintf(buffer, sizeof(buffer), "'%s' ", argv[optind++]);
            write(buffer);
        }
        snprintf(buffer, sizeof(buffer), "\n");
        write(buffer);
    }

    return err;
};

