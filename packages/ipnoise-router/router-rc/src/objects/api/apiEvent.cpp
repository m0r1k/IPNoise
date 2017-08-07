/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiEvent.hpp"

ApiEvent::ApiEvent(const string &a_name)
    :   m_name(a_name)
{
}

ApiEvent::~ApiEvent()
{
}

const string ApiEvent::getName()
{
    return m_name;
}

