#include "core/object/object/__BEGIN__.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/objectLink/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/transaction/main.hpp"
#include "core/object/uint32/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"
#include "db.hpp"

#include "core/object/context/main.hpp"

ObjectContext::ObjectContext(
    const Object::CreateFlags &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectContext::~ObjectContext()
{
}

void ObjectContext::do_init(
    const Object::CreateFlags &a_flags)
{
    ObjectInt32Sptr         prop_fd;
    ObjectTransactionSptr   prop_transaction;

    PROP_INT32(prop_fd, -1);
    PROP_TRANSACTION(prop_transaction);

    PROP_MAP(m_val);
    m_val->add("fd",            prop_fd);
    m_val->add("transaction",   prop_transaction);
}

string ObjectContext::getType()
{
    return ObjectString::_getType();
}

void ObjectContext::do_init_props()
{
    Object::do_init_props();
}

void ObjectContext::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectContext::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectContext::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectContext::toPerl()
{
    return m_val->toPerl();
}

string ObjectContext::toString()
{
    return m_val->toString();
}

ObjectSessionSptr ObjectContext::getSession()
{
    ObjectSessionSptr session;
    session = dynamic_pointer_cast<ObjectSession>(
        m_val->get("session")
    );
    return session;
}

string ObjectContext::getSessionId()
{
    string              ret;
    ObjectSessionSptr   session;

    session = getSession();
    if (session){
        ret = session->getId();
    }

    return ret;
}

void ObjectContext::setSession(
    ObjectSessionSptr a_session)
{
    m_val->add("session", a_session);
}

ObjectUserSptr ObjectContext::getUser()
{
    ObjectUserSptr user;
    user = dynamic_pointer_cast<ObjectUser>(
        m_val->get("user")
    );
    return user;
}

void ObjectContext::setUser(
    ObjectUserSptr  a_user)
{
    m_val->add("user", a_user);
}

void ObjectContext::setFd(
    const int32_t &a_fd)
{
    ObjectInt32Sptr prop_fd;
    PROP_INT32(prop_fd, a_fd);
    m_val->add("fd", a_fd);
}

int32_t ObjectContext::getFd()
{
    ObjectInt32Sptr prop_fd;
    int32_t         fd = -1;

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        m_val->get("fd")
    );
    if (prop_fd){
        fd = prop_fd->getVal();
    }

    return fd;
}

ObjectTransactionSptr ObjectContext::getTransaction()
{
    ObjectTransactionSptr   transaction;

    transaction = dynamic_pointer_cast<ObjectTransaction>(
        m_val->get("transaction")
    );

    return transaction;
}

void ObjectContext::transactionCommit()
{
    ObjectTransactionSptr   transaction;

    transaction = getTransaction();
    if (transaction){
        transaction->commit();
    }
}

void ObjectContext::transactionAdd(
    ObjectSptr  a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectTransactionSptr   transaction;
    string                  object_id;
    string                  object_type;

    if (!a_object){
        PFATAL("missing argument 'object'\n");
    }

    transaction = getTransaction();
    object_id   = a_object->getId();
    object_type = a_object->getType();

    PDEBUG(50, "'%s' (%s)\n",
        object_id.c_str(),
        object_type.c_str()
    );

    transaction->add(a_object);
}

void ObjectContext::transactionAddForSave(
    ObjectSptr  a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectTransactionSptr   transaction;
    string                  lock;
    string                  object_id;
    string                  object_type;

    if (!a_object){
        PFATAL("missing argument 'a_object'\n");
    }

    transaction = getTransaction();
    lock        = a_object->getLock()->toString();
    object_id   = a_object->getId();
    object_type = a_object->getType();

    if (    !a_object->isNew()
        &&  lock.empty())
    {
        PFATAL("attempt add for save unlocked object:\n"
            "  object_id:   '%s'\n"
            "  object_type: '%s'\n",
            object_id.c_str(),
            object_type.c_str()
        );
    }

    PDEBUG(50, "'%s' (%s)\n",
        object_id.c_str(),
        object_type.c_str()
    );

    transaction->addForSave(a_object);
}

/*
void ObjectContext::serializeToObjectMap(
    ObjectMapSptr a_out)
{
    ObjectMapSptr info_user;
    ObjectMapSptr info_session;

    OBJECT_MAP(info_user);
    OBJECT_MAP(info_session);

    m_user->serializeToObjectMap(info_user);
    m_session->serializeToObjectMap(info_session);

    a_out->add("user",      info_user);
    a_out->add("session",   info_session);
}
*/

string ObjectContext::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
/*
    string          ret;
    ObjectMapSptr   map;
    OBJECT_MAP(map);

    serializeToObjectMap(map);
    ret = map->serialize(a_delim);
    return ret;
*/
}

