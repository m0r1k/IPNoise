/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class TmpObject;

using namespace std;

#ifndef TMP_OBJECT_HPP
#define TMP_OBJECT_HPP

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

class TmpObject
    :   public Object
{
    public:
        TmpObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_tmp"
        );
        virtual ~TmpObject();

        // internal
        virtual TmpObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_tmp"
        );

        // generic
};

#endif

