/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string.h>
#include "cptBase64.h"

static const char * b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

cptBase64::cptBase64()
{
}

cptBase64::~cptBase64()
{
}

size_t cptBase64::decode(string * dst, const char * src, size_t len)
{
    size_t d_sz;
    if (len == 0)
        len = strlen(src);

    d_sz = len >> 2;
    d_sz *= 3;
    if (src[len - 1] == '=')
        d_sz--;
    if (src[len - 2] == '=')
        d_sz--;

    dst->resize(d_sz);

    for (size_t i = 0, k = 0; i < len; i += 4, k += 3) {
        uint32_t triplet = 0;
        for (uint32_t j = 0; j < 64; j++) {
            if (src[i + 0] == b64_table[j])
                triplet |= ((j & 0x3f) << 18);
            if (src[i + 1] == b64_table[j])
                triplet |= ((j & 0x3f) << 12);
            if (src[i + 2] == b64_table[j])
                triplet |= ((j & 0x3f) <<  6);
            if (src[i + 3] == b64_table[j])
                triplet |= ((j & 0x3f) <<  0);
        }
        if (src[i + 2] != '=') {
//            reinterpret_cast<unsigned char*>(dst->data())[k + 1] = static_cast<uint8_t>(triplet >> 8);
            dst[k + 1] = (unsigned char)(triplet >> 8);

            if (src[i + 3] != '=')
//                (unsigned char*)(dst->data())[k + 2] = static_cast<uint8_t>(triplet);
                dst[k + 2] = (unsigned char)(triplet);

        }
//        reinterpret_cast<uint8_t *>(dst->data())[k + 0] = static_cast<uint8_t>(triplet >> 16);
            dst[k + 0] = (unsigned char)(triplet >> 16);

    }

    return len;
}

size_t cptBase64::decode(string * dst, const string * src)
{
    return cptBase64::decode(dst, src->data(), src->size());
}

size_t cptBase64::decode(string * dst, const string& src)
{
    string tmp = src;//src.toLocal8Bit();
    return cptBase64::decode(dst, &tmp);
}

string cptBase64::encode(const uint8_t * src, size_t len)
{
    string enc;
    size_t enc_len, i, j;

    if (len == 0)
        len = strlen(reinterpret_cast<const char *>(src));

    enc_len = len / 3;
    if (len % 3 != 0)
        enc_len++;
    enc_len *= 4;
    enc.resize(enc_len);
    for (i = 0; i < enc.length(); i++){
        enc[i] = '=';
    }

    for (i = 0, j = 0; i < len - (len % 3); i += 3, j += 4) {
        uint32_t triplet = 0;

        triplet |= src[i + 0] << 16;
        triplet |= src[i + 1] << 8;
        triplet |= src[i + 2] << 0;

        enc[j + 0] = b64_table[(triplet >> 18) & 0x3f];
        enc[j + 1] = b64_table[(triplet >> 12) & 0x3f];
        enc[j + 2] = b64_table[(triplet >>  6) & 0x3f];
        enc[j + 3] = b64_table[(triplet >>  0) & 0x3f];
    }

    if (len > i) {
        uint32_t triplet = 0;
        triplet |= src[i + 0] << 16;
        if (len - i == 2)
            triplet |= src[i + 1] << 8;
        enc[j + 0] = b64_table[(triplet >> 18) & 0x3f];
        enc[j + 1] = b64_table[(triplet >> 12) & 0x3f];
        if (len - i == 2)
            enc[j + 2] = b64_table[(triplet >>  6) & 0x3f];
    }

    return enc;
}

string cptBase64::encode(const char * src, size_t len)
{
    return encode(reinterpret_cast<const uint8_t *>(src), len);
}

string cptBase64::encode(const string& src)
{
    string tmp = src;//src.toLocal8Bit();
    return encode(&tmp);
}

string cptBase64::encode(string * src)
{
    return encode(src->data(), src->size());
}

