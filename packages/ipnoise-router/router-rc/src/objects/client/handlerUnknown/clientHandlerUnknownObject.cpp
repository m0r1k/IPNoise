/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/serverObject.h"
#include "clientHandlerUnknownObject.h"

ClientHandlerUnknownObject * ClientHandlerUnknownObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ClientHandlerUnknownObject(a_node, a_doc, a_tagname);
}

ClientHandlerUnknownObject::ClientHandlerUnknownObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   ClientHandler(a_node, a_doc, a_tagname)
{
};

ClientHandlerUnknownObject::~ClientHandlerUnknownObject()
{
};

int ClientHandlerUnknownObject::do_init()
{
    int err = 0;
    return err;
}

int ClientHandlerUnknownObject::read_cb(
    char    *buffer1,
    char    *,          // buffer2
    int,                // len
    ReadState)          // rd_state
{
    int err = 0;

    PDEBUG_OBJ(this, 15, "[unknown handler] was read: '%s'\n", buffer1);

    return err;
};

