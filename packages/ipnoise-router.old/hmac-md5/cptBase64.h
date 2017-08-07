/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#ifndef CPTBASE64_H
#define CPTBASE64_H

#include <string>
#include <stdint.h>

using namespace std;

class cptBase64
{
    public:
        static size_t decode(string * dst, const char * src, size_t len = 0);
        static size_t decode(string * dst, const string& src);
        static size_t decode(string * dst, const string * src);
        static string encode(const uint8_t * src, size_t len = 0);
        static string encode(const char * src, size_t len = 0);
        static string encode(const string& src);
        static string encode(string * src);

    protected:
        cptBase64();

};

#endif

