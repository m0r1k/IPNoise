#include "class.hpp"
DEFINE_CLASS(ObjectUsers);

#ifndef OBJECT_USERS_HPP
#define OBJECT_USERS_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

#include <mongo/client/dbclient.h>
#include <ipnoise-common/log_common.h>
#include "utils.hpp"

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectUser);

using namespace std;

#define OBJECT_USERS_ID "core.users"

#define _OBJECT_USERS(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectUsers,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_USERS(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectUsers,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_USERS(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectUsers, ##__VA_ARGS__)

#define PROP_USERS(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectUsers, ##__VA_ARGS__)

class ObjectUsers
    :   public  Object
{
    public:
        ObjectUsers(const Object::CreateFlags &);
        virtual ~ObjectUsers();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
//        ObjectUserSptr  createUser();
//        void            getUsers(
//            vector<ObjectUserSptr>  &a_out,
//            const int32_t           &a_db_lock
//        );
        ObjectUserSptr  getUserByLogin(
            const string    &a_login,
            const int32_t   &a_db_lock  = 0
        );
        ObjectUserSptr  authUser(
            const string    &a_login,
            const string    &a_password = ""
        );

        // static
        static ObjectUserSptr       create_user();
        static ObjectUserSptr       auth_user(
            const string &a_login,
            const string &a_password = ""
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

