/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/object.h"

Object * Object::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new Object(a_node, a_doc, a_tagname);
}

Object::Object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   DomElement(a_node, a_doc, a_tagname)
{
};

Object::~Object()
{
};

