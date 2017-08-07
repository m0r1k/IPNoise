/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class UsersObject;

using namespace std;

#ifndef USERS_OBJECT_H
#define USERS_OBJECT_H

#define NEW_USER_NICKNAME "new user"

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/userObject.h"

class UsersObject
    :   public Object
{
    public:
        UsersObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_users"
        );
        virtual ~UsersObject();

        // internal
        virtual UsersObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_users"
        );
        virtual void registrySignals();

        // generic
        void            getUsers(vector<UserObject *> &a_users_out);
        UserObject  *   getAnyUser();
        string          getAnyUserHuid();
        UserObject  *   getUserByHuid(const string &a_huid);
        UserObject  *   getUserByLoginPassword(
            const string    &a_login,
            const string    &a_password
        );
        UserObject  *   createUserByHuid(
            const string    &a_huid,
            const string    &a_real_nickname = NEW_USER_NICKNAME
        );
        UserObject  *   createUser(
            const string    &a_login,
            const string    &a_password
        );
};

#endif

