#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/bin/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/int64/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/vec3/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "db.hpp"

Db::Db()
{
}

Db::~Db()
{
}

int32_t Db::save(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         err = 0;
    ObjectMapIt     objects_it;
    ObjectSptr      object;
    string          object_id;
    ObjectTimeSptr  prop_remove_time;
    ObjectTimeSptr  prop_cur_time;
    double          remove_time = 0.0f;
    double          cur_time    = 0.0f;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    // get object info
    object_id   = a_object->getId();
    remove_time = a_object->getTimeRemove()->getVal();

    PTIMING(0.000f, "'%s' save\n", object_id.c_str());

    PROP_TIME_NOW(prop_cur_time);
    cur_time = prop_cur_time->getVal();

    // update modify time
    prop_cur_time->setDirty(0);
    a_object->setTimeModify(prop_cur_time);

    // maybe it's removing?
    if (    0 < remove_time
        &&  (remove_time < cur_time))
    {
        // remove time expire, remove object
        remove(object_id);
        err = 0;
        goto out;
    }

    // ok, it's or insert or update
    if (!a_object->isNew()){
        err = update(a_object);
    } else {
        err = insert(a_object);
    }

    if (err){
        goto out;
    }

    a_object->isNew(0);

    // all ok
    err = 0;

out:
    return err;
}

