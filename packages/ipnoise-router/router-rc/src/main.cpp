/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <getopt.h>
#include <signal.h>
#include "main.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

int g_debug_level       = 0;
IPNoiseObject *ipnoise  = NULL;

static void shutdown_cb(evutil_socket_t, short, void *arg)
{
    IPNoiseObject *ipnoise = (IPNoiseObject *)arg;
    if (ipnoise){
        ipnoise->getDocument()->shutdown();
    }
}

int main(int argc, char *argv[])
{
    char buffer[1024];
    int res, err = 0;
    DomDocument *dom = NULL;
    struct stat st;

    string std_out, std_err;
    struct event *signal_event1 = NULL;
    struct event *signal_event2 = NULL;

    string config_file = DEFAULT_CONFIG_FILE;

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"config",      1, 0, 'c'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "hc:vd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 0:
                break;

            case 'h':
                PINFO("Usage:     %s [-options]                   \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "         -c, --config      Path to config.xml\n"
                    "         -v, --version     Program version   \n"
                    "         --debug <level>   Debug level       \n"
                    "\n",
                    argv[0]
                );
                exit(0);

            case 'c':
                config_file = optarg;
                break;

            case 'd':
                g_debug_level = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n", IPNOISE_ROUTER_VERSION);
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
        PINFO("config file: '%s'\n", config_file.c_str());
        PINFO("debug-level: '%d'\n", g_debug_level);
        PINFO("\n");
    }

    // init DOM
    res = stat(config_file.c_str(), &st);
    if (res){
        PERROR("Cannot open config file: '%s'\n", config_file.c_str());
        goto fail;
    }

    dom = new DomDocument(DomDocument::USE_ALL_PRIVS);
    if (!dom){
        PERROR("Cannot create DomDocument\n");
        goto fail;
    }

    res = dom->loadFromFile(config_file);
    if (res){
        PERROR("Cannot load config from file: '%s'\n",
            config_file.c_str());
        goto fail;
    }

    ipnoise = (IPNoiseObject *)dom->getRootElement();
    if (!ipnoise){
        PERROR("Cannot get <ipnoise> element :(\n");
        goto fail;
    }

    signal_event1 = evsignal_new(
        ipnoise->getDocument()->getEvBase(),
        SIGINT,
        shutdown_cb,
        (void *)ipnoise
    );

    if (!signal_event1 || event_add(signal_event1, NULL) < 0) {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    signal_event2 = evsignal_new(
        ipnoise->getDocument()->getEvBase(),
        SIGTERM,
        shutdown_cb,
        (void *)ipnoise
    );

    if (!signal_event2 || event_add(signal_event2, NULL) < 0) {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    // start events loop
    event_base_dispatch(ipnoise->getDocument()->getEvBase());

ret:
    if (signal_event1 != NULL){
        event_free(signal_event1);
        signal_event1 = NULL;
    }
    if (signal_event2 != NULL){
        event_free(signal_event2);
        signal_event2 = NULL;
    }

/*
    // stop libevent
    if (dns_base){
        evdns_base_free(dns_base, 0);
        dns_base = NULL;
    }
    if (base){
        event_base_free(base);
        base = NULL;
    }
*/


    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto ret;

}

