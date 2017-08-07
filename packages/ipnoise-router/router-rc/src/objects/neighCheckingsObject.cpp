/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "objects/neighCheckingsObject.h"

NeighCheckingsObject * NeighCheckingsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NeighCheckingsObject(a_node, a_doc, a_tagname);
}

NeighCheckingsObject::NeighCheckingsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    // do remove us after exit (will be called SessionObject::expired)
    setExpired("0");
};

NeighCheckingsObject::~NeighCheckingsObject()
{
};

