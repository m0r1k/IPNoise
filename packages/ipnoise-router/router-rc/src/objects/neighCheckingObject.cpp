/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/neighCheckingObject.h"

NeighCheckingObject * NeighCheckingObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NeighCheckingObject(a_node, a_doc, a_tagname);
}

NeighCheckingObject::NeighCheckingObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    // do remove us after exit (will be called SessionObject::expired)
    setExpired("0");
};

NeighCheckingObject::~NeighCheckingObject()
{
};

void NeighCheckingObject::setAttribute(
    const string    &name,
    const string    &value)
{
    // original "setAttribute" will call markAsDirty() everytime
    // we - never

    setAttributeSafe(name, value);
}

