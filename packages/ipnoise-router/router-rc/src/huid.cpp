#include "hmac-md5/gen-huid.h"
#include "huid.hpp"

HUID::HUID(const string &a_huid)
{
    memset(m_huid, 0x00, sizeof(m_huid));

    m_huid[0] = 0x2210;

    if (a_huid.size()){
        setHuid(a_huid);
    }
}

HUID::HUID(
    const string    &a_login,
    const string    &a_password)
{
    string huid = ::genHuid(a_login, a_password);
    setHuid(huid);
}

HUID::~HUID()
{
}

uint32_t HUID::getLocalId()
{
    uint32_t res = 65536 * m_huid[6] + m_huid[7];
    return res;
}

int32_t HUID::setHuid(const string &a_huid)
{
    int i, err = -1;
    Splitter huid(a_huid, ":");
    memset(m_huid, 0x00, sizeof(m_huid));

    if (8 != huid.size()){
        goto fail;
    }

    if ("2210" != huid[0]){
        goto fail;
    }

    for (i = 0; i < 8; i++){
        uint32_t tmp = 0;
        sscanf(huid[i].c_str(), "%x", &tmp);
        m_huid[i] = tmp;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    PERROR("Cannot init HUID from: '%s'\n", a_huid.c_str());
    goto out;
}

bool HUID::isLocal()
{
    return m_huid[1] & 0x8000;
}

void HUID::setLocal(bool a_val)
{
    if (a_val){
        m_huid[1] |= 0x8000; // 1000 0000 0000 0000
    } else {
        m_huid[1] &= 0x7000; // 0111 0000 0000 0000
    }
}

void HUID::resetPlace()
{
    m_huid[1] &= 0x8fff; // 1000 1111 1111 1111
}

void HUID::setPlace(HUID::Places a_place)
{
    unsigned short a = 0x8fff;
    unsigned short b = (a_place & 0x7) << 12;
    unsigned short c = a | b;

    resetPlace();
    m_huid[1] |= c;
}

HUID::Places HUID::getPlace()
{
    unsigned short res = (m_huid[1] & 0x7fff) >> 16; // 0111
    return (HUID::Places)res;
}

string HUID::toString()
{
    int32_t i;
    char buffer[512] = { 0x00 };
    string huid;

    for (i = 0; i < 8; i++){
        snprintf(buffer, sizeof(buffer), "%4.4x", m_huid[i]);
        if (huid.size()){
            huid += ":";
        }
        huid += buffer;
    }
    return huid;
}

