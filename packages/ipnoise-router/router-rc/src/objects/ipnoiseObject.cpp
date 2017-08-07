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
#include "objects/signalsObject.h"
#include "objects/slotsObject.h"
#include "objects/tmpObject.h"

IPNoiseObject * IPNoiseObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new IPNoiseObject(a_node, a_doc, a_tagname);
}

IPNoiseObject::IPNoiseObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

IPNoiseObject::~IPNoiseObject()
{
};

int32_t IPNoiseObject::autorun()
{
    int err = 0;

    // create container if not exist
    if (not isContainer()){
        setContainerVersion(0);
    }

    return err;
}

ServerObject * IPNoiseObject::getServerObject()
{
    ServerObject *server_obj = NULL;

    server_obj = (ServerObject *)getElementByTagName("ipn_server");
    if (not server_obj){
        server_obj = getDocument()->createElement<ServerObject>("ipn_server");
        appendChildSafe(server_obj);
    }
    return server_obj;
}

SignalsObject * IPNoiseObject::getSignalsObject()
{
    SignalsObject *signals_obj = NULL;

    signals_obj = (SignalsObject *)getElementByTagName("ipn_signals");
    if (!signals_obj){
        signals_obj = getDocument()->createElement<SignalsObject>("ipn_signals");
        // delete after exit
        signals_obj->setExpired("0");
        appendChildSafe(signals_obj);
    }
    return signals_obj;
}

SlotsObject * IPNoiseObject::getSlotsObject()
{
    SlotsObject *slots_obj = NULL;

    slots_obj = (SlotsObject *)getElementByTagName("ipn_slots");
    if (!slots_obj){
        slots_obj = getDocument()->createElement<SlotsObject>("ipn_slots");
        // delete after exit
        slots_obj->setExpired("0");
        appendChildSafe(slots_obj);
    }
    return slots_obj;
}

TmpObject * IPNoiseObject::getTmpObject()
{
    TmpObject *tmp_obj = NULL;

    tmp_obj = (TmpObject *)getElementByTagName("ipn_tmp");
    if (!tmp_obj){
        tmp_obj = getDocument()->createElement<TmpObject>("ipn_tmp");
        // delete after exit
        tmp_obj->setExpired("0");
        appendChildSafe(tmp_obj);
    }
    return tmp_obj;
}

