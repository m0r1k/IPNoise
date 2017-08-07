/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <map>
#include <string>

using namespace std;

class Object;

#ifndef OBJECT_H
#define OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "libxml2/domElement.h"

class DomDocument;

class Object
    :   public DomElement
{
    public:
        Object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );
        virtual ~Object();

        // internal
        virtual Object * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );

        // generic
};

#endif

