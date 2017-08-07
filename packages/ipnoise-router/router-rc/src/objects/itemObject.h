/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

using namespace std;

class ItemObject;

#ifndef ITEM_OBJECT_H
#define ITEM_OBJECT_H

#include <libxml/tree.h>
#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

class DomDocument;
class ContactObject;

#include "objects/neighsObject.h"
#include "objects/sessionsObject.h"
#include "objects/userObject.h"
#include "objects/contactAndItem.h"
#include "objects/packetObject.h"

class ItemObject
    :   public ContactAndItem
{
    public:
        ItemObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_item"
        );
        virtual             ~ItemObject();

        // internal
        virtual ItemObject *        create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_item"
        );
        virtual void        shutdown();
        virtual void        expired();
        virtual int32_t     autorun();
        virtual void        registrySignals();

        // generic
        virtual void                changed();
        SessionsObject          *   getCreateSessionsObject();
        PacketObject            *   queue(const string &);
        PacketObject            *   queue(DomElement *);
        ContactObject           *   getSelfContact();
        bool                        isSelfItem();
        bool                        isOnline();
        string                      isOnlineStr();
        void                        setOnline(bool);
        void                        requestUserInfo();

        // huid
        void                removeFromGroups();
        string              getHuid();
        void                setHuid(const string &huid);

        // api
        virtual string  getAPIStatus();
        virtual void    setAPIStatus(const string &);

        virtual string  getAPINickName();
        virtual void    setAPINickName(const string &);

        virtual string  getAPIIcon();
        virtual void    setAPIIcon(const string &);

        virtual string  getAPIRealNickName();
        virtual void    setAPIRealNickName(const string &);

        virtual string  getAPIRealIcon();
        virtual void    setAPIRealIcon(const string &);

        virtual void    getAPIInfo(string  &);

        // neighs
        void            getNeighs(vector<NeighObject *> &);
        void            getOnlineNeighs(vector<NeighObject *> &);
        NeighObject *   addNeigh(
            const string &a_dev,
            const string &a_lladdr,
            const string &a_metric = "1"
        );
};

#endif

