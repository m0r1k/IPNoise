/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#ifndef CPTHASH_H
#define CPTHASH_H

#include <string>
#include <stdint.h>

using namespace std;

class cptHash
{
    public:
        cptHash();
        virtual string compute(const uint8_t * msg, size_t sz) = 0;
        virtual string compute(const string& msg);
        virtual bool init() = 0;
        virtual bool update(const uint8_t * msg, size_t sz) = 0;
        virtual size_t finalize(uint8_t ** hash) = 0;
        virtual size_t blockSize() const = 0;
        virtual size_t hashSize() const = 0;
};

#endif

