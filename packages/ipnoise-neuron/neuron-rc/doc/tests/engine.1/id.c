#include <stdlib.h>
#include <stdio.h>

#include <ipnoise-common/log_common.h>

#include "id.h"

char * generate_id(
    const uint32_t a_len_bytes)
{
    char        *id     = NULL;
    uint32_t    size    = a_len_bytes * 2 + 1;
    uint32_t    i;

    id = calloc(1, size);
    if (!id){
        PERROR("cannot allocate memory,"
            " was needed '%d' byte(s)\n",
            size
        );
        goto fail;
    }

    for (i = 0; i < a_len_bytes; i++){
        uint8_t byte = ((double)rand() / RAND_MAX) * 255;
        snprintf(id + i*2, size - i*2,
            "%2.2x",
            byte
        );
    }

out:
    return id;
fail:
    goto out;
}

char * generate_id2(
    const uint32_t a_len_bytes)
{
    FILE    *file  = NULL;
    char    *ret   = NULL;
    char    *fname = "/dev/urandom";
    char    *mode  = "r";
    int32_t res;

    ret = malloc(a_len_bytes + 1);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            a_len_bytes
        );
        goto fail;
    }

    file = fopen(fname, mode);
    if (!file){
        PERROR("cannot open: '%s' for '%s'\n",
            fname,
            mode
        );
        goto fail;
    }

    res = fread(ret, 1, a_len_bytes, file);

    ret[res] = '\0';

    fclose(file);

out:
    return ret;
fail:
    goto out;
}

