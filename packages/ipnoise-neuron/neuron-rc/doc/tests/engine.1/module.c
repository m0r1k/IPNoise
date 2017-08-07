#include <dlfcn.h>
//#include <search.h>
#include <ipnoise-common/log_common.h>

#include "module.h"

/*
struct hsearch_data *g_modules = module_init_htable();

struct hsearch_data * module_init_htable()
{
    struct hsearch_data *htable = NULL;

    htable = calloc(1, sizeof(htable));
    if (!htable){
        PFATAL("cannot allocate memory,"
            " was needed: '%d' byte(s)\n"
        );
    }

    return htable;
}
*/

void module_free(
    Module *a_module)
{
    if (!a_module){
        PFATAL("missing argument: 'a_module'\n");
    }

    if (a_module->dl){
        dlclose(a_module->dl);
        a_module->dl = NULL;
    }

    if (a_module->path){
        free(a_module->path);
        a_module->path = NULL;
    }

    free(a_module);
}

Module * module_alloc()
{
    Module      *module = NULL;
    uint32_t    size    = sizeof(*module);

    module = calloc(1, size);
    if (!module){
        PERROR("cannot allocate memory,"
            " was needed '%d' byte(s)\n",
            size
        );
        goto fail;
    }

out:
    return module;
fail:
    if (module){
        module_free(module);
        module = NULL;
    }
    goto out;
}

Module * module_load(
    const char *a_path)
{
    Module *module = NULL;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    module = module_alloc();
    if (!module){
        PERROR("cannot allocate module\n");
        goto fail;
    }

    // open module
    module->dl = dlopen(a_path, RTLD_LAZY);
    if (!module->dl){
        PERROR("cannot open: '%s' (%s)\n",
            a_path,
            dlerror()
        );
        goto fail;
    }

    // remember file path
    module->path = strdup(a_path);
    if (!module->path){
#ifdef __x86_64__
        PERROR("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            strlen(a_path)
        );
#else
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            strlen(a_path)
        );
#endif
        goto fail;
    }

out:
    return module;
fail:
    if (module){
        module_free(module);
        module = NULL;
    }
    goto out;
}

