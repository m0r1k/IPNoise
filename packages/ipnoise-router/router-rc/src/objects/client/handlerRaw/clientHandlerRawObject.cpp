/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Mar 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/serverObject.h"

#include <ipnoise-common/utils.hpp>
#include "clientHandlerRawObject.h"

ClientHandlerRawObject * ClientHandlerRawObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ClientHandlerRawObject(a_node, a_doc, a_tagname);
}

ClientHandlerRawObject::ClientHandlerRawObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    : ClientHandler(a_node, a_doc, a_tagname)
{
};

ClientHandlerRawObject::~ClientHandlerRawObject()
{
};

int ClientHandlerRawObject::do_init()
{
    int err = 0;
    return err;
}

void ClientHandlerRawObject::closed_cb()
{
}

int ClientHandlerRawObject::read_cb(
    const string    &a_data,
    ReadState)      // rd_state
{
    int err = -1;

    vector<string>              cmds;
    vector<string>::iterator    cmds_it;

    PDEBUG_OBJ(this, 15,
        "[raw handler] was read: '%s'\n",
        a_data.c_str()
    );

    // store part of data
    m_buffer += a_data;

    // search packets
    cutIPNoisePackets(m_buffer, cmds);
    for (cmds_it = cmds.begin();
        cmds_it != cmds.end();
        cmds_it++)
    {
        const string    &cmd = *cmds_it;
        SkBuffObject    *skb = NULL;

        // process api command
        skb = getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        if (not skb){
            PERROR_OBJ(this, "Cannot allocate new skb\n");
            goto fail;
        }

        skb->setInContent(cmd.c_str());
        skb->setInDevName(getClient()->getRxDevName());
        skb->setInDevIndex(getClient()->getIno());

        getDocument()->emitSignalAndDelObj(
            "packet_incoming",
            skb
        );
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
};

