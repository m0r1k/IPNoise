#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/user/main.hpp"
//#include "core/object/god/main.hpp"
#include "core/object/map/main.hpp"
//#include "core/object/param/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/users/main.hpp"

ObjectUsers::ObjectUsers(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectUsers::~ObjectUsers()
{
};

void ObjectUsers::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.users");
}

string ObjectUsers::getType()
{
    return ObjectUsers::_getType();
}

void ObjectUsers::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    ObjectIdSptr    prop_id;

//    PROP_TYPE( prop_type,   "core.object.users");
//    PROP_ID(   prop_id,     "static.users");

//    INIT_PROP(this, Type, prop_type);
//    INIT_PROP(this, Id,   prop_id);
}

void ObjectUsers::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectUsers::do_autorun()
{
    int32_t err = 0;

    // create god if need
    //getCreateNeigh<ObjectGod>(
    //    "core.object.god"
    //);

    return err;
}

void ObjectUsers::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectUsers::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- user ----------------

ObjectUserSptr ObjectUsers::create_user()
{
    ObjectUserSptr  user;
    PFATAL("FIXME\n");
/*
    ObjectUsersSptr users;
    users = ThreadDb::get()->getObjectById<ObjectUsers>(
        PROP_ID(PROP_USERS_ID)
    );
    user = users->createUser();
*/
    return user;
}

/*
ObjectUserSptr ObjectUsers::createUser()
{
    ObjectUserSptr user;

    user = createNeigh<ObjectUser>(
        "core.object.user"
    );

    return user;
}
*/

ObjectUserSptr ObjectUsers::getUserByLogin(
    const string    &a_login,
    const int32_t   &a_db_lock)
{
    ObjectUserSptr      user;
    ObjectVectorSptr    users;
    ObjectVectorIt      users_it;
    ObjectUniqueSptr    object_types;

    if (!a_login.size()){
        goto out;
    }

    PROP_VECTOR(users);
    PROP_UNIQUE(object_types);
    object_types->add("core.object.user");

    // TODO optimize me
    // 20140609 morik@
    getNeighs(
        users,
        "",             // name
        0,              // get removed
        a_db_lock,      // db_lock
        object_types
    );
    for (users_it = users->begin();
        users_it != users->end();
        users_it++)
    {
        ObjectUserSptr  cur_user;
        string          login;

        cur_user = dynamic_pointer_cast<ObjectUser>(*users_it);
        login    = cur_user->getName()->toString();

        if (a_login == login){
            user = cur_user;
            break;
        }
    }

out:
    return user;
}

ObjectUserSptr ObjectUsers::authUser(
    const string &a_login,
    const string &a_password)
{
    ObjectUserSptr  ret, user;

    if (a_login.empty()){
        PFATAL("missing argument: 'a_login'\n");
    }

    if (a_password.empty()){
        PFATAL("missing argument: 'a_password'\n");
    }

    user = getUserByLogin(a_login);
    if (user){
        string password;
        password = user->getPassword()->toString();
        if (password == a_password){
            ret = user;
        }
    }

    return ret;
}

ObjectUserSptr ObjectUsers::auth_user(
    const string &a_login,
    const string &a_password)
{
    ObjectUserSptr  user;
    PFATAL("FIXME\n");
/*
    ObjectUsersSptr users;

    users = ThreadDb::get()->getObjectById<ObjectUsers>(
        PROP_ID(PROP_USERS_ID)
    );

    user = users->authUser(
        a_login,
        a_password
    );
*/
    return user;
}

// ---------------- module ----------------

string ObjectUsers::_getType()
{
    return "core.object.users";
}

string ObjectUsers::_getTypePerl()
{
    return "core::object::users::main";
}

int32_t ObjectUsers::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectUsers::_destroy()
{
}

ObjectSptr ObjectUsers::_object_create()
{
    ObjectSptr object;
    _OBJECT_USERS(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectUsers::_getType,
    .type_perl      = ObjectUsers::_getTypePerl,
    .init           = ObjectUsers::_init,
    .destroy        = ObjectUsers::_destroy,
    .object_create  = ObjectUsers::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

