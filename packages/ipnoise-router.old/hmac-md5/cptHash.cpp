/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#include "cptHash.h"

cptHash::cptHash()
{
}

string cptHash::compute(const string& msg)
{
    string tmp = msg;
    return compute((const uint8_t*)tmp.data(), tmp.length());
}

