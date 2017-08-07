/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *  IPNoise HUID generator
 *
 */

#include <stdio.h>
#include <string.h>
#include "cptHMAC.h"
#include "cptMD5.h"

#include "gen-huid.h"

using namespace std;

int main(int argc, char * argv[])
{
    if (argc != 3){
        printf("Usage: %s login password\n", argv[0]);
        return 1;
    }
    string  msg = argv[1];
    string  key = argv[2];

    int     i;
    string  md5_sum;
    string  hmac_val;
    string  hmac_val96;
    string  huid;
    cptMD5  md5;
    cptHMAC hmac;

    md5_sum = md5.cptHash::compute(msg);

    printf("Message:     %s\n", msg.c_str());
    printf("MD5:         %s\n", md5_sum.c_str());

    hmac_val    = hmac.compute(msg, key);
    hmac_val96  = hmac.compute96(msg, key);
    huid        = genHuid(msg, key);;

    printf("HMAC-MD5:    %s\n", hmac_val.c_str());
    printf("HMAC-MD5-96: %s\n", hmac_val96.c_str());
    printf("HUID:        %s\n", huid.c_str());

    return 0;
}

