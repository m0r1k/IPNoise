/*
 *  Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
 *
 */

#include "cptMD5.h"

#include <string.h>
#include <sstream>
#include <iomanip>

using namespace std;

cptMD5::cptMD5()
{
}

size_t cptMD5::blockSize() const
{
    return 64;
}

size_t cptMD5::hashSize() const
{
    return 16;
}

string cptMD5::compute(const uint8_t * msg, size_t sz)
{
    ostringstream ss;
    string s;
    uint8_t * hash;

    init();
    update(msg, sz);
    finalize(&hash);

    for (int i = 0; i < 16; i++)
        ss << hex << setfill('0') << setw(2) << static_cast<uint32_t>(hash[i]);

    s = ss.str();

    delete hash;

    return s;
}

bool cptMD5::init()
{
    _state[0] = 0x67452301;
    _state[1] = 0xefcdab89;
    _state[2] = 0x98badcfe;
    _state[3] = 0x10325476;
    _count[0] = _count[1] = 0;
    return true;
}

bool cptMD5::update(const uint8_t * src, size_t sz)
{
    size_t to_fill, in_buf;

    if (sz == 0)
        return false;

    in_buf = _count[0] & 0x3f;    // number of bytes already in buffer
    to_fill = 64 - in_buf;        // how many bytes are required to fill buffer
    _count[0] += sz;
    _count[0] &= 0xffffffff;

    if (_count[0] < sz)
        _count[1]++;

    if (in_buf > 0 && sz >= to_fill) {
        memcpy(_buf + in_buf, src, to_fill);
        transform(_buf);
        sz -= to_fill;
        src += to_fill;
        in_buf = 0;
    }

    while (sz >= 64) {
        transform(src);
        src += 64;
        sz -= 64;
    }

    if (sz > 0) {
        memcpy(_buf + in_buf, src, sz);
    }

    return true;
}

bool cptMD5::update(const string& msg)
{
    string tmp = msg;
//    tmp = msg.toLocal8Bit();
    return update((const uint8_t *)tmp.data(), tmp.length());
}

size_t cptMD5::finalize(uint8_t ** hash)
{
    static uint8_t padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    size_t padlen, in_buf;
    uint32_t sz_lo, sz_hi;
    uint8_t len[8];

    sz_hi = (_count[0] >> 29) | (_count[1] << 3);
    sz_lo = (_count[0] << 3);

    len[0] = static_cast<uint8_t>(sz_lo);
    len[1] = static_cast<uint8_t>(sz_lo >> 8);
    len[2] = static_cast<uint8_t>(sz_lo >> 16);
    len[3] = static_cast<uint8_t>(sz_lo >> 24);
    len[4] = static_cast<uint8_t>(sz_hi);
    len[5] = static_cast<uint8_t>(sz_hi >> 8);
    len[6] = static_cast<uint8_t>(sz_hi >> 16);
    len[7] = static_cast<uint8_t>(sz_hi >> 24);

    putUINT32(len, sz_lo);
    putUINT32(len + 4, sz_hi);

    in_buf = _count[0] & 0x3f;

    padlen = (in_buf < 56) ? (56 - in_buf) : (120 - in_buf);
    update(padding, padlen);
    update(len, 8);

    *hash = new uint8_t[16];

    putUINT32((*hash), _state[0]);
    putUINT32((*hash + 4), _state[1]);
    putUINT32((*hash + 8), _state[2]);
    putUINT32((*hash + 12), _state[3]);

    return 16;
}

uint32_t cptMD5::getUINT32(const uint8_t * src) const
{
    return  (static_cast<uint32_t>(src[0]) |
        (static_cast<uint32_t>(src[1]) << 8) |
        (static_cast<uint32_t>(src[2]) << 16) |
        (static_cast<uint32_t>(src[3]) << 24));
}

void cptMD5::putUINT32(uint8_t * dst, uint32_t x) const
{
    dst[0] = static_cast<uint8_t>(x);
    dst[1] = static_cast<uint8_t>(x >> 8);
    dst[2] = static_cast<uint8_t>(x >> 16);
    dst[3] = static_cast<uint8_t>(x >> 24);
}

uint32_t cptMD5::rol(uint32_t x, uint32_t n) const
{
    return (x << n) | ((x & 0xffffffff) >> (32 - n));
}

uint32_t cptMD5::F(uint32_t x, uint32_t y, uint32_t z) const
{
    return (x & y) | (~x & z);
}

uint32_t cptMD5::G(uint32_t x, uint32_t y, uint32_t z) const
{
    return (x & z) | (y & ~z);
}

uint32_t cptMD5::H(uint32_t x, uint32_t y, uint32_t z) const
{
    return x ^ y ^ z;
}

uint32_t cptMD5::I(uint32_t x, uint32_t y, uint32_t z) const
{
    return y ^ (x | ~z);
}

inline void cptMD5::FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const
{
    a = b + rol(a + F(b, c, d) + x + t, s);
}

inline void cptMD5::GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const
{
    a = b + rol(a + G(b, c, d) + x + t, s);
}

inline void cptMD5::HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const
{
    a = b + rol(a + H(b, c, d) + x + t, s);
}

inline void cptMD5::II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t t) const
{
    a = b + rol(a + I(b, c, d) + x + t, s);
}

