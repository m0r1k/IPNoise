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
#include "thread.hpp"
#include "db.hpp"

#include "db/mongo.hpp"

//recursive_mutex g_db_mutex;

DbMongo::DbMongo()
    :   Db()
{
}

DbMongo::~DbMongo()
{
}

int32_t DbMongo::do_init()
{
    PDEBUG(30, "DbMongo::do_init\n");

    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    int32_t err = -1;

    try {
        m_mongo_db.connect("127.0.0.1");
    } catch (const mongo::DBException &e){
        PERROR("[ connect ] mongo init failed (%s)\n",
            e.what()
        );
    } catch (...){
        PERROR("[ connect ] mongo init failed\n");
        goto fail;
    }

    // all ok;
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void DbMongo::_processCommand(
    ObjectVectorSptr    a_out,
    const string        &a_cmd,
    const int32_t       &a_get_removed,
    const int32_t       &a_db_lock,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectSptr          object;
    mongo::BSONElement  el;
    mongo::BSONObj      obj;
    string              type;
    mongo::BSONObj      info;
    mongo::BSONObj      cmd;
    int32_t             res;
    ThreadSptr          thread;
    ObjectContextSptr   context;
    string              location;

    PTIMING(0.010f, "send command to db\n");

    thread      = Thread::getCurThread();
    context     = thread->getCurContext();
    location    = thread->getLocation();

    try {
        PTIMING(0.001f, "convert command from json\n");
        cmd = mongo::fromjson(a_cmd);
    } catch (...){
        PFATAL("cannot parse command: '%s'\n",
            a_cmd.c_str()
        );
    }
    res = m_mongo_db.runCommand(
        "network",
        cmd,
        info
    );
    if (!res){
        goto out;
    }

    if (!info.hasField("value")){
        goto out;
    }

    el = info.getField("value");
    if (mongo::BSONType::Object != el.type()){
        goto out;
    }

    el.Val(obj);

    // Type
    if (obj.hasField("type")){
        type = obj.getStringField("type");
    }

    if (type.empty()){
        PFATAL("cannot load object: '%s'"
            " (missing field: 'type')\n",
            obj.toString().c_str()
        );
    }

    object = Object::createObject(
        type,
        a_do_autorun
    );
    if (!object){
        PFATAL("cannot load object: '%s'"
            " (cannot create object)\n",
            obj.toString().c_str()
        );
    }

    object->do_init_props();
    object->do_init_api();

    // fill object with data from DB
    res = object->parseBSON(obj);
    if (res){
        PFATAL("cannot load object: '%s'"
            " (cannot parse BSON)\n",
            obj.toString().c_str()
        );
    }

    // all ok, return this object
    if (    !object->isRemoved()
        ||  a_get_removed)
    {
        // loaded successful
        object->isNew(0);
        object->setDirty(0);

        if (a_db_lock){
            ObjectStringSptr prop_lock;
            PROP_STRING(prop_lock, location);
            INIT_PROP(object.get(), Lock, prop_lock);
        }

        // add to transaction
        context->transactionAdd(object);

        // add to output
        a_out->add(object);
    }

out:
    PDEBUG(60, "after processCommand:\n"
        "  a_cmd:           '%s'\n"
        "  a_get_removed:   '%d'\n"
        "  a_do_autorun:    '%d'\n"
        "  a_out->size():   '%d'\n",
        a_cmd.c_str(),
        a_get_removed,
        a_do_autorun,
        a_out->size()
    );
    return;
}

ObjectSptr DbMongo::_getObjectById(
    const string    &a_id,
    const string    &a_type,
    const int32_t   &a_get_removed,
    const int32_t   &a_db_lock,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    PTIMING(0.010f, "get object by ID: '%s', type: '%s'\n",
        a_id.c_str(),
        a_type.c_str()
    );

    unique_ptr<mongo::DBClientCursor>   cursor;

    ObjectVectorSptr    res;
    ObjectVectorIt      res_it;
    ObjectSptr          ret;
    string              cmd_str;
    ThreadSptr          thread;
    ObjectContextSptr   context;
    string              location;
    string              type;
    int32_t             res_size = 0;

    PROP_VECTOR(res);

    thread      = Thread::getCurThread();
    context     = thread->contextGet();
    location    = thread->getLocation();

    if (!a_type.empty()){
        type = ", type: \"" + a_type + "\"";
    }

    // TODO XXX remove ugly code
    //
    // I have tried use ObjectMap here, but look at this:
    // update: { $set: {}, $set: {} }
    // how map can contain two fields with same key?
    // stupid idiots, fucking mongo..
    // 20141117 morik@
    cmd_str = "{"
        "findAndModify: \"objects\","
        "query: {"
            "_id: \"" + a_id + "\""
            + type +
        "},"
        "update: {"
            "$set: { fake: 1 }"
        "}"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        res,
        cmd_str,
        a_get_removed,
        a_db_lock,
        a_do_autorun
    );

    if (!a_db_lock){
        goto check_size;
    }

    if (!res->size()){
        goto check_size;
    }

    while (1){
        // if object exist, try get it again,
        // but now with lock

        // clear
        PROP_VECTOR(res);

        cmd_str = "{"
            "findAndModify: \"objects\","
            "query: {"
//                "lock: { $in: [ \"\", \"" + location + "\" ] },"
                "lock: \"\","
                "_id: \"" + a_id + "\""
                + type +
            "},"
            "update: {"
                " $set: { lock: \"" + location + "\" }"
            "},"
            "upsert: false"
        "}";

        PDEBUG(60, "send DB request '%s'\n",
            cmd_str.c_str()
        );

        _processCommand(
            res,
            cmd_str,
            a_get_removed,
            a_db_lock,
            a_do_autorun
        );

        if (!res->empty()){
            // TODO FIXME XXX
            // if object will be deleted
            // between first and second commands
            // we will get infinity loop
            // 20141117 morik@
            break;
        }

        // wait 1ms and try again
        usleep(0.001 * 1e6);
        PFATAL("wait when: '%s' will be unlocked\n",
            a_id.c_str()
        );
    }

check_size:

    res_size = res->size();
    if (1 < res_size){
        PWARN("more than 1 object (%d), was found"
            " by ID: '%s'\n",
            res_size,
            a_id.c_str()
        )
        ret = *res->begin();
    } else if (1 == res_size){
        ret = *res->begin();
    }

    return ret;
}

