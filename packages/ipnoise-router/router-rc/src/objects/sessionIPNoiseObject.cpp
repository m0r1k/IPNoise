/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "objects/usersObject.h"
#include "objects/userObject.h"

#include "objects/sessionIPNoiseObject.h"

SessionIPNoiseObject * SessionIPNoiseObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionIPNoiseObject(a_node, a_doc, a_tagname);
}

SessionIPNoiseObject::SessionIPNoiseObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   SessionObject(a_node, a_doc, a_tagname)
{
    setApi("ipnoise");
};

SessionIPNoiseObject::~SessionIPNoiseObject()
{
};

int32_t SessionIPNoiseObject::autorun()
{
    int32_t err         = 0;
    int32_t priveleges  = 0;

    priveleges = getDocument()->getPriveleges();
    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element,"
            " document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

ItemObject * SessionIPNoiseObject::getItem()
{
    ItemObject *item = NULL;

    item = (ItemObject *)getParentNodeByTagName(
        "ipn_item"
    );

    return item;
}

void SessionIPNoiseObject::process_packet(
    SkBuffObject *a_skb)
{
    ServerObject *server = NULL;

    // search server
    server = (ServerObject *)getParentNodeByTagName("ipn_server");

    a_skb->setInDevName(IPNOISE_DEV_NAME);
    //a_skb->setInDevIndex(client->getIno());

    // do process
    server->process_packet(a_skb);
}

// --------------------------------------------------------

ssize_t SessionIPNoiseObject::xmit(
    DomElement      *a_el,
    const int32_t   &a_delete_after_send)
{
    ssize_t wrote = -1;

    UserObject  *user       = getOwner();
    ItemObject  *item       = getItem();
    string      src_huid;
    string      dst_huid;

    // get user
    user = getOwner();
    if (user){
        src_huid = user->getHuid();
    }
    if (src_huid.empty()){
        PERROR("Cannot get src huid\n");
        goto fail;
    }

    // get item
    item = getItem();
    if (item){
        dst_huid = item->getHuid();
    }
    if (dst_huid.empty()){
        PERROR("Cannot get dst huid\n");
        goto fail;
    }

    PDEBUG(20, "SessionIPNoiseObject::xmit"
        " dst_huid: '%s',"
        " src_huid: '%s':"
        "\n'%s'\n",
        dst_huid.c_str(),
        src_huid.c_str(),
        a_el->serialize(1).c_str()
    );

    PWARN("[ send ] session: '%s', seq: '%s', ack: '%s'\n"
        "%s\n",
        getSessId().c_str(),
        a_el->getAttribute("seq").c_str(),
        a_el->getAttribute("ack").c_str(),
        a_el->serialize(1).c_str()
    );

    wrote = ServerObject::sendMsg(
        a_el,
        dst_huid,
        src_huid,
        a_delete_after_send
    );

out:
    return wrote;
fail:
    if (wrote >= ssize_t(0)){
        wrote = ssize_t(-1);
    }
    goto out;
}

int32_t SessionIPNoiseObject::connect()
{
    int32_t err = 0;
    string  dst_sessid = getDstSessId();

    if ("init" != getState()){
        // already in progress
        goto out;
    }

    // update state
    setState("opening");

    // setup timeout for open session
    setExpired(SESSION_INIT_TIMEOUT);

    // create request
    {
        DomDocument *doc = getDocument();
        DomElement  *el  = NULL;

        el = doc->createElement("ipnoise");
        el->setAttributeSafe("flags",       "init");
        el->setAttributeSafe("src_sessid",  getSrcSessId());
        el->setAttributeSafe("dst_sessid",  getDstSessId());
        el->setAttributeHexSafe("seq",      getSeqUint());

        // element 'el' will be deleted after send
        xmit(el);

        // update seq
        setSeq(getSeqUint() + 1);
    }

out:
    return err;
}

