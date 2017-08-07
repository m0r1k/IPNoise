/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "gen-huid.h"

using namespace std;

/*
 * Attention, HUID always must have full length, don't truncate it!
 * Example:
 *   will work      2210:0000:0000:0000:0000:0000:0001:0001
 *   will not work  2210::1
 *
 * 20110816 morik@
 */
string genHuid(
    const string    &login,
    const string    &password)
{
    string huid = "2210:0000:";
    string hash;
    cptHMAC hmac;

    hash = hmac.compute96(login, password);

    int i;
    for (i = 0; i <= 5; i++){
        huid += hash.substr(i*4, 4);
        if (5 != i){
            huid += ":";
        }
    }
    return huid;
};

