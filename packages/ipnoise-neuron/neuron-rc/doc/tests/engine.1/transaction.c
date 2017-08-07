#include <stdlib.h>
#include <stdio.h>

#include <ipnoise-common/log_common.h>
#include "id.h"

#include "transaction.h"

void transaction_free(
    Transaction *a_transaction)
{
    if (!a_transaction){
        PFATAL("missing argument: 'a_transaction'\n");
    }

    if (a_transaction->id){
        free(a_transaction->id);
        a_transaction->id = NULL;
    }

    free(a_transaction);
}

Transaction * transaction_alloc()
{
    uint32_t    size    = 0;
    Transaction *ret    = NULL;

    ret = calloc(1, size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was needed '%d' byte(s)\n",
            size
        );
        goto fail;
    }

    ret->id = generate_id(OBJECT_ID_BYTES);

out:
    return ret;
fail:
    goto out;
}

char *transaction_dir(
    Transaction *a_transaction)
{
    char buffer[65535] = { 0x00 };
    char *ret          = NULL;

    snprintf(buffer, sizeof(buffer),
        "%s/%s",
        TRANSACTIONS_DIR,
        a_transaction->id
    );

    ret = strdup(buffer);
    if (!ret){
#ifdef __x86_64__
        PERROR("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            strlen(buffer)
        );
#else
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            strlen(buffer)
        );
#endif
        goto fail;
    }

out:
    return ret;
fail:
    goto out;
}

