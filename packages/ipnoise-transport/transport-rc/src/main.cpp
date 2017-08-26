#include <getopt.h>

#include "server.hpp"
#include "main.hpp"

int g_debug_level = 0;

int main(int argc, char **argv)
{
    int err                     = -1;
    struct event_base *base     = NULL;
    string transport_name       = "";
    string lladdr               = "";
    string self_name            = argv[0];
    Server server;
    char buffer[1024];

    // check self name
    if (string::npos != self_name.rfind("ipnoise-transport-tcp-v4"))
    {
        transport_name = "tcp_v4";
    } else if (string::npos != self_name.rfind(
            "ipnoise-transport-udp-v4"))
    {
        transport_name = "udp_v4";
    }

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"type",        1, 0, 't'},
            {"lladdr",      1, 0, 'l'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "ht:l:vd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 0:
                break;

            case 'h':
                PINFO(
                    "Usage:     %s [-options]                     \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "         -t, --type        [tcp_v4|udp_v4]     \n"
                    "         -l, --lladdr      Link address      \n"
                    "         -v, --version     Program version   \n"
                    "         --debug <level>   Debug level       \n"
                    "\n",
                    argv[0]
                );
                exit(0);

            case 't':
                transport_name = optarg;
                break;

            case 'l':
                lladdr = optarg;
                break;

            case 'd':
                g_debug_level = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n", IPNOISE_TRANSPORT_VERSION);
                exit(0);

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        char *buffer_ptr = buffer;
        int res;
        res = snprintf(buffer_ptr, (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: ");
        if (res > 0){
            buffer_ptr += res;
        }
        while (optind < argc){
            snprintf(buffer_ptr, (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ", argv[optind++]);
        }
        PERROR("%s\n", buffer);
    }

    if (g_debug_level){
        PINFO("Options:\n");
        PINFO("transport-type: '%s'\n", transport_name.c_str());
        PINFO("lladdr:         '%s'\n", lladdr.c_str());
        PINFO("debug-level:    '%d'\n", g_debug_level);
        PINFO("\n");
    }

    if (    transport_name == "tcp_v4"
        ||  transport_name == "udp_v4")
    {
    } else {
        PERROR("Missing or unsupported transport type: '%s',"
            " please use argument '--type transport'"
            " for provide transport type\n",
            transport_name.c_str()
        );
        goto fail;
    }

    // init libevent
    base = event_base_new();
    if (not base){
        PERROR("Could not initialize libevent!\n");
        goto fail;
    }

    // init server
    err = server.do_init(base, transport_name, lladdr);
    if (err){
        PERROR("Cannot init transport: '%s'\n", transport_name.c_str());
        goto fail;
    }

    // start events loop
    event_base_dispatch(base);

    err = 0;
out:
    return err;
fail:
    err = -1;
    goto out;
}

