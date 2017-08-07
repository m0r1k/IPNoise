/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class NeighCheckingObject;

using namespace std;

#ifndef NEIGH_CHECKING_OBJECT_H
#define NEIGH_CHECKING_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/neighObject.h"

class NeighCheckingObject
    :   public Object
{
    public:
        NeighCheckingObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh_checking"
        );
        ~NeighCheckingObject();

        // internal
        virtual NeighCheckingObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh_checking"
        );
        virtual void setAttribute(const string &name, const string &value);

        // generic
};

#endif

