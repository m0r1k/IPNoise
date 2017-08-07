#include <stdio.h>
#include <stdint.h>

#include "network.hpp"

int32_t     debug_level  = 10;
Network     *g_network   = NULL;
Modules     g_modules;

// main thread context
ContextSptr g_context;

int32_t main(
    int32_t argc,
    char    **argv)
{
    int32_t res, err = -1;

    // init random generator
    srand(time(NULL));

    g_network = new Network(argc, argv);
    if (!g_network){
        PERROR("Cannot allocate memory\n");
        goto fail;
    }

    res = g_network->doInit();
    if (res){
        PERROR("Cannot init network\n");
        goto fail;
    }

    err = g_network->exec();

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

