/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
#include "objects/object.h"

class GroupObject;

using namespace std;

#ifndef GROUP_OBJECT_H
#define GROUP_OBJECT_H

#include <ipnoise-common/log.h>
#include "objects/groupItemObject.h"

class DomDocument;

#include "objects/itemObject.h"
#include "objects/contactObject.h"

class GroupObject
    :   public Object
{
    public:
        GroupObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_group"
        );
        virtual ~GroupObject();

        // internal
        virtual GroupObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_group"
        );
        virtual void    expired();
        virtual void    setAttribute(
            const string &name,
            const string &value
        );
        virtual string  getAttribute(const string &name);

        // generic
        bool            isSystemGroup();
        bool            isRootGroup();
        void            setGroupName(const string name);
        string          getGroupName();
        void            setGroupId(const string id);
        void            setGroupId(int id);
        string          getGroupId();
        int             getGroupIdInt();

        void                getAllGroupsItems(
            vector<GroupItemObject *> &
        );
        GroupItemObject *   getGroupItem(
            const ContactObject::Id &a_contact_id
        );
        GroupItemObject *   addGroupItem(
            const ContactObject::Id &a_contact_id
        );
        GroupItemObject *   addGroupItem(
            ContactObject *
        );
        void                delGroupItem(
            const ContactObject::Id &a_contact_id
        );

        void            getGroupItems(vector<GroupItemObject *> &);
        void            getSubGroups(vector<GroupObject *> &);
};

#endif

