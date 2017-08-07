/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <getopt.h>

#include <ipnoise-common/log.h>
#include "libxml2/domDocument.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandDevelDump.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandDevelDump::ClientHandlerTelnetCommandDevelDump(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
};

ClientHandlerTelnetCommandDevelDump::~ClientHandlerTelnetCommandDevelDump()
{
};

const char *ClientHandlerTelnetCommandDevelDump::getName()
{
    return "devel_dump";
};

int ClientHandlerTelnetCommandDevelDump::run(int argc, char* const* argv)
{
    PDEBUG(5, "devel-dump command received\n");
    char buffer[1024]       = { 0x00 };
    int show_usage          = 0;
    int i, err              = 0;
    string dump1            = "";
    string dump2            = "";

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
            "         -h, --help          This help       \n"
            "\n",
            argv[0]
        );
        _handler->write(buffer);
        goto out;
    }

    // dump handler status
    dump1 = _handler->getDocument()->getRootElement()->serialize(1);
    for (i = 0; i < (int)dump1.size(); i++){
        if (dump1[i] == '\n'){
            dump2 += "\n";
        } else {
            dump2 += dump1[i];
        }
    }
    dump2 += "\n";
    _handler->write(dump2);


out:
    return err;
};

