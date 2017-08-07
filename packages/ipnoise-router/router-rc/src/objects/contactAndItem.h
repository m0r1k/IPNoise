/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

using namespace std;

class ContactAndItem;

#ifndef CONTACT_AND_ITEM_H
#define CONTACT_AND_ITEM_H

#include <libxml/tree.h>
class DomDocument;
class UserObject;

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>
#include "objects/object.h"

class ContactAndItem
    :   public Object
{
    public:
        ContactAndItem(
            xmlNodePtr      _node,
            DomDocument     *_doc,
            const string    &_tagname
        );
        virtual ~ContactAndItem();

        virtual void changed() = 0;

        // api
        virtual string  getAPIStatus()                      = 0;
        virtual void    setAPIStatus(const string &)        = 0;

        virtual string  getAPINickName()                    = 0;
        virtual void    setAPINickName(const string &)      = 0;

        virtual string  getAPIIcon()                        = 0;
        virtual void    setAPIIcon(const string &)          = 0;

        virtual string  getAPIRealNickName()                = 0;
        virtual void    setAPIRealNickName(const string &)  = 0;

        virtual string  getAPIRealIcon()                    = 0;
        virtual void    setAPIRealIcon(const string &)      = 0;

        virtual void    getAPIInfo(string  &)               = 0;
};

#endif

