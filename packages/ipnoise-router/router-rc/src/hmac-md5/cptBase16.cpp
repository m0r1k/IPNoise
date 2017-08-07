/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <cstring>
#include "cptBase16.h"

static const char * b16_table_uc = "0123456789ABCDEF";
static const char * b16_table_lc = "0123456789abcdef";

cptBase16::cptBase16()
{
}

cptBase16::~cptBase16()
{
}

size_t cptBase16::decode(string * dst, const char * src, size_t len, bool lc)
{
    size_t d_sz;
    const char * b16_table;

    if (lc)
        b16_table = b16_table_lc;
    else
        b16_table = b16_table_uc;

    if (len == 0)
        len = strlen(src);

    d_sz = len >> 1;

    dst->resize(d_sz);

    for (size_t i = 0, k = 0; i < len; i += 2, k += 1) {
        uint8_t byte = 0;
        for (uint32_t j = 0; j < 16; j++) {
            if (src[i + 0] == b16_table[j])
                byte |= ((j & 0xf) << 4);
            if (src[i + 1] == b16_table[j])
                byte |= ((j & 0xf) << 0);
        }
        dst[k] = (unsigned char)byte;
    }

    return len;
}

size_t cptBase16::decode(string * dst, const string * src, bool lc)
{
    return cptBase16::decode(dst, src->data(), src->size(), lc);
}

size_t cptBase16::decode(string * dst, const string& src, bool lc)
{
    string tmp = src;//src.toLocal8Bit();
    return cptBase16::decode(dst, &tmp, lc);
}

string cptBase16::encode(const uint8_t * src, size_t len, bool lc)
{
    string enc;
    size_t enc_len, i, j;
    const char * b16_table;

    if (lc)
        b16_table = b16_table_lc;
    else
        b16_table = b16_table_uc;

    if (len == 0)
        len = strlen(reinterpret_cast<const char *>(src));

    enc_len = len << 1;
    enc.resize(enc_len);

    for (i = 0, j = 0; i < len; i += 1, j += 2) {
        enc[j + 0] = b16_table[(src[i] >> 4) & 0xf];
        enc[j + 1] = b16_table[(src[i] >> 0) & 0xf];
    }

    return enc;
}

string cptBase16::encode(const char * src, size_t len, bool lc)
{
    return encode(reinterpret_cast<const char *>(src), len, lc);
}

string cptBase16::encode(const string& src, bool lc)
{
    string tmp = src;//src.toLocal8Bit();
    return encode(&tmp, lc);
}

string cptBase16::encode(string * src, bool lc)
{
    return encode(src->data(), src->size(), lc);
}

