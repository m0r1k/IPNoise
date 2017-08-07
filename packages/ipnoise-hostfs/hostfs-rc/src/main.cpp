#include <getopt.h>

#include "server.hpp"
#include "main.hpp"

int debug_level = 0;

int main(int argc, char **argv)
{
    int                 err             = -1;
    struct event_base   *base           = NULL;
    string              self_name       = argv[0];
    Server              server;
    char                buffer[1024]    = { 0x00 };

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "hvd:",
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
                    "         -v, --version     Program version   \n"
                    "         --debug <level>   Debug level       \n"
                    "\n",
                    argv[0]
                );
                exit(0);

            case 'd':
                debug_level = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n", IPNOISE_HOSTOS_VERSION);
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

    if (debug_level){
        PINFO("Options:\n");
        PINFO("debug-level:    '%d'\n", debug_level);
        PINFO("\n");
    }

    // init libevent
    base = event_base_new();
    if (not base){
        PERROR("Could not initialize libevent!\n");
        goto fail;
    }

    // init server
    err = server.do_init(base);
    if (err){
        PERROR("Cannot init ipnoise hostos server,"
            " err: '%d'\n", err);
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

