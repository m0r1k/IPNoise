/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "cptHash.h"

cptHash::cptHash()
{
}

cptHash::~cptHash()
{
}


string cptHash::compute(const string& msg)
{
    string tmp = msg;
    return compute((const uint8_t*)tmp.data(), tmp.length());
}