void DbMongo::unlockLocationObjects()
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    mongo::BSONObj  info;
    mongo::BSONObj  cmd;
    string          cmd_str;

    cmd_str = "{"
        "$set: { lock: \"\" }"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    cmd = mongo::fromjson(cmd_str);

    // TODO XXX FIXME
    // check location here and unlock only objects
    // for current location (hostname)
    // now we do unlock all lock objects (and it's bad idea)
    m_mongo_db.update(
        "network.objects",
        QUERY("lock" << mongo::NE << ""), // FIXME TODO XXX
        cmd,
        false,  // upsert
        true    // multi
    );
}

void DbMongo::cleanLocationObjects()
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    mongo::BSONObj  info;
    mongo::BSONObj  cmd;
    string          cmd_str;

    cmd_str = "{"
        "$set: { time_remove: 0.1 }"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    cmd = mongo::fromjson(cmd_str);

    // TODO XXX FIXME
    // check location here and remove only objects
    // for current location (hostname)
    // now we do delete all located objects
    // (and it's bad idea)
    m_mongo_db.update(
        "network.objects",
        QUERY("location" << mongo::NE << ""), // FIXME TODO XXX
        cmd,
        false,  // upsert
        true    // multi
    );
}

/*
void DbMongo::cleanActionObjects()
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    mongo::BSONObj  info;
    mongo::BSONObj  cmd;
    string          cmd_str;

    cmd_str = "{"
        "$set: { time_remove: 0.1 }"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    cmd = mongo::fromjson(cmd_str);

    m_mongo_db.update(
        "network.objects",
        QUERY("type" << "core.object.action"),
        cmd,
        false,  // upsert
        true    // multi
    );
}
*/

void DbMongo::do_clean()
{
    //cleanActionObjects();
    unlockLocationObjects();
    cleanLocationObjects();
}

