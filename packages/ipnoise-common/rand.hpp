#ifndef RAND_HPP
#define RAND_HPP

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// stl
#include <string>

#include <ipnoise-common/types.h>

using namespace std;

static string getRand(const int &a_size = 8)
{
    int32_t buf_size    = a_size * 2 + 1;
    char    *buf        = NULL;
    int i,  byte        = 0;
    string  ret;

    // alloc memory
    buf = (char *)malloc(buf_size);
    if (not buf){
        goto out;
    }

    // clear
    memset(buf, 0x00, buf_size);

    // generate
    for (i = 0; i < a_size; i++){
        byte = int((double(rand())/RAND_MAX)*255);
        snprintf(buf + i*2, buf_size - i*2, "%2.2x", byte);
    }

    // store result
    ret = buf;

    // clear
    free(buf);

out:
    return ret;
}

#endif

