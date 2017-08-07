/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/packetReceiversObject.h"

PacketReceiversObject * PacketReceiversObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new PacketReceiversObject(a_node, a_doc, a_tagname);
}

PacketReceiversObject::PacketReceiversObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

PacketReceiversObject::~PacketReceiversObject()
{
};

void PacketReceiversObject::shutdown()
{
}

int32_t PacketReceiversObject::autorun()
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

void PacketReceiversObject::timer()
{
}

PacketReceiverObject * PacketReceiversObject::getReceiver(
    const string &a_sessid)
{
    char                    buffer[512] = { 0x00 };
    PacketReceiverObject    *receiver   = NULL;

    snprintf(buffer, sizeof(buffer),
        "ipn_packet_receiver[@sessid=\"%s\"]",
        a_sessid.c_str()
    );
    receiver = (PacketReceiverObject *)getElementByXpath(buffer);
    return receiver;
}

void PacketReceiversObject::getReceivers(
    vector<PacketReceiverObject *>  &a_out)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    getElementsByTagName("ipn_packet_receiver", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        PacketReceiverObject *receiver = NULL;
        receiver = (PacketReceiverObject *)*res_it;
        a_out.push_back(receiver);
    }
}

PacketReceiverObject * PacketReceiversObject::getCreateReceiver(
    const string &a_sessid)
{
    PacketReceiverObject *receiver = NULL;

    receiver = getReceiver(a_sessid);
    if (receiver){
        // already exist
        goto out;
    }

    receiver = getDocument()->createElement<PacketReceiverObject>(
        "ipn_packet_receiver"
    );
    appendChildSafe(receiver);
    receiver->setSessId(a_sessid);

out:
    return receiver;
}

