/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#ifndef GEN_HUID_H
#define GEN_HUID_H

#include <string.h>
#include "cptHMAC.h"
#include "cptMD5.h"

using namespace std;

string genHuid(string login, string password);

#endif

