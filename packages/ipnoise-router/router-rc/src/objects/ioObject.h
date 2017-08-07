/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

class IoObject;

using namespace std;

#ifndef IO_OBJECT_H
#define IO_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

class IoObject
    :   public  Object
{
    public:
        IoObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );
        virtual ~IoObject();

        // internal
        virtual IoObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );

        // generic
        void addTxBytes(size_t bytes);
        void addRxBytes(size_t bytes);
};

#endif

