/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
class GroupsObject;

using namespace std;

#ifndef GROUPS_OBJECT_H
#define GROUPS_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/itemObject.h"
#include "objects/groupObject.h"

#define DEFAULT_NEW_GROUP_NAME          "new group"

#define GROUP_TYPE_NAME_ROOT            "root"
#define GROUP_TYPE_NAME_MY_SELF         "myself"
#define GROUP_TYPE_NAME_SERVICES        "services"
#define GROUP_TYPE_NAME_NON_IN_LIST     "non in list"
#define GROUP_TYPE_NAME_ALL_ITEMS       "all items"

class GroupsObject
    :   public  Object
{
    public:
        GroupsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_groups"
        );
        virtual ~GroupsObject();

        // internal
        virtual GroupsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_groups"
        );
        virtual int32_t     autorun();
        virtual void        setAttribute(
            const string    &a_name,
            const string    &a_value
        );
        virtual string      getAttribute(const string &a_name);

        // generic
        enum GroupDelStrategy {
            GROUP_DEL_STRATEGY_0 = 0,
            GROUP_DEL_STRATEGY_1,
            GROUP_DEL_STRATEGY_MAX
        };

        enum {
            // don't change or update "config.xml.example" too
            GROUP_TYPE_ROOT             = 0,
            GROUP_TYPE_ALL_ITEMS        = 1,
            GROUP_TYPE_SERVICES         = 2,
            GROUP_TYPE_NON_IN_LIST      = 3,
            GROUP_TYPE_MY_SELF          = 4,
            // users groups start from here
            GROUP_TYPE_USER_DEFINED     = 100
        };

        // get
        GroupObject *       getRootGroup();
        GroupObject *       getGroupById(int a_group_id);
        GroupObject *       getGroupById(const string &a_group_id);
        GroupItemObject *   getContactFromGroup(
            int                     a_group_id,
            const ContactObject::Id &a_contact_id
        );

        // create
        GroupObject *       createGroup(
            const int32_t &a_group_id,
            const int32_t &a_parent_group_id = 0
        );
        GroupObject *       smartCreateGroup(
            int a_group_id = GROUP_TYPE_USER_DEFINED,
            int a_parent_group_id = 0
        );

        // add
        GroupItemObject *   addContactToGroup(
            int a_group_id,
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToGroup(
            int a_group_id,
            ContactObject   *
        );
        GroupItemObject *   addContactToMyselfGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToMyselfGroup(
            ContactObject *
        );
        GroupItemObject *   addContactToNonInListGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToNonInListGroup(
            ContactObject *
        );
        GroupItemObject *   addContactToRootGroup(
            const ContactObject::Id &
        );
        GroupItemObject *   addContactToRootGroup(
            ContactObject *
        );

        // del
        bool    isStrategySupported(GroupDelStrategy);
        void    getAllGroupsItems(
            vector<GroupItemObject *> &
        );

        int     delGroupStrategy_0(int);
        int     delGroupStrategy_1(int);
        int     delGroup(
            int                 a_group_id,
            GroupDelStrategy    a_strategy = GROUP_DEL_STRATEGY_1
        );
        void    delContactFromGroup(
            int                     a_group_id,
            const ContactObject::Id &
        );
        void    delContactFromGroup(
            int             a_group_id,
            ContactObject   *a_item
        );
        void    delContactFromAllItemsGroup(
            const ContactObject::Id &
        );
        void   delContactFromAllItemsGroup(
            ContactObject *
        );
};

#endif