void DbMongo::getActionsForCurThread(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectTimeSptr  cur_time;
    ThreadSptr      thread;
    string          cmd_str;
    string          location;

    PROP_TIME_NOW(cur_time);

    thread      = Thread::getCurThread();
    location    = thread->getLocation();

    cmd_str = "{"
        "findAndModify: \"objects\","
        "query: {"
            " lock: \"\","
            " type: \"core.object.action\","
            " location: \"" + location + "\","
            " time_process: { $lt: " + cur_time->toString() + " },"
            " time_process: { $gt: 0.0 }"
        "},"
        "sort:      { time_create: +1 }," // from oldest to newest
        "update:    {"
            " $set: { lock: \"" + location + "\" },"
            " $set: { time_process: -0.1 }"
        "},"
        "upsert:    false"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        a_out,
        cmd_str,
        a_get_removed,
        1,              // db_lock
        a_do_autorun
    );

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbMongo::getActionsForAnyThread(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectTimeSptr  cur_time;
    ThreadSptr      thread;
    string          location;
    string          cmd_str;

    thread   = Thread::getCurThread();
    location = thread->getLocation();

    PROP_TIME_NOW(cur_time);

    cmd_str = "{"
        "findAndModify: \"objects\","
        "query:     {"
            " type: \"core.object.action\","
            " lock: \"\","
            " location: \"\","
            " time_process: { $lt: " + cur_time->toString() + " },"
            " time_process: { $gt: 0.0 }"
        "},"
        "sort:      { time_create: +1 }," // from oldest to newest
        "update:    {"
            " $set: { lock: \"" + location + "\" },"
            " $set: { time_process: -0.1 }"
        "},"
        "upsert:    false"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        a_out,
        cmd_str,
        a_get_removed,
        1,              // db_lock
        a_do_autorun
    );

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbMongo::getActionsForProcess(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    getActionsForCurThread(
        a_out,
        a_get_removed,
        a_do_autorun
    );
    getActionsForAnyThread(
        a_out,
        a_get_removed,
        a_do_autorun
    );
}

void DbMongo::getObjectsForRemoveExpired(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectTimeSptr  cur_time;
    ThreadSptr      thread;
    string          location;
    string          cmd_str;

    thread      = Thread::getCurThread();
    location    = thread->getLocation();

    PROP_TIME_NOW(cur_time);

    cmd_str = "{"
        "findAndModify: \"objects\","
        "query:     {"
            " lock: \"\","
            " time_remove: { $lt: " + cur_time->toString() + " },"
            " time_remove: { $gt: 0.0 }"
        "},"
        "sort:      { time_remove: +1 }," // from oldest to newest
        "update:    {"
            " $set: { lock: \"" + location + "\" },"
            " $set: { time_process: -0.1 }"
        "},"
        "upsert:    false,"
        "limit:     1"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        a_out,
        cmd_str,
        0,              // get_removed
        1,              // db_lock
        a_do_autorun
    );

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

/*
void DbMongo::getObjectsForRemoveLocation(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectTimeSptr  cur_time;
    ThreadSptr      thread;
    string          location;
    string          cmd_str;

    thread      = Thread::getCurThread();
    location    = thread->getLocation();

    PROP_TIME_NOW(cur_time);

    cmd_str = "{"
        "findAndModify: \"objects\","
        "query:     {"
            " location: { $ne: \"\" }"
        "},"
        "sort:      { time_create: +1 }," // from oldest to newest
        "update:    {"
            " $set: { lock: \"" + location + "\" },"
            " $set: { time_process: -0.1 }"
        "},"
        "upsert:    false,"
        "limit:     1"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        a_out,
        cmd_str,
        0,              // get_removed
        1,              // db_lock
        a_do_autorun
    );

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbMongo::getObjectsForRemoveLock(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectTimeSptr  cur_time;
    ThreadSptr      thread;
    string          location;
    string          cmd_str;

    thread      = Thread::getCurThread();
    location    = thread->getLocation();

    PROP_TIME_NOW(cur_time);

    cmd_str = "{"
        "findAndModify: \"objects\","
        "query:     {"
            " lock: { $ne: \"\" },"
            " lock: { $ne: \"" + location + "\" }"
        "},"
        "sort:      { time_create: +1 }," // from oldest to newest
        "update:    {"
            " $set: { lock: \"" + location + "\" },"
            " $set: { time_process: -0.1 }"
        "},"
        "upsert:    false,"
        "limit:     1"
    "}";

    PDEBUG(60, "send DB request '%s'\n",
        cmd_str.c_str()
    );

    _processCommand(
        a_out,
        cmd_str,
        0,              // get_removed
        1,              // db_lock
        a_do_autorun
    );

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}
*/

void DbMongo::getObjectsForRemove(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    getObjectsForRemoveExpired(a_out, a_do_autorun);
}

