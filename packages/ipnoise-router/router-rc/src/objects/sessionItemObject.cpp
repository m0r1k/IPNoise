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

#include "objects/sessionItemObject.h"

SessionItemObject * SessionItemObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionItemObject(a_node, a_doc, a_tagname);
}

SessionItemObject::SessionItemObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   SessionObject(a_node, a_doc, a_tagname)
{
    setApi("ipnoise");
};

SessionItemObject::~SessionItemObject()
{
};

void SessionItemObject::registrySignals()
{
    getDocument()->registrySignal(
        "ipnoise_conn_up",
        "ipn_session_item"
    );
    getDocument()->registrySignal(
        "ipnoise_conn_down",
        "ipn_session_item"
    );
}

ItemObject * SessionItemObject::getItem()
{
    ItemObject *item = NULL;

    item = (ItemObject *)getParentNodeByTagName(
        "ipn_item"
    );

    return item;
}

int SessionItemObject::connect()
{
    NetClientObject     *client = NULL;
    ItemObject          *item   = NULL;
    int                 res     = 0;
    int                 err     = 0;

    // get item
    item = getItem();

    // check connection
    client = (NetClientObject *)getElementByXpath(
        "//ipn_net_client[@state=\"connecting\"]"
    );
    if (client){
        // already in progress
        goto out;
    }

    client = (NetClientObject *)getElementByXpath(
        "//ipn_net_client[@state=\"connected\"]"
    );
    if (client){
        // already in progress
        goto out;
    }

    // delete this conection
    if (client){
        DomDocument::deleteElement(client);
    }

    // create new net client
    client = getDocument()->createElement<NetClientObject>("ipn_net_client");
    appendChildSafe(client);

    // setup callbacks
    client->setPartialReadCb(
        SessionItemObject::outcomingPartialReadCb,
        (void *)this
    );
    client->setConnectedCb(
        SessionItemObject::outcomingConnectedCb,
        (void *)this
    );
    client->setConnectClosedCb(
        SessionItemObject::outcomingConnectClosedCb,
        (void *)this
    );

    res = client->connect(item->getHuid(), 2210);
    if (res){
        PERROR_OBJ(this, "Connect failed to: '%s' port: '%d'\n",
            item->getHuid().c_str(), 2210);
        goto fail;
    }

out:
    return err;

fail:
    goto out;
}

void SessionItemObject::outcomingPartialReadCb(
    NetClientObject         *client,
    size_t,                 // current_offset
    size_t                  current_length,
    void                    *ctx)
{
    SessionItemObject   *sess = (SessionItemObject *)ctx;
    ItemObject          *item = NULL;
    assert(sess);

    vector<string>              cmds;
    vector<string>::iterator    cmds_it;

    // update statistic
    if (current_length <= 0){
        goto out;
    }

    sess->addRxBytes(current_length);
    sess->m_client_buffer += client->content;

    // get item
    item = sess->getItem();

    cutIPNoisePackets(
        sess->m_client_buffer,
        cmds
    );
    for (cmds_it = cmds.begin();
        cmds_it != cmds.end();
        cmds_it++)
    {
        const string &cmd = *cmds_it;
        SkBuffObject *skb = NULL;

        // process api command
        skb = sess->getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        if (not skb){
            PERROR_OBJ(sess, "Cannot allocate new skb\n");
            goto out;
        }

        skb->setInContent(cmd);
        skb->setTo(item->getHuid());
        skb->setFrom(client->getDstAddr());
        skb->setInDevName(IPNOISE_DEV_NAME);
        skb->setInDevIndex(sess->getIno());

        sess->getDocument()->emitSignalAndDelObj(
            "packet_incoming",
            skb
        );
    }

out:
    return;
}

void SessionItemObject::outcomingConnectedCb(
    NetClientObject         *, // client
    size_t,                 // current_offset
    size_t,                 // current_length
    void                    *ctx)
{
    SessionItemObject *sess_item = (SessionItemObject *)ctx;
    assert(sess_item);

    sess_item->getDocument()->emitSignal(
        (const char *)"ipnoise_conn_up",
        sess_item
    );
}

void SessionItemObject::outcomingConnectClosedCb(
    NetClientObject         *client,
    size_t,                 // current_offset
    size_t,                 // current_length
    void                    *ctx)
{
    SessionItemObject *sess_item = (SessionItemObject *)ctx;
    assert(sess_item);

    // request to reopen session
    sess_item->setState("closed");

    // request to delete us
    client->setExpired("1");

    sess_item->getDocument()->emitSignal(
        (const char *)"ipnoise_conn_down",
        sess_item
    );
}

bool SessionItemObject::isConnected()
{
    bool                    ret                 = false;
    NetClientObject         *net_client         = NULL;
    AcceptedClientObject    *accepted_client    = NULL;

    net_client = (NetClientObject *)getElementByTagName(
        "ipn_net_client"
    );
    if (net_client){
        // for outcoming connections
        ret = net_client->isConnected();
        goto out;
    }

    accepted_client = (AcceptedClientObject *)getElementByTagName(
        "ipn_net_client"
    );
    if (accepted_client){
        // for incoming connections
        ret = accepted_client->isConnected();
        goto out;
    }

out:
    return ret;
}