ObjectSptr ObjectContext::_getObjectById(
    const string    &a_id,
    const string    &a_type,
    const int32_t   &a_get_removed,
    const int32_t   &a_db_lock,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr              object;
    DbSptr                  db;
    ObjectTransactionSptr   transaction;

    if (a_id.empty()){
        PFATAL("missing argument 'a_id'\n");
    }

    db          = Thread::getCurThreadDb();
    transaction = getTransaction();

    // search in transaction
    object = transaction->_getObjectById(
        a_id,
        a_type,
        a_get_removed,
        a_do_autorun
    );
    if (object){
        goto out;
    }

    // search in db
    object = db->getObjectById(
        a_id,
        a_type,
        a_get_removed,
        a_do_autorun
    );
    if (object){
        transaction->add(object);
        goto out;
    }

out:
    return object;
}

ObjectSptr ObjectContext::_getCreateObjectById(
    const string    &a_id,
    const string    &a_type,
    const int32_t   &a_get_removed,
    const int32_t   &a_db_lock,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr object;
    object = _getObjectById(
        a_id,
        a_type,
        a_get_removed,
        a_do_autorun
    );
    if (!object){
        object = Object::createObject(
            a_type,
            a_do_autorun,
            a_id
        );
    }
    return object;
}

void ObjectContext::remove(
    ObjectSptr      a_object,
    const int32_t   a_force)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    DbSptr              db;
    string              object_id;
    ObjectTimeSptr      remove_time;
    ObjectUniqueSptr    link_types;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    // check that object locked in DB
    a_object->checkDbLock();

    db        = Thread::getCurThreadDb();
    object_id = a_object->getId();

    if (object_id.empty()){
        PFATAL("empty object_id\n");
    }

    PTIMING(0.010f, "request remove '%s', a_force: '%d'\n",
        object_id.c_str(),
        a_force
    );

    if (!a_force){
        // just mark as deleted
        PROP_TIME(remove_time, -0.1f);

        // TODO link with current user's trash
        // 20140609 morik@

        //ObjectTrashSptr trash;
        //trash = ThreadDb::get()->getCreateTrashObject();

        // addNeigh(trash);

    } else {
        // request remove from DB (remove_time > 0)
        PROP_TIME(remove_time, 0.1f);
    }

    // mark as deleted
    a_object->setTimeRemove(remove_time);
    transactionAddForSave(a_object);

    // remove/unlink from neigh
    {
        ObjectVectorSptr    links;
        ObjectVectorIt      links_it;

        PROP_VECTOR(links);

        a_object->getLinks(
            links,
            "",         // name
            0,          // get removed
            1           // db_lock
        );
        for (links_it = links->begin();
            links_it != links->end();
            links_it++)
        {
            ObjectSptr  link        = *links_it;
            string      link_id     = link->getId();
            string      link_type   = link->getType();
            string      tmp         = "core.object.link";

            if (link_type.substr(0, tmp.size()) == tmp){
                // it's neigh - link, remove it
                Object::remove(link, a_force);
            } else {
                // just remove us from links info
                Object::removeLink(link, object_id);
            }
        }
    }

    return;
}

/*
ObjectSptr ObjectContext::_getCurObject()
{
    ObjectSptr cur_object;
    cur_object = m_val->get("cur_object");
    return cur_object;
}

void ObjectContext::_setCurObject(
    ObjectSptr a_object)
{
    m_val->add("cur_object", a_object);
}
*/

// ---------------- static ----------------

ObjectUserSptr ObjectContext::getCurUser()
{
    ObjectContextSptr   context;
    ObjectUserSptr      context_user;

    // get curent user
    context = Thread::getCurContext();
    if (context){
        context_user = context->getUser();
    }

    return context_user;
}

ObjectSessionSptr ObjectContext::getCurSession()
{
    ObjectContextSptr   context;
    ObjectSessionSptr   context_session;

    // get curent user
    context = Thread::getCurContext();
    if (context){
        context_session = context->getSession();
    }

    return context_session;
}

ObjectContextSptr ObjectContext::getCurContext()
{
    return Thread::getCurContext();
}

/*
ObjectSptr ObjectContext::getCurObject()
{
    ObjectSptr          ret;
    ObjectContextSptr   context;

    context = Thread::getCurContext();
    ret     = context->_getCurObject();

    return ret;
}

void ObjectContext::setCurObject(
    ObjectSptr a_object)
{
    ObjectSptr          ret;
    ObjectContextSptr   context;

    context = Thread::getCurContext();
    context->setCurObject(a_object);
}
*/

// ---------------- module ----------------

string ObjectContext::_getType()
{
    return "core.object.context";
}

string ObjectContext::_getTypePerl()
{
    return "core::object::context::main";
}

int32_t ObjectContext::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectContext::_destroy()
{
}

ObjectSptr ObjectContext::_object_create()
{
    ObjectSptr object;
    _OBJECT_CONTEXT(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectContext::_getType,
    .type_perl      = ObjectContext::_getTypePerl,
    .init           = ObjectContext::_init,
    .destroy        = ObjectContext::_destroy,
    .object_create  = ObjectContext::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

