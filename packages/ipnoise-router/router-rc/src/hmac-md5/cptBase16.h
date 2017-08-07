/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#ifndef CPTBASE16_H
#define CPTBASE16_H

#include <string>
#include <stdint.h>

using namespace std;

class cptBase16
{
    public:
        cptBase16();
        virtual ~cptBase16();

        static size_t decode(string * dst, const char * src,
            size_t len = 0, bool lc = false);
        static size_t decode(string * dst, const string& src,
            bool lc = false);
        static size_t decode(string * dst, const string * src,
            bool lc = false);
        static string encode(const uint8_t * src, size_t len = 0,
            bool lc = false);
        static string encode(const char * src, size_t len = 0, bool lc = false);
        static string encode(const string& src, bool lc = false);
        static string encode(string * src, bool lc = false);
};

#endif
