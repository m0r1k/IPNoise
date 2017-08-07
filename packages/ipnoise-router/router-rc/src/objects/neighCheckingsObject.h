/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

class NeighCheckingsObject;

using namespace std;

#ifndef NEIGH_CHECKINGS_OBJECT_H
#define NEIGH_CHECKINGS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

class NeighCheckingsObject
    :   public Object
{
    public:
        NeighCheckingsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh_checkings"
        );
        ~NeighCheckingsObject();

        // internal
        virtual NeighCheckingsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh_checkings"
        );

        // generic
};

#endif

