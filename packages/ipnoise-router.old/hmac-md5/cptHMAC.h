/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#ifndef CPTHMAC_H
#define CPTHMAC_H

#include "cptHash.h"

using namespace std;

class cptHMAC
{
    public:
        cptHMAC(cptHash * hash = NULL);

        virtual ~cptHMAC();
        virtual std::string compute(const uint8_t * msg, size_t msg_sz,
            const uint8_t * key, size_t key_sz);
        virtual size_t compute(uint8_t ** hmac, const uint8_t * msg,
            size_t msg_sz, const uint8_t * key, size_t key_sz);
        virtual string compute(const string& msg, const string& key);
        virtual string compute96(const string& msg, const string& key);


    protected:
        cptHash * _hash;
};

#endif

