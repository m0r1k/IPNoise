#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"

#include "core/object/transaction/main.hpp"

ObjectTransaction::ObjectTransaction(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectTransaction::~ObjectTransaction()
{
}

void ObjectTransaction::do_init(
    const Object::CreateFlags &a_flags)
{
    PROP_MAP(m_queue_all);
//    PROP_MAP(m_queue_save);
}

string ObjectTransaction::getType()
{
    return ObjectTransaction::_getType();
}

void ObjectTransaction::do_init_props()
{
    Object::do_init_props();
}

void ObjectTransaction::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectTransaction::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_queue_all->toBSON(a_builder, a_field_name);
}

void ObjectTransaction::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_queue_all->toBSON(a_builder);
}

SV * ObjectTransaction::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_queue_all->toPerl();
}

ObjectStringSptr ObjectTransaction::toString()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_queue_all->toString();
}

ObjectStringSptr ObjectTransaction::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_queue_all->serialize(a_delim);
}

ObjectMapSptr ObjectTransaction::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_queue_all;
}

ObjectSptr ObjectTransaction::_getObjectById(
    const string    &a_id,
    const string    &a_type,
    const int32_t   &a_get_removed,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info;
    ObjectSptr      object;
    ObjectSptr      tmp_object;
    string          tmp_object_type;

    // search
    info = dynamic_pointer_cast<ObjectMap>(
        m_queue_all->get(a_id)
    );
    if (!info){
        goto out;
    }
    tmp_object = info->get("object");
    if (tmp_object){
        tmp_object_type = tmp_object->getType();
    }

    if (tmp_object_type.empty()){
        m_queue_all->erase(a_id);
        goto out;
    }

    // check type
    if (    !a_type.empty()
        &&  a_type != tmp_object_type)
    {
        goto out;
    }

    // check is removed
    if (    tmp_object->isRemoved()
        &&  !a_get_removed)
    {
        goto out;
    }

    // all ok, we have found it
    object = tmp_object;

out:
    return object;
}

ObjectMapSptr ObjectTransaction::getObjectInfo(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info;
    string          object_id = a_object->getId();

    info = dynamic_pointer_cast<ObjectMap>(
        m_queue_all->get(object_id)
    );

    return info;
}

ObjectMapSptr ObjectTransaction::getCreateObjectInfo(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info;
    string          object_id = a_object->getId();

    //PWARN("getCreateObjectInfo for ID: '%s',"
    //    " type: '%s'\n",
    //    object_id.c_str(),
    //    a_object->getType().c_str()
    //);

    info = getObjectInfo(a_object);
    if (!info){
        PROP_MAP(info);
        m_queue_all->add(object_id, info);
    }

    info->add("object", a_object);

    return info;
}

void ObjectTransaction::add(
    ObjectSptr  a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (!a_object){
        PFATAL("missing argument 'object'\n");
    }

    getCreateObjectInfo(a_object);
}

void ObjectTransaction::addForSave(
    ObjectSptr  a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info;
    string          object_id = a_object->getId();
    string          lock;

    if (!a_object){
        PFATAL("'%s', missing argument: 'a_object'\n",
            object_id.c_str()
        );
    }

    lock = a_object->getLock()->toString();

    // if object is not new
    // check that it was locked
    if (!a_object->isNew()){
        if (lock.empty()){
            PFATAL("attempt to add for save unlocked object:\n"
                "  object_id:   '%s'\n"
                "  object_type: '%s'\n",
                a_object->getId().c_str(),
                a_object->getType().c_str()
            );
        }
    }

    info = getCreateObjectInfo(a_object);
    info->add("save", 1);

//    m_queue_save->add(object_id, info);
}

void ObjectTransaction::commit()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapIt it;
    DbSptr      db          = Thread::getCurThreadDb();
    int32_t     show_header = 1;

again:
    for (it = m_queue_all->begin();
        it != m_queue_all->end();
        it++)
    {
        ObjectMapSptr       prop_info;
        ObjectInt32Sptr     prop_save;
        int32_t             save = 0;
        ObjectStringSptr    prop_lock;
        string              lock;
        ObjectSptr          object;
        string              object_id;
        string              object_id_key;

        object_id_key = it->first->toString();
        prop_info     = dynamic_pointer_cast<ObjectMap>(
            it->second
        );

        prop_save = dynamic_pointer_cast<ObjectInt32>(
            prop_info->get("save")
        );
        if (prop_save){
            save = prop_save->getVal();
        }

        object      = prop_info->get("object");
        object_id   = object->getId();
        prop_lock   = object->getLock();
        lock        = prop_lock->toString();

        if (object_id != object_id_key){
            // id was changed
            //if (!m_queue_all->has(object_id)){
            //    m_queue_all->add(object_id, prop_info);
            //}
            //m_queue_all->erase(it);
            //m_queue_all->add(object_id, prop_info);
            //goto again;
            PFATAL("key ID: '%s'"
                " is not equal object ID: '%s'\n",
                object_id_key.c_str(),
                object_id.c_str()
            );
        }

        // if object locked, unlock and request save
        if (!lock.empty()){
            // unlock
            ObjectStringSptr    empty;
            PROP_STRING(empty);
            object->setLock(empty);
            save = 1;
        }

        // save to DB
        if (save){
            if (show_header){
                PDEBUG(50, "commit transaction:\n");
                show_header = 0;
            }

            PDEBUG(50, "  commit object:\n"
                "  object id:           '%s'\n"
                "  object type:         '%s'\n"
                "  object name:         '%s'\n"
                "  object lock:         '%s'\n"
                "  object time_remove:  '%s'\n",
                object_id.c_str(),
                object->getType().c_str(),
                object->getName()->toString()->c_str(),
                lock.c_str(),
                object->getTimeRemove()->toString()->c_str()
            );

            // if object is not new
            // check that it was locked
            if (!object->isNew()){
                if (lock.empty()){
                    PFATAL("attempt to save"
                        " not locked object:\n"
                        "  object_id:   '%s'\n"
                        "  object_type: '%s'\n",
                        object->getId().c_str(),
                        object->getType().c_str()
                    );
                }
            }

            //PWARN("unlock object: '%s'\n", object_id.c_str());

            // save in DB
            db->save(object);

            m_queue_all->erase(object_id);
            goto again;
        }
    }
}

void ObjectTransaction::remove(
    const string &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (a_object_id.empty()){
        PFATAL("missing argument: 'object_id'\n");
    }

    m_queue_all->erase(a_object_id);
    //m_queue_save->erase(a_object_id);
}

// ---------------- module ----------------

string ObjectTransaction::_getType()
{
    return "core.object.transaction";
}

string ObjectTransaction::_getTypePerl()
{
    return "core::object::transaction::main";
}

int32_t ObjectTransaction::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectTransaction::_destroy()
{
}

ObjectSptr ObjectTransaction::_object_create()
{
    ObjectSptr object;
    _OBJECT_TRANSACTION(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectTransaction::_getType,
    .type_perl      = ObjectTransaction::_getTypePerl,
    .init           = ObjectTransaction::_init,
    .destroy        = ObjectTransaction::_destroy,
    .object_create  = ObjectTransaction::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

