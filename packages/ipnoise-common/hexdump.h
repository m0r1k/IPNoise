#ifndef IPNOISE_HEXDUMP_H
#define IPNOISE_HEXDUMP_H

#ifndef __KERNEL__
#include <string.h>
#include "ipnoise-common/log_common.h"
#else
#define PINFO   pr_info
#define PERROR  pr_err
#endif

/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

inline static void ipnoise_hexdump(
    const unsigned char   *a_data,
    const int32_t         a_len)
{
    int             i, m        = 0;
    int             rows        = a_len % 16 + 1;
    int             malloc_len  = 0;
    unsigned char   *buffer     = NULL;
    unsigned char   line[20];

    if (0 >= a_len){
        goto out;
    }

    malloc_len = 1024 + 1024*((rows + 1)*128)%1024;

    PINFO("hexdump, trying to alloc '%d' byte(s)\n",
        malloc_len
    );

#ifndef __KERNEL__
    buffer = (unsigned char *)calloc(
        1,
        malloc_len
    );
#else
    buffer = (unsigned char *)kzalloc(
        malloc_len,
        GFP_KERNEL
    );
#endif
    if (!buffer){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)",
            malloc_len
        );
        goto out;
    }
    for (i = 0, m = 0;
        i < a_len + (16 - (a_len % 16));
        i++, m++)
    {
        if (i < a_len){
            sprintf(
                (char *)(buffer + strlen((const char*)buffer)),
                "%2.2x ",
                (unsigned int)a_data[i]
            );
            if (    32 <= a_data[i]
                &&  127 > a_data[i])
            {
                line[m] = a_data[i];
            } else {
                line[m] = '.';
            }
        } else {
            sprintf(
                (char *)(buffer + strlen((const char*)buffer)),
                "   "
            );
            line[m] = ' ';
        }
        if (i && ((i + 1) % 8 == 0)){
            sprintf(
                (char *)(buffer + strlen((const char*)buffer)),
                " "
            );
        }
        if (i && ((i + 1) % 16 == 0)){
            line[m + 1] = '\0';
            m = -1;
            sprintf(
                (char *)(buffer + strlen((const char*)buffer)),
                " %s\n",
                line
            );
        }
    }
    sprintf(
        (char *)(buffer + strlen((const char*)buffer)),
        "\n"
    );
    PINFO("%s\n", buffer);

#ifndef __KERNEL__
    free(buffer);
#else
    kfree(buffer);
#endif

out:
    return;
};

#endif

