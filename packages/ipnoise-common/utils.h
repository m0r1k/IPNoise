#ifndef IPNOISE_COMMON_UTILS_H
#define IPNOISE_COMMON_UTILS_H

#include <netdb.h>
#include <stdlib.h>

static int32_t getprotonobyname(
    const char *a_name)
{
    int32_t         ret     = -1;
    struct protoent *entry  = NULL;

    entry = getprotobyname(a_name);
    if (entry){
        ret = entry->p_proto;
    }

    endprotoent();

    return ret;
};

#endif

