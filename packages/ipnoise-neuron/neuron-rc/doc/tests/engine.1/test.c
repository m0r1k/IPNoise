#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include <ipnoise-common/log_common.h>

#include "test.h"

int32_t test_speed(
    int32_t     (*a_func)(
        int32_t     a_argc,
        char        **a_argv
    ),
    uint32_t    a_count,
    int32_t     a_argc,
    char        **a_argv)
{
    int32_t         res, err    = -1;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;
    struct timeval  tv;
    uint32_t        i;

    if (!a_func){
        PFATAL("missing argument: 'a_func'\n");
    }
    if (!a_count){
        PFATAL("empty argument: 'a_count'\n");
    }

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < a_count; i++){
        res = a_func(a_argc, a_argv);
        if (res){
            err = res;
            PERROR("test failed, res: '%d'\n", res);
            goto fail;
        }
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per iteration: '%f' us\n"
        "\n",
        __FUNCTION__,
        a_count,
        time_delta,
        (time_delta * 1e6) / a_count
    );

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