void DbMongo::getObjectsByPos(
    ObjectVec3Sptr      a_pos_min,
    ObjectVec3Sptr      a_pos_max,
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_db_lock,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    PFATAL("FIXME\n");
/*
    ObjectSptr                         object;
    unique_ptr<mongo::DBClientCursor>  cursor;

    PDEBUG(50, "search objects:\n"
        "  a_pos_min: '%s'\n"
        "  a_pos_max: '%s'\n",
        a_pos_min->toString().c_str(),
        a_pos_max->toString().c_str()
    );

    cursor = auto_ptr<mongo::DBClientCursor>(
        m_mongo_db.query(
            "network.objects",
            QUERY(  "pos.0" << mongo::GT << a_pos_min->get0()
                <<  "pos.0" << mongo::LT << a_pos_max->get0()
                <<  "pos.1" << mongo::GT << a_pos_min->get1()
                <<  "pos.1" << mongo::LT << a_pos_max->get1()
                <<  "pos.2" << mongo::GT << a_pos_min->get2()
                <<  "pos.2" << mongo::LT << a_pos_max->get2()
            )
        )
    );

    while (cursor->more()){
        mongo::BSONObj      obj;
        int32_t             res;
        string              type;
        ObjectSptr          object;

        obj = cursor->next();

        // Type
        if (obj.hasField("type")){
            type = obj.getStringField("type");
        }

        if (!type.size()){
            PFATAL("cannot load object: '%s'"
                " (missing field: 'type')\n",
                obj.toString().c_str()
            );
        }

        object = _createObject(
            type,
            a_do_autorun
        );
        if (!object){
            PFATAL("cannot load object: '%s'"
                " (cannot create object)\n",
                obj.toString().c_str()
            );
        }

        res = object->parseBSON(obj);
        if (res){
            PFATAL("cannot load object: '%s'"
                " (cannot parse BSON)\n",
                obj.toString().c_str()
            );
        }

        // all ok, return this object
        if (    !object->isRemoved()
            ||  a_get_removed)
        {
            // loaded successful
            object->isNew(0);
            object->setDirty(0);
            // add to output
            a_out->push_back(OBJECT(object));
        }
    }

    PDEBUG(50, "found size: '%d'\n", a_out->size());
*/
}

int32_t DbMongo::insert(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    int32_t                 err = -1;
    mongo::BSONArrayBuilder builder;
    mongo::BSONObj          json_obj;
    mongo::BSONObj          tmp_obj;
    vector<mongo::BSONObj>  tmp_vec;

    ObjectMapSptr           props_dirty;
    ObjectMapIt             it;
    string                  object_id;

    object_id = a_object->getId();

    PTIMING(0.005f, "'%s' save (insert)\n", object_id.c_str());

    PROP_MAP(props_dirty);
    a_object->getAllPropsDirty(props_dirty);

/*
            if (a_is_init){                                 \
                (Object *)new_val.get()                     \
                    ->setDirty(0);                          \
            } else {                                        \
                objectChanged(                              \
                    #name,                                  \
                    dynamic_pointer_cast<Object>(old_val),  \
                    dynamic_pointer_cast<Object>(new_val)   \
                );                                          \
            }                                               \
*/

    if (props_dirty->empty()){
        err = 0;
        goto out;
    }

    PDEBUG(60, "object ID: '%s', props: '%s'\n",
        object_id.c_str(),
        props_dirty->toString().c_str()
    );

    // get json_obj object
    // TODO maybe exist better way here?
    props_dirty->toBSON(builder);
    tmp_obj = builder.arr();
    tmp_obj.Vals(tmp_vec);
    if (!tmp_vec.size()){
        PFATAL("cannot serialise objects to BSON\n");
    }
    json_obj = tmp_vec.at(0);

    // insert to DB
    m_mongo_db.insert("network.objects", json_obj);

    // all ok
    err = 0;

out:
    return err;
}

