#include <getopt.h>

#include <ipnoise-common/log.h>
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandSearchItems.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandSearchItems::ClientHandlerTelnetCommandSearchItems(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
    _handler = handler;
};

ClientHandlerTelnetCommandSearchItems::~ClientHandlerTelnetCommandSearchItems()
{
};

const char *ClientHandlerTelnetCommandSearchItems::getName()
{
    return "api_searchItems";
};

int ClientHandlerTelnetCommandSearchItems::run(int argc, char* const* argv)
{
    PDEBUG(5, "searchitems command received\n");
    int err = 0;
    char buffer[1024] = { 0x00 };

    // init optind
    optind = 1;

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help", 0, 0, 'h'},
            {"xml",  0, 0, 'x'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv,
            "hx",
            long_options, &option_index
        );

        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                snprintf(buffer, sizeof(buffer),
                    "Usage:     %s [options]                      \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "         -x, --xml         Show command xml  \n"
                    "\n",
                    argv[0]
                );
                write(buffer);
                break;
            case 'x':
                snprintf(buffer, sizeof(buffer),
                    "<ipnoise>"
                    "</ipnoise>"
                    "\n"
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

