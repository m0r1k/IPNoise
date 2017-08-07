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
#include "objects/clientObject.h"

#include "objects/sessionClientObject.h"

SessionClientObject * SessionClientObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionClientObject(a_node, a_doc, a_tagname);
}

SessionClientObject::SessionClientObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   SessionObject(a_node, a_doc, a_tagname)
{
    setApi("client");
};

SessionClientObject::~SessionClientObject()
{
};

void SessionClientObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "client_closed",
        "ipn_client"
    );
}

void SessionClientObject::slot(
    const string    &a_signal_name,
    DomElement      *a_object)
{
    PacketsObject *packets_obj = NULL;

    // get/create packets object
    packets_obj = getSessionsObject()->getCreatePacketsObject();

    if (    "client_closed" == a_signal_name
        &&  "ipn_client" == a_object->getTagName())
    {
        ClientObject *client = (ClientObject *)a_object;

        PWARN("MORIK OK OK OK, state: '%s'\n",
            client->getState().c_str()
        );

        // mark us as closed
        setState("closed");

        // request remove us
        setExpired("+1");
    }
}

void SessionClientObject::process_packet(
    SkBuffObject *a_skb)
{
    ServerObject *server = NULL;
    ClientObject *client = NULL;

    // search server
    server = (ServerObject *)getParentNodeByTagName("ipn_server");

    // search client
    client = a_skb->getClient();
    if (not client){
        PERROR("client was not found, skb:\n%s\n",
            a_skb->serialize(1).c_str()
        );
        goto fail;
    }

    a_skb->setInDevName(CLIENTS_DEV_NAME);
    a_skb->setInDevIndex(client->getIno());

    // do process
    server->process_packet(a_skb);

out:
    return;
fail:
    goto out;
}

ssize_t SessionClientObject::xmit(
    DomElement      *a_packet_data,
    const int32_t   &a_delete_after_send)
{
    ssize_t err = -1;

    // send packet
    err = xmitLocal(a_packet_data);
    if (err < ssize_t(0)){
        goto fail;
    }

out:
    if (a_delete_after_send){
        DomDocument::deleteElement(a_packet_data);
        a_packet_data = NULL;
    }
    return err;

fail:
    if (err >= ssize_t(0)){
        err = ssize_t(-1);
    }
    goto out;
}

