/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#ifndef GEN_HUID_H
#define GEN_HUID_H

#include <string.h>
#include <string>

#include "cptHMAC.h"
#include "cptMD5.h"

using namespace std;

string genHuid(
    const string    &login,
    const string    &password
);

#endif

