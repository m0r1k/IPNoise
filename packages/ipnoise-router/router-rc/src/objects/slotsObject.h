/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class SlotsObject;

using namespace std;

#ifndef SLOTS_OBJECT_H
#define SLOTS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

class SlotsObject
    :   public Object
{
    public:
        SlotsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_slots"
        );
        virtual ~SlotsObject();

        // internal
        virtual SlotsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_slots"
        );

        // generic
};

#endif

