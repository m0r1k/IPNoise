/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "cptHash.h"
#include "cptMD5.h"
#include "cptHMAC.h"

using namespace std;

cptHMAC::cptHMAC(cptHash * hash)
{
    if (not hash){
        hash = new cptMD5();
    }
    _hash = hash;
}

cptHMAC::~cptHMAC()
{
    delete _hash;
}

size_t cptHMAC::compute(uint8_t ** hmac, const uint8_t * msg, size_t msg_sz, const uint8_t * key, size_t key_sz)
{
    uint8_t * padded_key;
    uint8_t * ipad;
    uint8_t * opad;
    uint8_t * h_ipad;
    size_t hmac_sz;

    padded_key = new uint8_t[_hash->blockSize()];
    ipad = new uint8_t[_hash->blockSize()];
    opad = new uint8_t[_hash->blockSize()];
    memset(padded_key, 0, _hash->blockSize());

    if (key_sz > _hash->blockSize()) {    // hash the key first
        uint8_t * hashed_key;
        size_t hsz;

        _hash->init();
        _hash->update(key, key_sz);
        hsz = _hash->finalize(&hashed_key);
        memcpy(padded_key, hashed_key, hsz);
        delete hashed_key;
    } else {                // copy key directly
        memcpy(padded_key, key, key_sz);
    }

    for (size_t i = 0; i < _hash->blockSize(); i++) {
        ipad[i] = padded_key[i] ^ 0x36;    // K XOR ipad
        opad[i] = padded_key[i] ^ 0x5c;    // K XOR opad
    }

    _hash->init();
    _hash->update(ipad, _hash->blockSize());
    _hash->update(msg, msg_sz);
    _hash->finalize(&h_ipad);        // H(K XOR ipad, msg)

    _hash->init();
    _hash->update(opad, _hash->blockSize());
    _hash->update(h_ipad, _hash->hashSize());
    hmac_sz = _hash->finalize(hmac);    // H(K XOR opad, H(K XOR ipad, msg))

    delete padded_key;
    delete ipad;
    delete opad;
    delete h_ipad;
    return hmac_sz;
}

string cptHMAC::compute(const uint8_t * msg, size_t msg_sz,
    const uint8_t * key, size_t key_sz)
{
    string s;
    ostringstream ss;
    uint8_t * hmac;
    size_t hmac_sz;

    hmac_sz = compute(&hmac, msg, msg_sz, key, key_sz);
    for (size_t i = 0; i < hmac_sz; i++){
        ss << hex << setfill('0') << setw(2) << static_cast<uint32_t>(hmac[i]);
    }

    s = ss.str();

    delete hmac;

    return s;
}

string cptHMAC::compute(const string& msg, const string& key)
{
    string tmpm, tmpk;
    tmpm = msg;
    tmpk = key;

    return compute(reinterpret_cast<const uint8_t *>(tmpm.data()), tmpm.size(),
            reinterpret_cast<const uint8_t *>(tmpk.data()), tmpk.size());
}

string cptHMAC::compute96(const string& msg, const string& key)
{
    int i;
    string hmac_val_96;
    string hmac_val = compute(msg, key);

    for (i = 0; i < 12 * 2; i++){
        hmac_val_96 = hmac_val_96 + hmac_val[i];
    }
 
    return hmac_val_96;
}

