/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#include "gen-huid.h"

using namespace std;

string genHuid(string login, string password)
{
    string huid = "2210:0:";
    string hash;
    cptHMAC hmac;

    hash = hmac.compute96(login, password);

    int i;
    for (i = 0; i <= 5; i++){
        huid += hash.substr(i*4, 4);
        if (i != 5){
            huid += ":";
        }
    }
    return huid;
};

