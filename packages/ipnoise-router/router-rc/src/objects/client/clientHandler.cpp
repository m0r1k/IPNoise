/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/serverObject.h"
#include "clientHandler.h"

ClientHandler::ClientHandler(
    xmlNodePtr      _node,
    DomDocument     *_doc,
    const string    &_tagname)
    :   IoObject(_node, _doc, _tagname)
{
};

ClientHandler::~ClientHandler()
{
};

ClientObject * ClientHandler::getClient()
{
    ClientObject *client = NULL;

    client = (ClientObject *)getParentNode();
    return client;
}

int ClientHandler::do_init()
{
    int err = -1;
    PERROR_OBJ(this, "method not implemented\n");
    return err;
}

int ClientHandler::read_cb(
    const string &, // a_data
    ReadState)      // rd_state
{
    int err = 0;
    return err;
}

void ClientHandler::closed_cb()
{
}

// --------------------------- write ---------------------------

int ClientHandler::write(const string &buffer)
{
    return getClient()->send(buffer);
};

// --------------------------- send_answer ---------------------------

int ClientHandler::send_answer(const string &buffer)
{
    return getClient()->send_answer(buffer);
}

