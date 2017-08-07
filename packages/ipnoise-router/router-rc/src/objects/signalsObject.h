/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class SignalsObject;

using namespace std;

#ifndef SIGNALS_OBJECT_H
#define SIGNALS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

class SignalsObject
    :   public Object
{
    public:
        SignalsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_signals"
        );
        virtual ~SignalsObject();

        // internal
        virtual SignalsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_signals"
        );

        // generic
};

#endif