int32_t DbMongo::update(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    int32_t                 err = -1;
    mongo::BSONArrayBuilder builder;
    mongo::BSONObj          json_obj;
    mongo::BSONObj          tmp_obj;
    vector<mongo::BSONObj>  tmp_vec;

    ObjectMapSptr           props_dirty;
    ObjectMapIt             it;
    string                  object_id;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    object_id = a_object->getId();

    PTIMING(0.005f, "'%s' save (update)\n", object_id.c_str());

    PROP_MAP(props_dirty);

    a_object->getAllPropsDirty(props_dirty);
    if (props_dirty->empty()){
        err = 0;
        goto out;
    }

    PDEBUG(60, "object ID: '%s', props: '%s'\n",
        object_id.c_str(),
        props_dirty->toString().c_str()
    );

    // get json_obj object
    // TODO maybe exist better way here?
    props_dirty->toBSON(builder);
    tmp_obj = builder.arr();
    tmp_obj.Vals(tmp_vec);
    if (!tmp_vec.size()){
        PFATAL("cannot serialise objects to BSON\n");
    }
    json_obj = tmp_vec.at(0);

    //mongo::BSONObj obj = mongo::fromjson(a_json_obj.c_str());
    m_mongo_db.update(
        "network.objects",
        QUERY("_id" << object_id), //mongo::OID(id)),
        json_obj
    );

    // all ok
    err = 0;

out:
    return err;
}

int32_t DbMongo::save(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         err = -1;
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

    PTIMING(0.010f, "'%s' save\n", object_id.c_str());

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
        goto out;
    }

    // ok, it's or insert or update
    if (!a_object->isNew()){
        err = update(a_object);
    } else {
        err = insert(a_object);
    }

    if (!err){
        // all ok
        a_object->isNew(0);
    };

out:
    return err;
}

void DbMongo::remove(
    const string    &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PTIMING(0.001f, "'%s' remove\n", a_object_id.c_str());

    if (a_object_id.empty()){
        PFATAL("missing argument 'a_object_id'\n");
    }

    m_mongo_db.remove(
        "network.objects",
        QUERY("_id" << a_object_id)
    );
}

bool DbMongo::isObjectExist(
    const string &a_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

//    lock(m_mutex, g_db_mutex);
//    lock_guard<recursive_mutex> guard1(m_mutex,    adopt_lock);
//    lock_guard<recursive_mutex> guard2(g_db_mutex, adopt_lock);

    ObjectSptr  object;
    bool        ret = false;

    unique_ptr<mongo::DBClientCursor>  cursor;

    cursor = auto_ptr<mongo::DBClientCursor>(
        m_mongo_db.query(
            "network.objects",
            QUERY(
                   "_id"     << a_id
                << "removed" << 0
            )
        )
    );
    if (cursor->more()){
        ret = true;
    }

    return ret;
}

// ---------------- static ----------------

