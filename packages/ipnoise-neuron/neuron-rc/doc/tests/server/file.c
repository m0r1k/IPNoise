#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "base64.h"

#include "file.h"

int64_t file_load(
    const char      *a_path,
    struct evbuffer *a_out)
{
    int32_t         res;
    int64_t         len     = -1;
    int32_t         fd      = -1;
    struct stat     st;

    // add content
    fd = open(a_path, O_RDONLY);
    if (0 > fd){
        PWARN("cannot open: '%s'"
            " for read\n",
            a_path
        );
        goto fail;
    }
    res = fstat(fd, &st);
    if (res){
        PERROR("stat failed, file: '%s'\n",
            a_path
        );
        close(fd);
        goto fail;
    }

    // evbuffer_add_file will close fd
    res = evbuffer_add_file(
        a_out,
        fd,
        0,
        st.st_size
    );
    if (res){
        PERROR("evbuffer_add_file: '%s' failed\n",
            a_path
        );
        goto fail;
    }

out:
    return st.st_size;
fail:
    if (0 <= len){
        len = -1;
    }
    goto out;
}

int32_t file_load_as_base64(
    const char      *a_path,
    struct evbuffer *a_out)
{
    int32_t         err     = -1;
    struct evbuffer *file   = NULL;
    int64_t         len     = -1;

    file = evbuffer_new();

    // trying to load file
    len = file_load(a_path, file);
    if (0 >= len){
        PERROR("cannot load file: '%s'\n",
            a_path
        );
        goto fail;
    }

    // convert to base64
    {
        char buf[ base64_encoded_len ( len ) + 1 ];
        base64_encode(
            EVBUFFER_DATA(file),
            EVBUFFER_LENGTH(file),
            buf
        );
        evbuffer_add_printf(a_out, "%s", buf);
    }

    // all ok
    err = 0;

out:
    if (file){
        evbuffer_free(file);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

