#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "log.h"
#include "object.h"

int32_t file_load(
    const char  *a_file_name,
    char        **a_out)
{
    int32_t     res, err    = -1;
    int32_t     fd          = -1;
    size_t      was_read    = 0;
    struct stat st;

    fd = open(a_file_name, O_RDONLY);
    if (0 > fd){
        TERROR("cannot open: '%s' for read\n",
            a_file_name
        );
        goto fail;
    }

    res = fstat(fd, &st);
    if (res){
        TERROR("stat: '%s' failed\n",
            a_file_name
        );
        goto fail;
    }

    *a_out = malloc(st.st_size);
    if (!*a_out){
        TERROR("cannot allocate memory,"
            " was needed: '%ld' byte(s)\n",
            st.st_size
        );
        goto fail;
    }

    do {
        res = read(
            fd,
            *a_out + was_read,
            st.st_size - was_read
        );
        if (0 < res){
            // part of data
            was_read += res;
        } else if (!res){
            // eof
            break;
        } else if (errno == EINTR){
            // continue
        } else {
            // error
            break;
        }
    } while (1);

    if (st.st_size != was_read){
        TERROR("st.st_size (%ld) != was_read: '%ld'\n",
            st.st_size,
            was_read
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    if (0 <= fd){
        close(fd);
    }
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t main(void)
{
    int32_t     res, err    = -1;
    Object      *window     = NULL;
    Object      *child      = NULL;
    const char  *fname      = "test.js";
    char        *code       = NULL;

    res = file_load(fname, &code);
    if (res){
        TERROR("cannot open: '%s' for read\n", fname);
        goto fail;
    }

    res = int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr(
        "object",
        "Window",
        &window
    );
    if (res){
        PERROR("int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr() failed\n");
        goto fail;
    }

    // create child
    res = int32t___object__create___Object_ptr_ptr(&child);
    if (res){
        PERROR("int32t___object__create___Object_ptr_ptr() failed\n");
        goto fail;
    }

    res = int32t___object__add_child___Object_ptr__Object_ptr__BackLinkType(
        window,
        child,
        BACKLINK_PARENT
    );

    if (res){
        PERROR("int32t___object__add_child___Object_ptr__Object_ptr__BackLinkType() failed\n");
        goto fail;
    }

    //err = int32t___object__parse___Object_ptr__const_char_ptr(
    //    window,
    //    code
    //);

    void___object__ref_dec___Object_ptr(
        window
    );

    // all ok
    err = 0;

out:
    if (code){
        free(code);
    }

    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
};

