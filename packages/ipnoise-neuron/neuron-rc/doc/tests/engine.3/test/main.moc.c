#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ipnoise-common/log_common.h>

#define CALL(a_func, a_ret_type, ...)   \
    looks_like_pre2processor_was_not_started_CALL_was_

#include "object.h"

Object          *object = NULL;
const char      *name   = "size";
const int32_t   val     = 12;
double          res;

double object_prop_set_morik(
    Object          *a_object,
    const char      *a_name,
    const int32_t   a_val)
{
    double ret = 0.123456789f;

    PWARN("MORIK was here,"
        " name: '%s', val: '%d'\n",
        a_name,
        a_val
    );

    return ret;
}

int32_t main(void)
{
    int32_t err     = -1;
    Object  *obj    = NULL;
    double  res     = 0.0f;

    if (1){
        res = object_prop_set_morik(
            obj,
            "prop_name",
            45
        );
    }

    if (0){
        res = CALL(
            object_prop_set_morik,
            double,
            (Object *)object,
            (const char *)name,
            (const int32_t)val
        );
    }

    PWARN("res: '%f'\n", res);

    // all ok
    err = 0;

    return err;
}


