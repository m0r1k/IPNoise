#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

typedef struct
{
    void    *dl;
    char    *path;
} Module;

void        module_free(Module *a_module);
Module *    module_alloc();
Module *    module_load(const char *a_path);

#endif

