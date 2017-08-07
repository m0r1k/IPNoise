#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>

#include <ipnoise-common/log_common.h>

#include "dir.h"
#include "test.h"
#include "transaction.h"
#include "object.h"

int32_t     g_debug_level   = 0;
GHashTable  *g_hash         = NULL;

void usage(
    int32_t a_argc,
    char    **a_argv)
{
    PERROR("usage: %s <file.js>\n",
        a_argv[0]
    );
}

int32_t test_create_child_link_unlink(
    int32_t a_argc,
    char    **a_argv)
{
    int32_t     err                 = -1;
    char        *object_name        = NULL;
//    Transaction *transaction        = NULL;
    Object      *object_file        = NULL;
    Object      *object_dumper      = NULL;
    char        *fname              = NULL;

    if (1 >= a_argc){
        usage(a_argc, a_argv);
        goto fail;
    }

    // get file name
    fname = a_argv[1];

/*
    // create transaction
    transaction = transaction_alloc();
    if (!transaction){
        PERROR("cannot allocate transaction\n");
        goto fail;
    }
*/

    // create file object
    object_name = "core.object.file";
    object_file = object_alloc(
//        transaction,
        object_name
    );
    if (!object_file){
        PERROR("cannot create object: '%s'\n",
            object_name
        );
        goto fail;
    }

    // create dumper
    object_name     = "core.object.dumper";
    object_dumper   = object_alloc(
//        transaction,
        object_name
    );
    if (!object_dumper){
        PERROR("cannot create object: '%s'\n",
            object_name
        );
        goto fail;
    }

    // add dumper to file
    object_child_link(
//        transaction,
        object_file,
        object_dumper
    );

    // start file
    object_action_process_str(
        object_file,
        "to.stream.of.bytes",
        fname
    );

    //object_child_unlink(
    //    transaction,
    //    object_file,
    //    object_dumper
    //);

    if (object_file){
        object_free(object_file);
        object_file = NULL;
    }
    if (object_dumper){
        object_free(object_dumper);
        object_dumper = NULL;
    }
/*
    if (transaction){
        transaction_free(transaction);
        transaction = NULL;
    }
*/
    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t test_string(
    int32_t a_argc,
    char    **a_argv)
{
    int32_t     err             = -1;
    char        buffer[65535]   = { 0x00 };

    if (0){
        // 1e3 iterations:
        // hostos:  time per iteration - 0.061989 us
        // ipnoise:
        Object      *object = NULL;
        uint32_t    size    = sizeof(*object);

        object = malloc(size);
        free(object);
    }

    if (0){
        // 1e3 iteration:
        // hostos:  time per iteration - 51.884890 us
        // ipnoise:
        Object *object = NULL;
        object = object_alloc("core.object.file");

        if (object){
            object_free(object);
            object = NULL;
        }
    }

    if (0){
        // 1e3 iteration:
        // hostos:  time per iteration - 8.048058 us
        // ipnoise:
        snprintf(buffer, sizeof(buffer),
            "%d",
            g_hash_table_size(g_hash)
        );

        g_hash_table_insert(
            g_hash,
            strdup(buffer),
            strdup(buffer)
        );

        g_hash_table_lookup(g_hash, "123");
    }

    if (0){
        // 1e3 iteration:
        // hostos:  time per iteration - 42.474031 us
        // ipnoise:
        char *id = generate_id(20);
        snprintf(buffer, sizeof(buffer),
            "%s/%s",
            OBJECTS_DIR,
            id
        );
        mkdirr(buffer, DEFAULT_MKDIR_MODE);
        free(id);
    }

    if (1){
        // 1e3 iteration:
        // hostos:  time per iteration - 13.196230 us
        // ipnoise:
        char *id = "123";
        snprintf(buffer, sizeof(buffer),
            "%s/%s",
            OBJECTS_DIR,
            id
        );
        mkdirr(buffer, DEFAULT_MKDIR_MODE);
    }

    // all ok
    err = 0;

    return err;
}

int32_t main(
    int32_t a_argc,
    char    **a_argv)
{
    int32_t res, err = -1;
    char    *key     = "1";

    // init random generator
    srand(time(NULL));

    g_hash = g_hash_table_new(
        g_str_hash,
        g_str_equal
    );

    res = test_speed(
        test_string,
        1e3,
        a_argc,
        a_argv
    );
    if (res){
        err = res;
        PERROR("test failed, res: '%d'\n",
            res
        );
        goto fail;
    }

    printf("There are %d keys in the hash table\n",
        g_hash_table_size(g_hash)
    );

    key = "1";
    printf("value by key: '%s' -> '%s'\n",
        key,
        (char *)g_hash_table_lookup(g_hash, key)
    );

    // all ok
    err = 0;

out:
    if (g_hash){
        g_hash_table_destroy(g_hash);
        g_hash = NULL;
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