void cptMD5::transform(const uint8_t * buf)
{
    uint32_t x[16];
    uint32_t A, B, C, D;

    for (int i = 0; i < 16; i++)
        x[i] = getUINT32(buf + (i << 2));

    A = _state[0];
    B = _state[1];
    C = _state[2];
    D = _state[3];

    FF(A, B, C, D,  x[0],  7, 0xD76AA478);
    FF(D, A, B, C,  x[1], 12, 0xE8C7B756);
    FF(C, D, A, B,  x[2], 17, 0x242070DB);
    FF(B, C, D, A,  x[3], 22, 0xC1BDCEEE);
    FF(A, B, C, D,  x[4],  7, 0xF57C0FAF);
    FF(D, A, B, C,  x[5], 12, 0x4787C62A);
    FF(C, D, A, B,  x[6], 17, 0xA8304613);
    FF(B, C, D, A,  x[7], 22, 0xFD469501);
    FF(A, B, C, D,  x[8],  7, 0x698098D8);
    FF(D, A, B, C,  x[9], 12, 0x8B44F7AF);
    FF(C, D, A, B, x[10], 17, 0xFFFF5BB1);
    FF(B, C, D, A, x[11], 22, 0x895CD7BE);
    FF(A, B, C, D, x[12],  7, 0x6B901122);
    FF(D, A, B, C, x[13], 12, 0xFD987193);
    FF(C, D, A, B, x[14], 17, 0xA679438E);
    FF(B, C, D, A, x[15], 22, 0x49B40821);

    GG(A, B, C, D,  x[1],  5, 0xF61E2562);
    GG(D, A, B, C,  x[6],  9, 0xC040B340);
    GG(C, D, A, B, x[11], 14, 0x265E5A51);
    GG(B, C, D, A,  x[0], 20, 0xE9B6C7AA);
    GG(A, B, C, D,  x[5],  5, 0xD62F105D);
    GG(D, A, B, C, x[10],  9, 0x02441453);
    GG(C, D, A, B, x[15], 14, 0xD8A1E681);
    GG(B, C, D, A,  x[4], 20, 0xE7D3FBC8);
    GG(A, B, C, D,  x[9],  5, 0x21E1CDE6);
    GG(D, A, B, C, x[14],  9, 0xC33707D6);
    GG(C, D, A, B,  x[3], 14, 0xF4D50D87);
    GG(B, C, D, A,  x[8], 20, 0x455A14ED);
    GG(A, B, C, D, x[13],  5, 0xA9E3E905);
    GG(D, A, B, C,  x[2],  9, 0xFCEFA3F8);
    GG(C, D, A, B,  x[7], 14, 0x676F02D9);
    GG(B, C, D, A, x[12], 20, 0x8D2A4C8A);

    HH(A, B, C, D,  x[5],  4, 0xFFFA3942);
    HH(D, A, B, C,  x[8], 11, 0x8771F681);
    HH(C, D, A, B, x[11], 16, 0x6D9D6122);
    HH(B, C, D, A, x[14], 23, 0xFDE5380C);
    HH(A, B, C, D,  x[1],  4, 0xA4BEEA44);
    HH(D, A, B, C,  x[4], 11, 0x4BDECFA9);
    HH(C, D, A, B,  x[7], 16, 0xF6BB4B60);
    HH(B, C, D, A, x[10], 23, 0xBEBFBC70);
    HH(A, B, C, D, x[13],  4, 0x289B7EC6);
    HH(D, A, B, C,  x[0], 11, 0xEAA127FA);
    HH(C, D, A, B,  x[3], 16, 0xD4EF3085);
    HH(B, C, D, A,  x[6], 23, 0x04881D05);
    HH(A, B, C, D,  x[9],  4, 0xD9D4D039);
    HH(D, A, B, C, x[12], 11, 0xE6DB99E5);
    HH(C, D, A, B, x[15], 16, 0x1FA27CF8);
    HH(B, C, D, A,  x[2], 23, 0xC4AC5665);

    II(A, B, C, D,  x[0],  6, 0xF4292244);
    II(D, A, B, C,  x[7], 10, 0x432AFF97);
    II(C, D, A, B, x[14], 15, 0xAB9423A7);
    II(B, C, D, A,  x[5], 21, 0xFC93A039);
    II(A, B, C, D, x[12],  6, 0x655B59C3);
    II(D, A, B, C,  x[3], 10, 0x8F0CCC92);
    II(C, D, A, B, x[10], 15, 0xFFEFF47D);
    II(B, C, D, A,  x[1], 21, 0x85845DD1);
    II(A, B, C, D,  x[8],  6, 0x6FA87E4F);
    II(D, A, B, C, x[15], 10, 0xFE2CE6E0);
    II(C, D, A, B,  x[6], 15, 0xA3014314);
    II(B, C, D, A, x[13], 21, 0x4E0811A1);
    II(A, B, C, D,  x[4],  6, 0xF7537E82);
    II(D, A, B, C, x[11], 10, 0xBD3AF235);
    II(C, D, A, B,  x[2], 15, 0x2AD7D2BB);
    II(B, C, D, A,  x[9], 21, 0xEB86D391);

    _state[0] += A;
    _state[1] += B;
    _state[2] += C;
    _state[3] += D;
}