void DbMongo::bsonToProps(
    mongo::BSONObj  a_obj,
    ObjectSptr      a_props)
{
    list< mongo::BSONElement >           els;
    list< mongo::BSONElement >::iterator els_it;

    string props_type = a_props->getType();

    a_obj.elems(els);

    for (els_it = els.begin();
        els_it != els.end();
        els_it++)
    {
        string              key     = els_it->fieldName();
        mongo::BSONType     type    = els_it->type();
        ObjectStringSptr    prop_key;

        PROP_STRING(prop_key, key);

        if (mongo::BSONType::String == type){
            string val;
            els_it->Val(val);

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectStringSptr    prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_STRING(prop_val, val);
                props->add(prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectStringSptr    prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_STRING(prop_val, val);
                props->add(prop_key, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::NumberInt == type){
            int32_t val = els_it->Int();

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectInt32Sptr     prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_INT32(prop_val, val);
                props->add(prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectInt32Sptr     prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_INT32(prop_val, val);
                props->add(prop_key, prop_val);
            } else {
                PERROR("unsupported a_props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::NumberLong == type){
            int64_t val = els_it->Long();

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectInt64Sptr     prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_INT64(prop_val, val);
                props->add(prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectInt64Sptr     prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_INT64(prop_val, val);
                props->add(prop_key, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::NumberDouble == type){
            double val = els_it->Double();

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectDoubleSptr    prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_DOUBLE(prop_val, val);
                props->add(prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectDoubleSptr    prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_DOUBLE(prop_val, val);
                props->add(prop_key, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::Object == type){
            mongo::BSONObj val;
            els_it->Val(val);

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectMapSptr       prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_MAP(prop_val);
                props->add(prop_val);
                bsonToProps(val, prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr   props;
                ObjectMapSptr   prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_MAP(prop_val);
                props->add(prop_key, prop_val);
                bsonToProps(val, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::Array == type){
            mongo::BSONObj val;
            els_it->Val(val);

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectVectorSptr    prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_VECTOR(prop_val);
                props->add(prop_val);
                bsonToProps(val, prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectVectorSptr    prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_VECTOR(prop_val);
                props->add(prop_key, prop_val);
                bsonToProps(val, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else if (mongo::BSONType::BinData == type){
            const char      *data_ptr   = NULL;
            int32_t         data_len    = 0;
            string          val;

            data_ptr = els_it->binData(data_len);
            if (    data_ptr
                &&  (0 < data_len))
            {
                val.assign(data_ptr, data_len);
            }

            if ("core.object.vector" == props_type){
                ObjectVectorSptr    props;
                ObjectBinSptr       prop_val;
                props = dynamic_pointer_cast<ObjectVector>(
                    a_props
                );
                PROP_BIN(prop_val, val);
                props->add(prop_val);
            } else if ("core.object.map" == props_type){
                ObjectMapSptr       props;
                ObjectBinSptr       prop_val;
                props = dynamic_pointer_cast<ObjectMap>(
                    a_props
                );
                PROP_BIN(prop_val, val);
                props->add(prop_key, prop_val);
            } else {
                PERROR("unsupported props type: '%s'\n",
                    props_type.c_str()
                );
            }
        } else {
            PERROR("unsupported element type: '%d',"
                " dump: '%s'\n",
                els_it->type(),
                els_it->toString().c_str()
            );
        }
    }
}

ObjectVectorSptr DbMongo::getArrayFieldAsVectorOfDouble(
    mongo::BSONObj  a_obj,
    const string    &a_field_name)
{
    ObjectVectorSptr                ret;
    mongo::BSONElement              el;
    mongo::BSONObj                  obj;
    vector<double>                  values;
    vector<double>::const_iterator  values_it;

    if (!a_obj.hasField(a_field_name)){
        goto out;
    }

    el = a_obj.getField(a_field_name);
    if (mongo::BSONType::Array != el.type()){
        PERROR("cannot parse '%s' attribute,"
            " object dump: '%s'\n",
            a_field_name.c_str(),
            a_obj.toString().c_str()
        );
        goto fail;
    }

    el.Val(obj);
    obj.Vals(values);

    OBJECT_VECTOR(ret);
    for (values_it = values.begin();
        values_it != values.end();
        values_it++)
    {
        ObjectDoubleSptr    prop_cur_val;
        double              cur_val = *values_it;
        PROP_DOUBLE(prop_cur_val, cur_val);
        ret->add(prop_cur_val);
    }

out:
    return ret;
fail:
    ret = ObjectVectorSptr();
    goto out;
}

ObjectVec3Sptr DbMongo::getArrayFieldAsPropVec3(
    mongo::BSONObj  a_obj,
    const string    &a_field_name)
{
    ObjectVec3Sptr    ret;
    ObjectVectorSptr  res;

    double pos_x = 0.0f;
    double pos_y = 0.0f;
    double pos_z = 0.0f;

    res = DbMongo::getArrayFieldAsVectorOfDouble(
        a_obj,
        a_field_name
    );

    pos_x = dynamic_pointer_cast<ObjectDouble>(
        res->at(0)
    )->getVal();
    pos_y = dynamic_pointer_cast<ObjectDouble>(
        res->at(1)
    )->getVal();
    pos_z = dynamic_pointer_cast<ObjectDouble>(
        res->at(2)
    )->getVal();

    PROP_VEC3(
        ret,
        pos_x,
        pos_y,
        pos_z
    );

    return ret;
}

ObjectVec3Sptr DbMongo::getMapFieldAsPropVec3(
    mongo::BSONObj  a_obj,
    const string    &a_field_name)
{
    ObjectVec3Sptr        ret;
    mongo::BSONObj      obj;
    mongo::BSONElement  el;

    el = a_obj.getField(a_field_name);
    if (mongo::BSONType::Object != el.type()){
        PFATAL("BSON element is not object,"
            " object dump: '%s'\n",
            a_obj.toString().c_str()
        );
    }
    el.Val(obj);

    if (    obj.hasField("0")
        &&  obj.hasField("1")
        &&  obj.hasField("2"))
    {
        double val_0 = obj.getField("0").Double();
        double val_1 = obj.getField("1").Double();
        double val_2 = obj.getField("2").Double();

        PROP_VEC3(ret, val_0, val_1, val_2);
    }

    return ret;
}

