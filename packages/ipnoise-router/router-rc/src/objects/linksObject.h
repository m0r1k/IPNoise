/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

class LinksObject;

using namespace std;

#ifndef LINKS_OBJECT_H
#define LINKS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/linkObject.h"

class LinksObject
    :   public  Object
{
    public:
        LinksObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_links"
        );
        virtual ~LinksObject();

        // internal
        virtual LinksObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_links"
        );
        virtual int32_t autorun();

        // generic
        LinkObject * getLinkByName(const string &a_name);
        LinkObject * getCreateLinkByName(
            const string    &a_ifname,
            const int       &a_ifindex
        );

};

#endif

