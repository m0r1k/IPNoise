/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include "libxml2/domDocument.h"
#include "objects/acceptedClientObject.h"

AcceptedClientObject * AcceptedClientObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new AcceptedClientObject(a_node, a_doc, a_tagname);
}

AcceptedClientObject::AcceptedClientObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   IoObject(a_node, a_doc, a_tagname)
{
    // don't save us after exit
    setExpired("0");
};

AcceptedClientObject::~AcceptedClientObject()
{
};

SessionItemObject * AcceptedClientObject::getSessionItemObject()
{
    SessionItemObject *session_item = NULL;
    session_item = (SessionItemObject *)getParentNode();
    assert ("ipn_session_item" == session_item->getTagName());
    return session_item;
}

int AcceptedClientObject::write(
    const string &a_data)
{
    int ret = 0;
    int res = 0;

    vector<ClientObject *>              clients;
    vector<ClientObject *>::iterator    clients_it;

    getClients(clients);

    for (clients_it = clients.begin();
        clients_it != clients.end();
        clients_it++)
    {
        res = (*clients_it)->write(a_data);
        if (res > 0){
            addTxBytes(res);
            ret += res;
        }
    }

    return ret;
}

void AcceptedClientObject::getClients(vector<ClientObject *> &clients)
{
    char buffer[1024] = { 0x00 };
    SessionItemObject * session_item = NULL;
    string sessid;

    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    session_item    = getSessionItemObject();
    sessid          = session_item->getSessId();

    if (sessid.empty()){
        goto out;
    }

    snprintf(buffer, sizeof(buffer),
        "//ipn_client[@sessid=\"%s\"]",
        sessid.c_str()
    );
    res.clear();
    getDocument()->getRootElement()->getElementsByXpath(
        buffer,
        res
    );

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        clients.push_back((ClientObject *)(*res_it));
    }

out:
    return;
}

bool AcceptedClientObject::isConnected()
{
    bool ret = false;
    vector<ClientObject *> clients;

    getClients(clients);

    if (not clients.empty()){
        ret = true;
    }

    return ret;
}

