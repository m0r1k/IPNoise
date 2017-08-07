#include <getopt.h>

#include <ipnoise-common/log.h>
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConferenceJoin.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandConferenceJoin::ClientHandlerTelnetCommandConferenceJoin(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
    _handler = handler;
};

ClientHandlerTelnetCommandConferenceJoin::~ClientHandlerTelnetCommandConferenceJoin()
{
};

const char *ClientHandlerTelnetCommandConferenceJoin::getName()
{
    return "api_conferenceJoin";
};

int ClientHandlerTelnetCommandConferenceJoin::run(int argc, char* const* argv)
{
    PDEBUG(5, "conferenceJoin command received\n");
    char buffer[1024]   = { 0x00 };
    int show_xml        = 0;
    int show_usage      = 0;
    int show_prototype  = 0;
    int send_command    = 0;
    int res, err        = 0;
    string command      = "";

    PLACEHOLDERS::iterator placeholders_i;
    PLACEHOLDERS           placeholders;

    // init placeholders
    placeholders["sessid"]      =   _handler->apiGetSessionId();
    placeholders["conf_id"]     =   "";
    placeholders["flags"]       =   "create,excl";
    placeholders["conf_flags"]  =   "private";
    placeholders["huid"]        =   "";

    // init command prototype
    res = do_init(argv[0],
        "<ipnoise ver=\"0.01\" sessid=\"[[sessid]]\">\n"
        "    <commands>\n"
        "       <command type=\"conferenceJoin\" ver=\"0.01\""
                    " conf_id=\"[[conf_id]]\""
                    " flags=\"[[flags]]\""
                    " conf_flags=\"[[conf_flags]]\""
                    " huid=\"[[huid]]\""
                    "/>\n"
        "    </commands>\n"
        "</ipnoise>\n"
    );
    if (res){
        err = res;
        goto fail;
    }

    // get args
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",                0, 0, 'h'},
            {"prototype",           0, 0, 'p'},
            {"xml",                 0, 0, 'x'},
            {"sessid",              1, 0, '0'},
            {"conf_id",             1, 0, '1'},
            {"flags",               1, 0, '2'},
            {"conf_flags",          1, 0, '3'},
            {"huid",                1, 0, '4'},
            {"send",                0, 0, 's'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv,
            "hxp0:1:2:3:s",
            long_options, &option_index
        );

        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                show_usage = 1;
                break;

            case 'x':
                show_xml = 1;
                break;

            case 'p':
                show_prototype = 1;
                break;

            case '0':
                placeholders["sessid"] = optarg;
                break;

            case '1':
                placeholders["conf_id"] = optarg;
                break;

            case '2':
                placeholders["flags"] = optarg;
                break;

            case '3':
                placeholders["conf_flags"] = optarg;
                break;

            case '4':
                placeholders["huid"] = optarg;
                break;

            case 's':
                send_command = 1;
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
            "Usage:     %s [options]                                        \n"
            "options:                                                       \n"
            "         -h, --help            This help                       \n"
            "         -p, --xml             Just show prototype             \n"
            "         -x, --xml             Just show xml, don't send       \n"
            "             --sessid          API session ID                  \n"
            "             --conf_id         conference ID                   \n"
            "             --flags           open flags                      \n"
            "             --conf_flags      conference's flags              \n"
            "             --huid            conference owner's huid         \n"
            "         -s, --send            Send command                    \n"
            "\n",
            argv[0]
        );
        _handler->write(buffer);
        goto out;
    }

    if (show_prototype){
        command = getCommand();
        _handler->write("Prototype:\n");
        _handler->write(command);
    }

    // replace placeholders
    command = getCommand(placeholders);

    if (show_xml){
        _handler->write(command);
    }

    if (send_command){
        _handler->do_http_command(command);
    }

out:
    return err;

fail:
    if (!err){
        err = -1;
    }
    goto out;
};

