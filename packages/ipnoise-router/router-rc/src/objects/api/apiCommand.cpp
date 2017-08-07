/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommand.hpp"

ApiCommand::ApiCommand(const string &a_name)
    :   m_name(a_name)
{
}

ApiCommand::~ApiCommand()
{
}

const string ApiCommand::getName()
{
    return m_name;
}

