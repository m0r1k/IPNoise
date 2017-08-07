/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#ifndef CPTMD5_H
#define CPTMD5_H

#include <stdint.h>
#include "cptHash.h"

class cptMD5
    :   public cptHash
{
    public:
        cptMD5();
        virtual ~cptMD5();

        virtual string compute(const uint8_t * msg, size_t sz);
        virtual bool init();
        virtual bool update(const uint8_t * msg, size_t sz);
        virtual bool update(const string& msg);
        virtual size_t finalize(uint8_t ** hash);
        virtual size_t blockSize() const;
        virtual size_t hashSize() const;

    protected:
        void transform(const uint8_t * buf);

        uint32_t getUINT32(const uint8_t * src) const;
        void putUINT32(uint8_t * dst, uint32_t x) const;
        uint32_t rol(uint32_t x, uint32_t n) const;

        uint32_t F(uint32_t x, uint32_t y, uint32_t z) const;
        uint32_t G(uint32_t x, uint32_t y, uint32_t z) const;
        uint32_t H(uint32_t x, uint32_t y, uint32_t z) const;
        uint32_t I(uint32_t x, uint32_t y, uint32_t z) const;

        void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const;
        void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const;
        void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const;
        void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const;

    private:

        uint32_t _state[4];
        size_t _count[2];
        uint8_t _buf[64];
};

#endif

