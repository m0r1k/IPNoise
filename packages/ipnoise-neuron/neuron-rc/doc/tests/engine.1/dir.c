#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ipnoise-common/log_common.h>

#include "dir.h"

int32_t mkdirr(
    const char      *a_path,
    const uint32_t  a_mode)
{
    const char  *path       = a_path;
    int32_t     res, err    = -1;
    struct stat st;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    PDEBUG(100, "attempt to mkdir: '%s', mode: '%d'\n",
        a_path,
        a_mode
    );

    do {
        uint8_t c = *path;
        if ('\0' == c){
            break;
        }
        if ('/' == c){
            uint32_t size  = path - a_path;
            char     *part = strndup(a_path, size + 1);
            if (!part){
                PERROR("cannot allocate memory,"
                    " was needed: '%d' byte(s)\n",
                    size
                );
                goto fail;
            }
            res = stat(part, &st);
            if (    res
                ||  !S_ISDIR(st.st_mode))
            {
                res = mkdir(part, a_mode);
                if (res){
                    PERROR("mkdir: '%s' (mode: '%d') failed,"
                        " res: '%d'\n",
                        part,
                        a_mode,
                        res
                    );
                    free(part);
                    part = NULL;
                    goto fail;
                }
                free(part);
                part = NULL;
            }
        }
        path++;
    } while (1);

    if ('\0' != *a_path){
        res = stat(a_path, &st);
        if (    res
            ||  !S_ISDIR(st.st_mode))
        {
            res = mkdir(a_path, a_mode);
            if (res){
                PERROR("mkdir: '%s' (mode: '%d') failed,"
                    " res: '%d'\n",
                    a_path,
                    a_mode,
                    res
                );
                goto fail;
            }
        }
    }

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

