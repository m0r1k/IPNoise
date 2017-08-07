/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/linksObject.h"

LinksObject * LinksObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new LinksObject(a_node, a_doc, a_tagname);
}

LinksObject::LinksObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

LinksObject::~LinksObject()
{
};

int32_t LinksObject::autorun()
{
    int err = 0;
    LinkObject *link = NULL;

    // add 'lo' link
    link = getDocument()->createElement<LinkObject>("ipn_link");
    link->setupLink("lo", -1);
    appendChildSafe(link);

    // add 'client' link
    link = getDocument()->createElement<LinkObject>("ipn_link");
    link->setupLink("client", -1);
    appendChildSafe(link);

    // add 'ipnoise' link
    link = getDocument()->createElement<LinkObject>("ipn_link");
    link->setupLink("ipnoise", -1);
    appendChildSafe(link);

    return err;
}

LinkObject * LinksObject::getLinkByName(const string &a_name)
{
    LinkObject *link = NULL;

    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

    getElementsByTagName("ipn_link", items);
    for (items_i = items.begin();
        items_i != items.end();
        items_i++)
    {
        LinkObject *cur_link = (LinkObject *)(*items_i);
        if (cur_link->getName() == a_name){
            link = cur_link;
            break;
        }
    }

    return link;
}

LinkObject * LinksObject::getCreateLinkByName(
    const string    &a_ifname,
    const int       &a_ifindex)
{
    LinkObject *link = NULL;

    link = getLinkByName(a_ifname);
    if (!link){
        link = getDocument()
            ->createElement<LinkObject>("ipn_link");
        link->setupLink(a_ifname, a_ifindex);
        appendChildSafe(link);
    }

    return link;
}

