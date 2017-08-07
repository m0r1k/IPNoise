/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include "objects/ioObject.h"

IoObject * IoObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new IoObject(a_node, a_doc, a_tagname);
}

IoObject::IoObject(
    xmlNodePtr      _node,
    DomDocument     *_doc,
    const string    &_tagname)
    :   Object(_node, _doc, _tagname)
{
    // don't save us after exit
    setExpired("0");
};

IoObject::~IoObject()
{
};

void IoObject::addTxBytes(size_t bytes)
{
    char    buffer[1024]    = { 0x00 };
    size_t  tx_bytes        = 0;

    string  tx_bytes_str = getAttribute("tx_bytes");
    if (not tx_bytes_str.empty()){
        tx_bytes += atol(tx_bytes_str.c_str());
    }

    tx_bytes += bytes;

    snprintf(buffer, sizeof(buffer),
        "%d", tx_bytes);
    setAttributeSafe("tx_bytes", buffer);
}

void IoObject::addRxBytes(size_t bytes)
{
    char    buffer[1024]    = { 0x00 };
    size_t  rx_bytes        = 0;

    string  rx_bytes_str = getAttribute("rx_bytes");
    if (not rx_bytes_str.empty()){
        rx_bytes += atol(rx_bytes_str.c_str());
    }

    rx_bytes += bytes;

    snprintf(buffer, sizeof(buffer),
        "%d", rx_bytes);
    setAttributeSafe("rx_bytes", buffer);
}

