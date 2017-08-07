/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "objects/usersObject.h"
#include "objects/userObject.h"
#include "objects/sessionUnknownObject.h"

SessionUnknownObject * SessionUnknownObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionUnknownObject(a_node, a_doc, a_tagname);
}

SessionUnknownObject::SessionUnknownObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   SessionObject(a_node, a_doc, a_tagname)
{
    setApi("client");
};

SessionUnknownObject::~SessionUnknownObject()
{
};


