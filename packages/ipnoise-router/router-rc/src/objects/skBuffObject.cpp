/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/skBuffObject.h"

SkBuffObject * SkBuffObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SkBuffObject(a_node, a_doc, a_tagname);
}

SkBuffObject::SkBuffObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    // don't save us
    setExpired("0");
};

SkBuffObject::~SkBuffObject()
{
};

int32_t SkBuffObject::autorun()
{
    int32_t err = 0;

    in_dom  = getDocument()->createElement("ipn_in_dom");
    appendChildSafe(in_dom);

    out_dom = getDocument()->createElement("ipn_out_dom");
    appendChildSafe(out_dom);

    return err;
}

void SkBuffObject::registrySignals()
{
    getDocument()->registrySignal(
        "packet_incoming",
        "ipn_skbuff"
    );
    getDocument()->registrySignal(
        "packet_outcoming",
        "ipn_skbuff"
    );
}

void SkBuffObject::delFromLL()
{
    delAttribute("from_ll");
}

string SkBuffObject::getFromLL()
{
    return getAttribute("from_ll");
}

string SkBuffObject::getInDevName()
{
    return getAttribute("indev_name");
}

void SkBuffObject::setInDevName(const string &a_dev_name)
{
    setAttributeSafe("indev_name", a_dev_name);
}

string SkBuffObject::getOutDevName()
{
    return getAttribute("outdev_name");
}

void SkBuffObject::setOutDevName(const string &a_dev_name)
{
    setAttributeSafe("outdev_name", a_dev_name);
}

string SkBuffObject::getInDevIndex()
{
    return getAttribute("indev_index");
}

void SkBuffObject::setInDevIndex(const string &a_dev_index)
{
    setAttributeSafe("indev_index", a_dev_index);
}

string SkBuffObject::getFrom()
{
    return getAttribute("from");
}

void SkBuffObject::setFrom(const string &a_from)
{
    setAttributeSafe("from", a_from);
}

string SkBuffObject::getTo()
{
    return getAttribute("to");
}

void SkBuffObject::setTo(const string &a_to)
{
    setAttributeSafe("to", a_to);
}

string SkBuffObject::getToLL()
{
    return getAttribute("to_ll");
}

void SkBuffObject::setToLL(const string &a_to_ll)
{
    setAttributeSafe("to_ll", a_to_ll);
}

string SkBuffObject::toString(const string &descr)
{
    string ret          = "";
    string indom        = "";
    string outdom       = "";
    DomElement *element = NULL;
    string attr;

    if (not descr.empty()){
        ret += descr;
    } else {
        ret += "Socket buffer dump below\n";
    }

    ret += "--------------- SKB --------------- {\n";
//    ret += "\n";
//    ret += serialize(1);
//    ret += "\n";
    // ---------------------- IN ---------------------- {
    ret += "----- IN -----\n";
    // from
    ret += "From: ";
    attr = getAttribute("from");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // to
    ret += "To: ";
    attr = getAttribute("to");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // from (ll address)
    ret += "From (ll address): ";
    attr = getAttribute("from_ll");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // dev name
    ret += "Input device name: '" + getInDevName() + "'\n";
    // dev index
    ret += "Input device index: '" + getInDevIndex() + "'\n";
    element = getInDom();
    ret += "Data:\n";
    if (element){
        indom = element->serialize(1);
    }
    ret += indom;
    ret += "\n";
    // ---------------------- IN ---------------------- }

    // delimeter
    ret += "\n";

    // ---------------------- OUT ---------------------- {

    ret += "----- OUT -----\n";
    element = getOutDom();
    // from
    ret += "From: ";
    attr = getAttribute("from");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // to
    ret += "To: ";
    attr = getAttribute("to");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // from (ll address)
    ret += "To (ll address): ";
    attr = getAttribute("to_ll");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // dev name
    ret += "Output device name: ";
    attr = getAttribute("outdev_name");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    // dev index
    ret += "Output device index: ";
    attr = getAttribute("outdev_index");
    if (not attr.empty()){ ret += attr; }
    ret += "\n";
    ret += "Data:\n";
    if (element){
        outdom = element->serialize(1);
    }
    ret += outdom;
    ret += "\n";

    // ---------------------- OUT ---------------------- }

    ret += "--------------- SKB --------------- }\n";

    return ret;
}

void SkBuffObject::dump2stderr(const string &descr)
{
    string res = toString(descr);
    PWARN("%s\n", res.c_str());
}

DomElement *SkBuffObject::getInDom(
    int delete_if_exist     __attribute__ ((unused)))
{
    return in_dom;
}

DomElement *SkBuffObject::getOutDom(
    int delete_if_exist     __attribute__ ((unused)))
{
    return out_dom;
}

void SkBuffObject::prepare()
{
    char buffer[1024];
    vector <DomElement *> items;
    DomElement *item = NULL;
    int i;

    // remove attributes
    snprintf(buffer, sizeof(buffer), "//*");

    items.clear();
    in_dom->getElementsByXpath(buffer, items);
    for (i = 0; i < (int)items.size(); i++){
        item = items[i];
        item->delAttributeSafe("_ino");
    }

    items.clear();
    out_dom->getElementsByXpath(buffer, items);
    for (i = 0; i < (int)items.size(); i++){
        item = items[i];
        item->delAttributeSafe("_ino");
    }
}

void SkBuffObject::setInContent(const string &data)
{
    DomElement *element = NULL;

    // remove childs
    DomDocument::deleteChildsSafe(in_dom);

    // create new node
    element = getDocument()->createElementFromContent(data);
    if (element){
        in_dom->appendChildSafe(element);
    } else {
        PERROR_OBJ(this, "createElementFromContent failed for: '%s'\n",
            data.c_str());
        goto fail;
    }

out:
    return;

fail:
    goto out;
}

void SkBuffObject::setInContent(string &data)
{
    setInContent(data.c_str());
}

ClientObject * SkBuffObject::getClient()
{
    ClientObject    *client = NULL;
    string          client_ino;

    if (CLIENTS_DEV_NAME != getInDevName()){
        goto out;
    }

    client_ino = getInDevIndex();
    if (not client_ino.size()){
        goto out;
    }

    client = (ClientObject *)getDocument()->getElementByIno(
        client_ino
    );

out:
    return client;
}


