/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

class ItemsObject;

using namespace std;

#ifndef ITEMS_OBJECT_H
#define ITEMS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/itemObject.h"

class ItemsObject
    :   public  Object
{
    public:
        ItemsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_items"
        );
        virtual ~ItemsObject();

        // internal
        virtual ItemsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_items"
        );
        virtual void    registrySlots();
        virtual void    slot(
            const string    &a_signal_name,
            DomElement      *object
        );

        // generic
        ItemObject *    getItem(const string &huid);
        void            getItems(vector<ItemObject *> &a_res);
        ItemObject *    getCreateItem(const string &a_huid);
};

#endif

