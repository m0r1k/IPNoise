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
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConnect.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

ClientHandlerTelnetCommandConnect::ClientHandlerTelnetCommandConnect(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
};

ClientHandlerTelnetCommandConnect::~ClientHandlerTelnetCommandConnect()
{
};

const char *ClientHandlerTelnetCommandConnect::getName()
{
    return "connect";
};

void ClientHandlerTelnetCommandConnect::http_connected_cb(
    NetClientHttpObject     *, // *http_client
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };
    ClientHandlerTelnetObject *_handler = (ClientHandlerTelnetObject *)ctx;
    snprintf(buffer, sizeof(buffer), "[http_connected_cb]"
        " current_offset: '%u', current_length: '%u'\n",
        current_offset,
        current_length
    );
    _handler->write(buffer);
}

void ClientHandlerTelnetCommandConnect::http_connect_closed_cb(
    NetClientHttpObject     *, // *http_client
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };
    ClientHandlerTelnetObject *_handler = (ClientHandlerTelnetObject *)ctx;
    snprintf(buffer, sizeof(buffer), "[http_connect_closed_cb]"
        " current_offset: '%u', current_length: '%u'\n",
        current_offset,
        current_length
    );
    _handler->write(buffer);
}

// --------------------------------------------------------------------------80

void ClientHandlerTelnetCommandConnect::do_close_conn(
    ClientHandlerTelnetObject *_handler)
{
    assert(_handler);
    NetClientHttpObject *net_client_http    = NULL;
    DomElement          *element            = NULL;

    element = _handler->getElementByTagName("ipn_net_client_http");
    if (element){
        net_client_http = (NetClientHttpObject *)element;
        net_client_http->close();
    }

    return;
}

void ClientHandlerTelnetCommandConnect::do_show_status()
{
    string conn_type    = "";
    string conn_to      = "";
    string conn_state   = "";
    string status       = "";

    NetClientHttpObject *net_client_http    = NULL;
    DomElement          *element            = NULL;

    element = _handler->getElementByTagName("ipn_net_client_http");

    status += "http connection";
    if (element){
        net_client_http = (NetClientHttpObject *)element;
        string dst_addr = net_client_http->getDstAddr();
        string dst_port = net_client_http->getDstPort();

        if (dst_addr.size()){
            status += " to '" + dst_addr + "' port '" + dst_port + "'";
        }
        status += " is " + net_client_http->getState() + "";
        status += "\n";
    } else {
        status += " is closed\n";
    }

    _handler->write(status);

    return;
}

int ClientHandlerTelnetCommandConnect::run(int argc, char* const* argv)
{
    PDEBUG(5, "connect command received\n");
    char buffer[1024]       = { 0x00 };
    int show_usage          = 0;
    int err                 = 0;
    string host_port        = DEFAULT_HTTP_API_HOST_PORT;
    string cmd              = "";
    string sessid;

    // init command prototype
    do_init(argv[0], "");

    // get args
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",            0, 0, 'h'},
            {"status",          0, 0, '0'},
            {"close",           0, 0, '1'},
            {"http",            0, 0, '5'},
            {"raw",             0, 0, '6'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv,
            "h015:6:",
            long_options, &option_index
        );

        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                show_usage = 1;
                break;
            case '0':
                // show status
                do_show_status();
                break;
            case '1':
                // close connection
                do_close_conn(_handler);
                break;
            case '5':
                // http-connect
                sessid = _handler->apiGetSessionId();
                cmd =  "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
                cmd += "<ipnoise ver=\"0.01\" sessid=\""+sessid+"\">";
                cmd += "  <commands>";
                cmd += "    <command type=\"openSession\" ver=\"0.01\" locale=\"en\"/>";
                cmd += "  </commands>";
                cmd += "</ipnoise>";
                if (optarg){
                    host_port = optarg;
                }
                _handler->apiSetHostPort(host_port);
                _handler->do_http_command(cmd);
                break;
            case '6':
                // raw-connect
                // do_raw_conn(optarg);
                _handler->write("yet unsupported\n");
                break;
            default:
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
            "Usage:     %s [-options]                                    \n"
            "options:                                                    \n"
            "         -h,   --help                      This help        \n"
            "               --status                    Show status      \n"
            "               --hostport  <host:port>     default: '%s'    \n"
            "               --http                      Connect to engine\n"
            "               --raw                       Connect to engine\n"
            "               --close                     Close connection \n"
            "\n",
            argv[0],
            DEFAULT_HTTP_API_HOST_PORT
        );
        _handler->write(buffer);
        goto out;
    }

out:
    return err;
};

