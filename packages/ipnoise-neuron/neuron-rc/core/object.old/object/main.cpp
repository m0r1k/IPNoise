#include <math.h>
#include <assert.h>

#include "base64.hpp"

// don't forget update
// core/object/object/main.cpp
// core/object/object/main.hpp
// Makefile
//
// and don't forget add
// #include "core/object/context/main.hpp"
// in your module main.cpp file

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
//#include "core/object/basket/main.hpp"
#include "core/object/camera/main.hpp"
//#include "core/object/dialog/main.hpp"
//#include "core/object/dialogs/main.hpp"
#include "core/object/double/main.hpp"
//#include "core/object/dream/main.hpp"
//#include "core/object/dreams/main.hpp"
//#include "core/object/dreamland/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/int64/main.hpp"
#include "core/object/map/main.hpp"
//#include "core/object/media/main.hpp"
#include "core/object/objectLink/main.hpp"
//#include "core/object/objectLinkAction/main.hpp"
//#include "core/object/objectLinkInfo/main.hpp"
#include "core/object/openssl/main.hpp"
//#include "core/object/param/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/template/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/uint32/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/vec3/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/websockFrame/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"
#include "db/mongo.hpp"

recursive_mutex g_modules_mutex;
ModulesSptr     g_modules;

// ---------------- cmp object ----------------

CmpObject::CmpObject()
{
}

CmpObject::~CmpObject()
{
}

bool CmpObject::operator () (
    ObjectSptr a,
    ObjectSptr b)
{
    return a->toStringStd() < b->toStringStd();
}

// ---------------- object ----------------

Object::Object(
    const Object::CreateFlags &a_flags)
{
    do_init(a_flags);
};

Object::~Object()
{
};

void Object::do_init(
    const Object::CreateFlags &a_flags)
{
    ObjectVec3Sptr view_pos;
    ObjectVec3Sptr view_up;
    ObjectVec3Sptr view_strafe;
    ObjectVec3Sptr view_zoom;

    m_is_new        = 1;        // is new object
    m_is_dirty      = 0;        // for props
    m_flags         = a_flags;  // object create flags
}

string Object::getType()
{
    return Object::_getType();
}

void Object::do_init_props()
{
    ObjectStringSptr    prop_lock;
    ObjectUint32Sptr    prop_flags;
    ObjectTimeSptr      prop_time_create;
    ObjectTimeSptr      prop_time_modify;
    ObjectTimeSptr      prop_time_remove;
    ObjectStringSptr    prop_name;
    ObjectStringSptr    prop_descr;
    ObjectStringSptr    prop_title;
    ObjectStringSptr    prop_url;
    ObjectMapSptr       prop_links_info;
    ObjectVec3Sptr      prop_pos;
    ObjectVec3Sptr      prop_view_pos;
    ObjectVec3Sptr      prop_view_up;
    ObjectVec3Sptr      prop_view_strafe;
    ObjectDoubleSptr    prop_view_zoom;
    ObjectVec3Sptr      prop_view_ryp;
//    ObjectVectorSptr    prop_signals;
//    ObjectVectorSptr    prop_events;

    // setup props
    m_id = Object::generateId();

    PROP_STRING(prop_lock);
    PROP_UINT32(prop_flags, m_flags);

    PROP_TIME_NOW(prop_time_create);
    PROP_TIME_NOW(prop_time_modify);
    PROP_TIME(prop_time_remove);

    PROP_STRING(prop_name);
    PROP_STRING(prop_descr);
    PROP_STRING(prop_title);

    PROP_STRING(prop_url, UPLOAD_DIR + m_id);

    // TODO XXX fixme, add ownerhere
    // INIT_PROP(this, Owner(context->getUserId());

    PROP_MAP(prop_links_info);

    // position
    PROP_VEC3(prop_pos,        0.00f, 0.00f, 0.00f);
    PROP_VEC3(prop_view_pos,   0.00f, 0.00f, -ELECTRON_RADIUS);
    PROP_VEC3(prop_view_up,    0.00f, 1.00f, 0.00f);
    PROP_VEC3(prop_view_strafe,ELECTRON_RADIUS, 0.00f, 0.00f);
    PROP_DOUBLE(prop_view_zoom,ELECTRON_RADIUS);
    PROP_VEC3(prop_view_ryp,   0.00f, 0.00f, 0.00f);

    // signals, events
//    PROP_VECTOR(prop_signals);
//    PROP_VECTOR(prop_events);

    // init props
    INIT_PROP(this, Lock,           prop_lock);
    INIT_PROP(this, ObjectFlags,    prop_flags);
    INIT_PROP(this, TimeCreate,     prop_time_create);
    INIT_PROP(this, TimeModify,     prop_time_modify);
    INIT_PROP(this, TimeRemove,     prop_time_remove);
    INIT_PROP(this, Name,           prop_name);
    INIT_PROP(this, Descr,          prop_descr);
    INIT_PROP(this, Title,          prop_title);
    INIT_PROP(this, Url,            prop_url);
    INIT_PROP(this, LinksInfo,      prop_links_info);
    INIT_PROP(this, Pos,            prop_pos);
    INIT_PROP(this, ViewPos,        prop_view_pos);
    INIT_PROP(this, ViewUp,         prop_view_up);
    INIT_PROP(this, ViewStrafe,     prop_view_strafe);
    INIT_PROP(this, ViewZoom,       prop_view_zoom);
    INIT_PROP(this, ViewRYP,        prop_view_ryp);
//    INIT_PROP(this, Signals,        prop_signals);
//    INIT_PROP(this, Events,         prop_events);
}

void Object::do_init_api()
{
    // register actions
    registerAction("create",             Object::actionCreate);
    registerAction("update",             Object::actionUpdate);
    registerAction("load",               Object::actionLoad);
    registerAction("delete",             Object::actionDelete);
    registerAction("neighUnlink",        Object::actionNeighUnlink);
    registerAction("render",             Object::actionRender);
    registerAction("renderEmbedded",     Object::actionRenderEmbedded);
    registerAction("getEvents",          Object::actionGetEvents);
    registerAction("getNeighs",          Object::actionGetNeighs);
    registerAction("getCreateNeigh",     Object::actionGetCreateNeigh);
    registerAction("bookmark",           Object::actionBookmark);
    registerAction("updateEventsSubscr", Object::actionUpdateEventsSubscr);
    registerAction("accel",              Object::actionAccel);
    registerAction("rotateViewVector",   Object::actionRotateViewVector);
    registerAction("move",               Object::actionMove);
}

int32_t Object::do_autorun()
{
    int32_t err = 0;

    updateSphere();

    return err;
}

void Object::setId(
    const string &a_id)
{
    m_id = a_id;
}

string Object::getId()
{
    return m_id;
}

/*
void Object::processActions(
    ObjectVectorSptr a_actions)
{
    lock<recursive_mutex>(m_mutex, a_actions->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_actions->m_mutex, adopt_lock);

    ObjectVectorIt it;

    PTIMING(0.010f, "%s process actions\n",
        getId().c_str()
    );

    PDEBUG(50, "process actions, object ID: '%s'\n"
        "  actions: '%s'\n",
        getId().c_str(),
        a_actions->toString()->c_str()
    );

    for (it = a_actions->begin();
        it != a_actions->end();
        it++)
    {
        ObjectActionSptr    action;
        action = dynamic_pointer_cast<ObjectAction>(*it);
        processAction(action);
    }
}
*/

ApiHandler Object::getHandler(
    const string &a_action_name)
{
    ApiHandlersIt   it;
    ApiHandler      handler = NULL;

    // search handler
    it = m_action_handlers.find(a_action_name);
    if (m_action_handlers.end() != it){
        handler = it->second;
    }

    return handler;
}

void Object::processAction(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock(a_object->m_mutex, a_action->m_mutex);
    lock_guard<recursive_mutex> guard1(a_object->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_action->m_mutex, adopt_lock);

    string              name;
    ObjectContextSptr   context;
    ApiHandler          handler = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    // get action name
    name = a_action->getName()->toString();
    if (name.empty()){
        PFATAL("empty action name, action below:\n'%s'\n",
            a_action->toString()->c_str()
        );
    }
    handler = a_object->getHandler(name);

    // process method
    if (handler){
        (*handler)(
            a_object,
            a_action
        );
    } else {
        PWARN("unsupported action: '%s'"
            " for object ID: '%s'\n",
            name.c_str(),
            a_object->getId().c_str()
        );
    }
}

/*
void Object::processSignals(
    ObjectVectorSptr a_signals)
{
    lock<recursive_mutex>(m_mutex, a_signals->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_signals->m_mutex, adopt_lock);

    ObjectVectorIt it;

    PTIMING(0.010f, "%s process signals\n",
        getId().c_str()
    );

    PDEBUG(50, "process signals, object ID: '%s'\n"
        "  signals: '%s'\n",
        getId().c_str(),
        a_signals->toString()->c_str()
    );

    for (it = a_signals->begin();
        it != a_signals->end();
        it++)
    {
        ObjectHttpAnswerSptr    answer;
        ObjectMapSptr           req_props;
        ObjectMapSptr           req_params;

        PTIMING(0.010f, "%s process signal\n",
            getId().c_str()
        );

        PROP_HTTP_ANSWER(answer);
        req_props  = dynamic_pointer_cast<ObjectMap>(*it);
        req_params = dynamic_pointer_cast<ObjectMap>(
            req_props->get("params")
        );
        if (!req_params){
            PROP_MAP(req_params);
        }
        signal(
            answer,
            req_props,
            req_params
        );
    }
}

void Object::processEvents(
    ObjectVectorSptr a_events)
{
    lock<recursive_mutex>(m_mutex, a_events->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_events->m_mutex, adopt_lock);

    ObjectVectorIt it;

    PTIMING(0.010f, "%s process events\n",
        getId().c_str()
    );

    PDEBUG(50, "process events, object ID: '%s'\n"
        "  events: '%s'\n",
        getId().c_str(),
        a_events->toString()->c_str()
    );

    for (it = a_events->begin();
        it != a_events->end();
        it++)
    {
        ObjectHttpAnswerSptr    answer;
        ObjectMapSptr           req_props;
        ObjectMapSptr           req_params;

        PROP_HTTP_ANSWER(answer);
        req_props  = dynamic_pointer_cast<ObjectMap>(*it);
        req_params = dynamic_pointer_cast<ObjectMap>(
            req_props->get("params")
        );
        if (!req_params){
            PROP_MAP(req_params);
        }
        event(
            answer,
            req_props,
            req_params
        );
    }
}

void Object::objectChanged(
    const string    &a_object_name,
    ObjectSptr      a_old_val,
    ObjectSptr      a_new_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (    "Pos"       == a_object_name
        ||  "ViewUp"    == a_object_name
        ||  "ViewRYP"   == a_object_name)
    {

    } else if ("ViewZoom" == a_object_name){
        // recalculate position of view and strafe
        updateSphere();
    }
}
*/

int32_t Object::isNew()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    return m_is_new;
}

void Object::isNew(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);
    m_is_new = a_val;
}

void Object::setName(
    const string &a_value)
{
    ObjectStringSptr    prop_value;
    PROP_STRING(prop_value, a_value);
    setName(prop_value);
}

bool Object::isRemoved()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    bool    ret         = false;
    double  time_remove = getTimeRemove()->getVal();

    if (0 > time_remove){
        ret = true;
    }

    return ret;
}

void Object::addToBookmarks()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    PFATAL("FIXME\n");
/*
    ObjectContextSptr   context;
    ObjectUserSptr      cur_user;

    cur_user = Context::getCurUser();

    // add neigh to user's bookmarks
    cur_user->addToBookmarks(this);
*/
}

void Object::delFromBookmarks()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    PFATAL("FIXME\n");
/*
    ObjectContextSptr   context;
    ObjectUserSptr      cur_user;

    cur_user = Context::getCurUser();

    // add neigh to user's bookmarks
    cur_user->delFromBookmarks(this);
*/
}

/*
void Object::removeLink(
    const string    &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectContextSptr context      = Thread::getCurContext();
    int32_t           links_before = m_LinksInfo->size();
    int32_t           links_after  = 0;
    uint32_t          flags        = 0;

    if (a_object_id.empty()){
        PFATAL("missing argument 'a_object_id'\n");
    }

    m_LinksInfo->erase(a_object_id);

    links_after = m_LinksInfo->size();
    flags       = getObjectFlags()->getVal();

    if (    links_before
        &&  !links_after)
    {
        if (flags & Object::REMOVE_WHEN_LINKS_LOST){
            PWARN("'%s' (type: '%s') all links lost,"
                " object will be removed (force = 0)\n",
                getId().c_str(),
                getType().c_str()
            );
            remove();
        }
        if (flags & Object::REMOVE_WHEN_LINKS_LOST_FORCE){
            PWARN("'%s' (type: '%s') all links lost,"
                " object will be removed (force = 1)\n",
                getId().c_str(),
                getType().c_str()
            );
            remove(1);
        }
    }
}

void Object::neighUnlink(
    const string    &a_object_id,
    const int32_t   &a_force)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;

    PROP_VECTOR(links);
    getLinks(
        links,  // out
        "",     // name
        0,      // get removed
        1       // db_lock
    );

    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr  link    = *links_it;
        string      link_id = link->getId();
        ObjectSptr  object;

        if (link->hasLink(a_object_id)){
            // remove from links info
            if (a_force){
                removeLink(link_id);
            }
            // remove link
            link->remove(a_force);
        }
    }
}
*/

void Object::neighUnlink(
    ObjectSptr      a_object_src,
    ObjectSptr      a_object_dst,
    const int32_t   &a_force)
{
    lock(a_object_src->m_mutex, a_object_dst->m_mutex);
    lock_guard<recursive_mutex> guard1(a_object_src->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_object_dst->m_mutex, adopt_lock);

    Object::neighUnlink(
        a_object_src,
        a_object_dst->getId(),
        a_force
    );
}

/*
void Object::neighUnlinkAll(
    const int32_t   &a_force)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;

    PROP_VECTOR(links);
    getLinks(links);

    // note, all removing must going via removeLink method,
    // because removeLink must handling
    // REMOVE_WHEN_LINKS_LOST* object flags
    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr  link    = *links_it;
        string      link_id = link->getId();
        ObjectSptr  object;

        // remove link
        link->remove(a_force);

        // remove from links info
        removeLink(link_id);
    }
}
*/

void Object::neighDelete(
    ObjectSptr      a_object_src,
    const string    &a_neigh_id,
    const int32_t   &a_force)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;
    ObjectContextSptr   context = Thread::getCurContext();

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    if (a_neigh_id.empty()){
        PFATAL("missing argument: 'a_neigh_id'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    PROP_VECTOR(links);
    a_object_src->getLinks(links);

    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr  link    = *links_it;
        string      link_id = link->getId();

        if (link->hasLink(a_neigh_id)){
            ObjectSptr object;
            if (a_force){
                Object::removeLink(a_object_src, link_id);
            }
            Object::remove(link, a_force);

            // load object
            object = context->getObjectById(
                a_neigh_id, // id
                "",         // type
                0,          // get removed
                1           // db lock
            );
            if (object){
                Object::remove(object, a_force);
            }
        }
    }
}

void Object::getPublicProps(
    ObjectMapSptr a_objects_out,
    ObjectMapSptr a_objects_in)
{
    lock(a_objects_out->m_mutex, a_objects_in->m_mutex);
    lock_guard<recursive_mutex> guard1(a_objects_out->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_objects_in->m_mutex,  adopt_lock);

    ObjectMapSptr objects_in;
    ObjectMapIt   it;

    if (!a_objects_out){
        PFATAL("missing argument: 'a_objects_out'\n");
    }

    if (!a_objects_in){
        PFATAL("missing argument: 'a_objects_in'\n");
    }

    for (it = objects_in->begin();
        it != objects_in->end();
        it++)
    {
        ObjectSptr object_key   = it->first;
        ObjectSptr object_val   = it->second;
        string   key        = object_key->toString();

        if ("_id" == key){
            a_objects_out->add("id", object_val);
            continue;
        } else if ("type" == key){
        } else if ("name" == key){
        } else if ("title" == key){
        } else if ("descr" == key){
        } else if ("pos" == key){
        } else {
            // all other objecterties hidden from public
            continue;
        }

        a_objects_out->add(object_key, object_val);
    }
}

void Object::getPublicProps(
    ObjectMapSptr a_objects_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr objects_in;

    if (!a_objects_out){
        PFATAL("missing argument: 'a_objects_out'\n");
    }

    PROP_MAP(objects_in);
    getAllProps(objects_in);

    Object::getPublicProps(a_objects_out, objects_in);
}

void Object::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string              id;
    ObjectStringSptr    prop_id;

    if (!(Object::CREATE_OBJECT & m_flags)){
        goto out;
    }

    id = getId();
    PROP_STRING(prop_id, id);

    // add props for save
    a_props->add("_id", prop_id);

    SAVE_PROP("lock",           getLock);
    SAVE_PROP("object_flags",   getObjectFlags);
    SAVE_PROP("time_create",    getTimeCreate);
    SAVE_PROP("time_modify",    getTimeModify);
    SAVE_PROP("time_remove",    getTimeRemove);
    SAVE_PROP("type",           getType);
    SAVE_PROP("name",           getName);
    SAVE_PROP("descr",          getDescr);
    SAVE_PROP("title",          getTitle);
    SAVE_PROP("url",            getUrl);
    SAVE_PROP("links",          getLinksInfo);
    SAVE_PROP("pos",            getPos);
    SAVE_PROP("view_pos",       getViewPos);
    SAVE_PROP("view_up",        getViewUp);
    SAVE_PROP("view_strafe",    getViewStrafe);
    SAVE_PROP("view_zoom",      getViewZoom);
//    SAVE_PROP("signals",        getSignals);
//    SAVE_PROP("events",         getEvents);

    // if ID is empty, remove it
    if (id.empty()){
        a_props->erase("_id");
    }

out:
    return;
}

void Object::setDirty(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (Object::CREATE_OBJECT & m_flags){
        // object
        ObjectMapSptr props;
        ObjectMapIt   it;

        PROP_MAP(props);

        getAllProps(props);

        for (it = props->begin();
            it != props->end();
            it++)
        {
            ObjectSptr prop_name  = it->first;
            ObjectSptr prop_val   = it->second;

            prop_name->setDirty(a_val);
            prop_val->setDirty(a_val);
        }
    } else {
        // prop
        m_is_dirty = a_val;
    }
}

/*
int32_t Object::_save()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         err = -1;
    ObjectMapSptr   props;
    ObjectMapSptr   props_dirty;
    ObjectMapIt     it;
    ObjectTimeSptr  cur_time;
    string          object_id = getId()->toString();

    PDEBUG(30, "save object: '%s'\n",
        object_id.c_str()
    );

    PTIMING(0.010f, "save object: '%s'\n",
        object_id.c_str()
    );

    PROP_MAP(props);
    PROP_MAP(props_dirty);
    PROP_TIME_NOW(cur_time);

    getAllProps(props);

  //
  //        if (a_is_init){                                 \
  //            (Object *)new_val.get()                     \
  //                ->setDirty(0);                          \
  //        } else {                                        \
  //            objectChanged(                              \
  //                #name,                                  \
  //                dynamic_pointer_cast<Object>(old_val),  \
  //                dynamic_pointer_cast<Object>(new_val)   \
  //            );                                          \
  //        }                                               \



    for (it = props->begin();
        it != props->end();
        it++)
    {
        ObjectSptr prop_name  = it->first;
        ObjectSptr prop_val   = it->second;

        if (    prop_name->isDirty()
            ||  prop_val->isDirty())
        {
            props_dirty->add(prop_name, prop_val);
        }
    }

    if (!props_dirty->empty()){
        //props->add("time_modify", cur_time);

        // save props
        err = saveProps(props);
        if (err){
            goto fail;
        }
    }

    // save params
    err = saveParams();
    if (err){
        goto fail;
    }

    // mark as not dirty now
    setDirty(0);

    if (!props_dirty->empty()){
        // update view
        if (    props_dirty->has("Pos")
            ||  props_dirty->has("ViewUp")
            ||  props_dirty->has("ViewRYP"))
        {
//
//          ObjectVec3Sptr  val;
//          double          x    = 0.0f;
//          double          y    = 0.0f;
//          double          z    = 0.0f;
//          double          min  = -1;
//          double          max  = +1;
//
//          val = dynamic_pointer_cast<ObjectVec3>(a_new_val);
//          x   = val->getX();
//          y   = val->getY();
//          z   = val->getZ();
//
//          if (props_dirty->has("ViewRYP"){
//              min = -2*M_PI;
//              max = +2*M_PI;
//          }
//
//          if (    min > x || max < x
//              ||  min > y || max < y
//              ||  min > z || max < z)
//          {
//              PFATAL("attempt to set invalid value: '%s',"
//                  " x,y,z must be %g <= val <= %g\n",
//                  val->toString()->c_str(),
//                  min,
//                  max
//              );
//          }
//
            // recalculate position of view and strafe
            updateSphere();
        } else if (props_dirty->has("ViewZoom")){
            // recalculate position of view and strafe
            updateSphere();
        }

        // submit event
        sendApiActionPropsChanged(props_dirty);
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t Object::save()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t             err     = 0;
    ObjectContextSptr   context = Thread::getCurContext();
    string              id      = getId();

    context->transactionMarkForSave(id);

    return err;
}

int32_t Object::saveProps(
    ObjectMapSptr   a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err         = 0;
    string  object_id   = getId()->toString();

    PTIMING(0.010f, "'%s'\n",
        object_id.c_str()
    );

    DbSptr  db  = Thread::getCurThreadDb();
    db->save(this, a_props);

    return err;
}

void Object::sendApiActionPropsChanged(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    res;
    ObjectVectorIt      res_it;
    ObjectActionSptr    action;
    ObjectUniqueSptr    object_types;
    ObjectStringSptr    empty;
    string              object_id = getId();
    ObjectContextSptr   context   = Thread::getCurContext();

    PROP_VECTOR(res);
    PROP_STRING(empty);
    PROP_UNIQUE(object_types);
    object_types->add("core.object.session");

    PDEBUG(50, "Object::sendApiActionPropsChanged:\n"
        " object ID:    '%s'\n"
        " props:        '%s'\n",
        object_id.c_str(),
        a_props->toString()->c_str()
    );

    // create action
    OBJECT_ACTION(action, "props.changed");
    action->addParam("object_id", object_id);
    action->addParam("props",     a_props);

    // search sessions
    getNeighs(
        res,
        "",             // object name
        0,              // get_removed
        0,              // db_lock
        object_types
    );
    for (res_it = res->begin();
        res_it != res->end();
        res_it++)
    {
        ObjectSptr          object = *res_it;
        ObjectSessionSptr   session;

        session = dynamic_pointer_cast<ObjectSession>(
            object
        );
        PWARN("send action:\n"
            "  src object ID:    '%s'\n"
            "  dst object ID:    '%s'\n"
            "  action:           '%s'\n",
            getId().c_str(),
            session->getId().c_str(),
            action->serialize().c_str()
        );

        // add action
        Object::addNeigh(action, object);
        context->transactionAddForSave(action);
    }
}

void Object::addAction(
    ObjectActionSptr a_ev)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectLinkActionSptr link;
    OBJECT_LINK_ACTION(link);
    addNeigh(a_ev.get(), link);
}

ObjectActionSptr Object::addAction(
    const string    &a_name)
{
    ObjectActionSptr action;
    OBJECT_ACTION(action, a_name);
    addAction(action);

    return action;
}

void Object::getActions(
    ObjectVectorSptr    a_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUniqueSptr    object_types;

    PROP_UNIQUE(object_types);
    object_types->add("core.object.action");

    getNeighs(
        a_out,          // out
        "",             // name
        0,              // get removed
        object_types
    );
}

void Object::getUnlinkActions(
    ObjectVectorSptr    a_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    actions;
    ObjectVectorIt      actions_it;

    PROP_VECTOR(actions);

    getActions(actions);

    for (actions_it = actions->begin();
        actions_it != actions->end();
        actions_it++)
    {
        ObjectActionSptr    action;
        action = dynamic_pointer_cast<ObjectAction>(
            *actions_it
        );
        action->neighUnlinkAll(1);
        a_out->add(action);
    }
}

// ---------------- signals ----------------

void Object::addSignal(
    ObjectMapSptr a_signal)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    signals;

    signals = getSignals();
    signals->add(a_signal);
}

ObjectMapSptr Object::addSignal(
    const string &a_signal)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr signal;

    PROP_MAP(signal);

    signal->add("name", a_signal);
    addSignal(signal);
    schedule();

    return signal;
}

void Object::clearSignals()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    empty;
    PROP_VECTOR(empty);
    setSignals(empty);
}

ObjectVectorSptr Object::getClearSignals()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr ret;
    ret = getSignals();
    clearSignals();
    return ret;
}


// ---------------- events ----------------

void Object::addEvent(
    ObjectMapSptr a_event)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr  events;

    events = getEvents();
    events->add(a_event);
}

ObjectMapSptr Object::addEvent(
    const string &a_event)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr event;

    PROP_MAP(event);

    event->add("name", a_event);
    addEvent(event);
    schedule();

    return event;
}

void Object::clearEvents()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    empty;
    PROP_VECTOR(empty);
    setEvents(empty);
}

ObjectVectorSptr Object::getClearEvents()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr ret;
    ret = getEvents();
    clearEvents();
    return ret;
}
*/

// ---------------- BSON ----------------

int32_t Object::parseBSON(
    mongo::BSONObj  a_obj)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;
    string  id;
    string  type;

    // Lock
    if (a_obj.hasField("lock")){
        ObjectStringSptr    prop_lock;
        string              lock;

        lock = a_obj.getStringField("lock");
        PROP_STRING(prop_lock, lock);

        INIT_PROP(this, Lock, prop_lock);
    }

    // flags
    if (a_obj.hasField("object_flags")){
        ObjectUint32Sptr    prop_flags;
        uint32_t            flags = 0;

        flags = uint32_t(a_obj.getIntField("object_flags"));
        PROP_UINT32(prop_flags, flags);

        INIT_PROP(this, ObjectFlags, prop_flags);
    }

    // ID
    if (a_obj.hasField("_id")){
        mongo::BSONElement  el;

        a_obj.getObjectID(el);
        el.Val(id);
        setId(id);
    }

    // Type
    if (a_obj.hasField("type")){
        type = a_obj.getStringField("type");
    }

    if (    !id.size()
        ||  !type.size())
    {
        goto fail;
    }

    // TimeCreate
    if (a_obj.hasField("time_create")){
        ObjectTimeSptr  prop_time;
        double          val = 0.0f;

        val = a_obj.getField("time_create").Double();
        PROP_TIME(prop_time, val);

        INIT_PROP(this, TimeCreate, prop_time);
    }

    // TimeModify
    if (a_obj.hasField("time_modify")){
        ObjectTimeSptr  prop_time;
        double          val = 0.0f;

        val = a_obj.getField("time_modify").Double();
        PROP_TIME(prop_time, val);

        INIT_PROP(this, TimeModify, prop_time);
    }

    // TimeRemove
    if (a_obj.hasField("time_remove")){
        ObjectTimeSptr prop_time;
        double         time_remove = 0.0f;

        time_remove = a_obj.getField("time_remove").Double();
        PROP_TIME(prop_time, time_remove);

        INIT_PROP(this, TimeRemove, prop_time);
    }

    // Name
    if (a_obj.hasField("name")){
        ObjectStringSptr    prop_name;
        string              name;

        name = a_obj.getStringField("name");
        PROP_STRING(prop_name, name);

        INIT_PROP(this, Name, prop_name);
    }

    // Descr
    if (a_obj.hasField("descr")){
        ObjectStringSptr    prop_descr;
        string              descr;

        descr = a_obj.getStringField("descr");
        PROP_STRING(prop_descr, descr);

        INIT_PROP(this, Descr, prop_descr);
    }

    // Title
    if (a_obj.hasField("title")){
        ObjectStringSptr    prop_title;
        string              title;

        title = a_obj.getStringField("title");
        PROP_STRING(prop_title, title);

        INIT_PROP(this, Title, prop_title);
    }

    // Url
    if (a_obj.hasField("url")){
        ObjectStringSptr    prop_url;
        string              url;

        url = a_obj.getStringField("url");
        PROP_STRING(prop_url, url);

        INIT_PROP(this, Url, prop_url);
    }

    // Links
    if (a_obj.hasField("links")){
        ObjectMapSptr       links;
        mongo::BSONElement  el;
        mongo::BSONObj      links_obj;

        PROP_MAP(links);

        el = a_obj.getField("links");
        if (mongo::BSONType::Object != el.type()){
            PERROR("cannot parse 'links' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(links_obj);

        DbMongo::bsonToProps(
            links_obj,
            links
        );

        INIT_PROP(this, LinksInfo, links);

/*
        ObjectMapSptr                           links_map;
        list< mongo::BSONElement >              els;
        list< mongo::BSONElement >::iterator    els_it;
        mongo::BSONElement                      el;
        mongo::BSONObj                          links_obj;

        PROP_MAP(links_map);

        el = a_obj.getField("links");
        if (mongo::BSONType::Object != el.type()){
            PERROR("cannot parse 'links' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(links_obj);
        links_obj.elems(els);
        for (els_it = els.begin();
            els_it != els.end();
            els_it++)
        {
            // for every neigh
            string          neigh_id    = els_it->fieldName();
            mongo::BSONType type        = els_it->type();
            mongo::BSONObj  neigh_obj;
            ObjectMapSptr   neigh_map;

            PROP_MAP(neigh_map);

            list< mongo::BSONElement >              els2;
            list< mongo::BSONElement >::iterator    els2_it;
            mongo::BSONElement                      el2;
            mongo::BSONObj                          links_obj;

            els_it->Val(links_obj);

            // check what neigh is object
            if (mongo::BSONType::Object != type){
                PERROR("cannot parse 'links' attribute,"
                    " object dump: '%s'\n",
                    links_obj.toString()->c_str()
                );
                goto fail;
            }

            // create object - neigh_id, it will be 'map' too
            links_map->add(
                neigh_id,
                neigh_map
            );
//
//          el2 = links_obj.getField(neigh_id);
//          if (mongo::BSONType::Object != el2.type()){
//              PERROR("cannot parse neigh ID: '%s'"
//                  " attribute, el type: '%d', "
//                  " object dump: '%s'\n",
//                  neigh_id.c_str(),
//                  el2.type(),
//                  a_obj.toString()->c_str()
//              );
//              goto fail;
//          }
//

            // get "type -> link" object
//            el2.Val(links_obj);
            links_obj.elems(els2);

            for (els2_it = els2.begin();
                els2_it != els2.end();
                els2_it++)
            {
                // for every neigh
                string              link_type = els2_it->fieldName();
                mongo::BSONType     type      = els2_it->type();
                mongo::BSONObj      link_obj;
                ObjectLinkSptr      link;

                // check what neigh is object
                if (mongo::BSONType::Object != type){
                    PERROR("cannot parse 'link type' value,"
                        " object dump: '%s'\n",
                        a_obj.toString()->c_str()
                    );
                    goto fail;
                }

                // get link object
                els2_it->Val(link_obj);

                // link_type
                if ("info" == link_type){
                    ObjectLinkInfoSptr link_info;
                    PROP_LINK_INFO(link_info);
                    link = link_info;

                    // walk_count
                    if (neigh_obj.hasField("walkers")){
                        ObjectMapSptr                         walkers_map;
                        list< mongo::BSONElement >            els;
                        list< mongo::BSONElement >::iterator  els_it;
                        mongo::BSONElement                    el;
                        mongo::BSONObj                        walkers_obj;

                        PROP_MAP(walkers_map);

                        el = neigh_obj.getField("walkers");
                        if (mongo::BSONType::Object != el.type()){
                            PERROR("cannot parse 'walkers' attribute,"
                                " object dump: '%s'\n",
                                a_obj.toString()->c_str()
                            );
                            goto fail;
                        }

                        el.Val(walkers_obj);
                        walkers_obj.elems(els);
                        for (els_it = els.begin();
                            els_it != els.end();
                            els_it++)
                        {
                            // for every walker
                            string          walker_id   = els_it->fieldName();
                            mongo::BSONType type        = els_it->type();
                            mongo::BSONObj  walker_obj;
                            ObjectMapSptr   walker_map;

                            PROP_MAP(walker_map);

                            // check what walker is object
                            if (mongo::BSONType::Object != type){
                                PERROR("cannot parse 'walkers' attribute,"
                                    " object dump: '%s'\n",
                                    a_obj.toString()->c_str()
                                );
                                goto fail;
                            }

                            // create object - walkers_id, it will be 'map' too
                            walkers_map->add(
                                walker_id,
                                walker_map
                            );

                            // get walker object
                            els_it->Val(walker_obj);

                            // walk_count
                            if (walker_obj.hasField("walk_count")){
                                int32_t         walk_count = 0;
                                ObjectInt32Sptr prop_walk_count;

                                walk_count = walker_obj.getIntField("walk_count");
                                PROP_INT32(
                                    prop_walk_count,
                                    walk_count
                                );

                                walker_map->add(
                                    "walk_count",
                                    prop_walk_count
                                );
                            }
                        }
                        // add to link
                        link_info->setWalkers(walkers_map);
                    }
                } else if ("event" == link_type){
                    ObjectLinkActionSptr link_event;
                    PROP_LINK_EVENT(link_event);
                    link = link_event;
                } else {
                    PFATAL("unsupported object link type: '%s'\n",
                        link_type.c_str()
                    );
                }

                // link_creator
                if (link_obj.hasField("link_creator")){
                    string link_creator = link_obj.getStringField("link_creator");
                    link->setLinkCreator(link_creator);
                }

                // create object - link_type, it will be 'map' too
                neigh_map->add(
                    link_type,
                    link
                );
            }
        }

        INIT_PROP(this, LinksInfo, links_map);
*/
    }

    // Position
    if (a_obj.hasField("pos")){
        ObjectVec3Sptr object_pos;
        object_pos = DbMongo::getMapFieldAsPropVec3(
            a_obj,
            "pos"
        );
        INIT_PROP(this, Pos, object_pos);
    }

    // Position of view
    if (a_obj.hasField("view_pos")){
        ObjectVec3Sptr object_view_pos;
        object_view_pos = DbMongo::getMapFieldAsPropVec3(
            a_obj,
            "view_pos"
        );
        INIT_PROP(this, ViewPos, object_view_pos);
    }

    // Position of Up
    if (a_obj.hasField("view_up")){
        ObjectVec3Sptr object_view_up;
        object_view_up = DbMongo::getMapFieldAsPropVec3(
            a_obj,
            "view_up"
        );
        INIT_PROP(this, ViewUp, object_view_up);
    }

    // strafe vector
    if (a_obj.hasField("view_strafe")){
        ObjectVec3Sptr object_view_strafe;
        object_view_strafe = DbMongo::getMapFieldAsPropVec3(
            a_obj,
            "view_strafe"
        );
        INIT_PROP(this, ViewStrafe, object_view_strafe);
    }

    // zoom val
    if (a_obj.hasField("view_zoom")){
        ObjectDoubleSptr    prop_view_zoom;
        double              view_zoom = 0.0f;

        view_zoom = a_obj.getField("view_zoom").Double();
        PROP_DOUBLE(prop_view_zoom, view_zoom);

        INIT_PROP(this, ViewZoom, prop_view_zoom);
    }

    // roll yaw pitch
    if (a_obj.hasField("view_ryp")){
        ObjectVec3Sptr object_view_ryp;
        object_view_ryp = DbMongo::getMapFieldAsPropVec3(
            a_obj,
            "view_ryp"
        );
        INIT_PROP(this, ViewRYP, object_view_ryp);
    }

/*
    // signals
    if (a_obj.hasField("signals")){
        ObjectVectorSptr    signals;
        mongo::BSONElement  el;
        mongo::BSONObj      signals_obj;

        PROP_VECTOR(signals);

        el = a_obj.getField("signals");
        if (mongo::BSONType::Array != el.type()){
            PERROR("cannot parse 'signals' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(signals_obj);

        DbMongo::bsonToProps(
            signals_obj,
            signals
        );

        INIT_PROP(this, Signals, signals);
    }

    // events
    if (a_obj.hasField("events")){
        ObjectVectorSptr    events;
        mongo::BSONElement  el;
        mongo::BSONObj      events_obj;

        PROP_VECTOR(events);

        el = a_obj.getField("events");
        if (mongo::BSONType::Array != el.type()){
            PERROR("cannot parse 'events' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(events_obj);

        DbMongo::bsonToProps(
            events_obj,
            events
        );

        INIT_PROP(this, Events, events);
    }
*/

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void Object::parseBuff(
    const char      *a_data,
    const int64_t   &a_size)
{
    PFATAL("method not implemented\n");
}

/*
string Object::serializeWithParams(
    const string    &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string          ret;
    ObjectMapSptr   obj;

    PROP_MAP(obj);

    // get object info
    serializeToObjectMap(obj, 1);

    // serialize it
    ret = obj->serialize(a_delim);

    return ret;
}


void Object::serializeToObjectMap(
    ObjectMapSptr   a_out,
    const int32_t   &a_serialize_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   props;
    ObjectMapIt     props_it;
    ObjectMapSptr   params;
    ObjectMapIt     params_it;
    string          ret;
    string          data_objects;
    string          data_params;

    if (!a_out){
        PFATAL("missing argument a_out\n");
    }

    PROP_MAP(props);
    PROP_MAP(params);

    // get props
    getAllProps(props);
    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        a_out->add(props_it->first, props_it->second);
    }

    // get params (dynamic props)
    getParams(params);
    for (params_it = params->begin();
        params_it != params->end();
        params_it++)
    {
        ObjectMapSptr props;
        PROP_MAP(props);
        params_it->second->getAllProps(props);
        a_out->add(params_it->first, props);
    }
}
*/

string Object::serialize(
    const string    &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string          ret;
    ObjectMapSptr   props;

    PROP_MAP(props);
    getAllProps(props);

    ret = props->serialize(a_delim);

/*
    // get object info
    serializeToObjectMap(obj, 0);

    // serialize it
    ret = obj->serialize(a_delim);
*/

    return ret;
}

void Object::getAllPropsDirty(
    ObjectMapSptr   a_out)
{
    lock(m_mutex, a_out->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard2(a_out->m_mutex,  adopt_lock);

    ObjectMapSptr   props;
    ObjectMapIt     props_it;

    PROP_MAP(props);

    getAllProps(props);

    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectSptr cur_prop_name = props_it->first;
        ObjectSptr cur_prop_val  = props_it->second;
        if (    cur_prop_name->isDirty()
            ||  cur_prop_val->isDirty())
        {
            a_out->add(cur_prop_name, cur_prop_val);
        }
    }
}

bool Object::isDirty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    bool ret = false;

    if (Object::CREATE_OBJECT & m_flags){
        // object
        ObjectMapSptr props;
        PROP_MAP(props);
        getAllPropsDirty(props);
        if (props->size()){
            ret = true;
        }
    } else {
        // prop
        ret = m_is_dirty;
    }

    return ret;
}

ObjectStringSptr Object::toString()
{
    return serialize();
}

string Object::getView(
    const ObjectMapSptr a_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr  object_view;
    string      view = "widget";

    // get template view
    if (a_params){
        object_view = a_params->get("view");
        if (object_view){
            view = object_view->toString();
        }
    }

    return view;
}

ObjectTemplateSptr Object::getTemplate(
    const ObjectMapSptr a_req_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr          object_webgl;
    ObjectTemplateSptr  tpl;

    PROP_TEMPLATE(tpl);
    tpl->setObject(this);

    // process webgl
    if (a_req_props){
        object_webgl = a_req_props->get("webgl");
        if (object_webgl){
            tpl->setPrefix("webgl");
        }
    }

    return tpl;
}

void Object::render(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PTIMING(0.010f, "render: '%s'\n",
        getId().c_str()
    );

    string              html;
    ObjectSptr          object_view;
    ObjectTemplateSptr  tpl     = getTemplate(a_req_props);
    string              view    = getView(a_req_params);
    string              id      = getId();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    PWARN("render: '%s'\n",
        a_req_props->toString()->c_str()
    );

    // process template
    html = tpl->render(id, view);
    a_answer->setAnswerBody(html);
}

void Object::renderEmbedded(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PTIMING(0.010f, "renderEmbedded: '%s'\n",
        getId().c_str()
    );

    string              html;
    ObjectSptr          object_view;
    ObjectTemplateSptr  tpl     = getTemplate(a_req_props);
    string              view    = getView(a_req_params);
    string              id      = getId();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    // process template
    html = tpl->renderEmbedded(id, view);

    PDEBUG(50, "renderEmbedded:\n"
        "  a_req_params:    '%s'\n"
        "  template prefix: '%s'\n"
        "  html:            '%s'\n",
        a_req_params->toString()->c_str(),
        tpl->getPrefix().c_str(),
        html.c_str()
    );

    a_answer->setAnswerBody(html);
}

/*
void Object::remove(
    const int32_t &a_force)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectContextSptr   context   = Thread::getCurContext();
    string              object_id = getId();

    // PTIMING(0.010f, "%s, a_force: '%d'\n",
    //    object_id.c_str(),
    //    a_force
    // );

    context->remove(this, a_force);
}
*/

void Object::strafe(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVec3Sptr  cur_pos         = getPos();
    ObjectVec3Sptr  cur_view_pos    = getViewPos();
    ObjectVec3Sptr  cur_view_strafe = getViewStrafe();

    ObjectVec3Sptr  new_pos;
    ObjectVec3Sptr  new_view_pos;

    PROP_VEC3(new_pos,      0.0f, 0.0f, 0.0f);
    PROP_VEC3(new_view_pos, 0.0f, 0.0f, 0.0f);

    // add strafe vector to position
    new_pos->setX(
        cur_pos->getX() + cur_view_strafe->getX() * a_val
    );
    new_pos->setZ(
        cur_pos->getZ() + cur_view_strafe->getZ() * a_val
    );
    new_pos->setY(
        cur_pos->getY() + cur_view_strafe->getY() * a_val
    );

    setPos(new_pos);

    PWARN("ObjectCamera::strafe\n"
        " a_val:            '%f'\n"
        " cur_pos:          '%s'\n"
        " cur_view_pos:     '%s'\n"
        " cur_view_strafe:  '%s'\n"
        " new_pos:          '%s'\n"
        " new_view_pos:     '%s'\n",
        a_val,
        cur_pos->toString()->c_str(),
        cur_view_pos->toString()->c_str(),
        cur_view_strafe->toString()->c_str(),
        new_pos->toString()->c_str(),
        new_view_pos->toString()->c_str()
    );

    //save();
}

void Object::accel(
    ObjectVec3Sptr  a_accel)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVec3Sptr     cur_pos;
    ObjectVec3Sptr     cur_view_pos;
    ObjectVec3Sptr     view_vector;
    ObjectVec3Sptr     new_pos;
    ObjectVec3Sptr     new_view_pos;

    if (!a_accel){
        PFATAL("missing argument: 'a_accel'\n");
    }

    cur_pos         = getPos();
    cur_view_pos    = getViewPos();

    PROP_VEC3(new_pos,      0.0f, 0.0f, 0.0f);
    PROP_VEC3(new_view_pos, 0.0f, 0.0f, 0.0f);

    PWARN("[ Object::accel ] calculate positions for:\n"
        "  accel:           '%s'\n"
        "  cur_pos:         '%s'\n"
        "  cur_view_pos:    '%s'\n",
        a_accel->toString()->c_str(),
        cur_pos->toString()->c_str(),
        cur_view_pos->toString().c_str()
    );

    // calculate new position
    new_pos->set0(cur_pos->get0() + a_accel->get0());
    new_pos->set1(cur_pos->get1() + a_accel->get1());
    new_pos->set2(cur_pos->get2() + a_accel->get2());

    // calculate new view position
    new_view_pos->set0(cur_view_pos->get0() + a_accel->get0());
    new_view_pos->set1(cur_view_pos->get1() + a_accel->get1());
    new_view_pos->set2(cur_view_pos->get2() + a_accel->get2());

    setPos(new_pos);
    setViewPos(new_view_pos);

    PWARN("[ Object::accel ] new positions for:\n"
        "  accel:           '%s'\n"
        "  cur_pos:         '%s'\n"
        "  cur_view_pos:    '%s'\n"
        "  new_pos:         '%s'\n"
        "  new_view_pos:    '%s'\n",
        a_accel->toString().c_str(),
        cur_pos->toString().c_str(),
        cur_view_pos->toString().c_str(),
        new_pos->toString().c_str(),
        new_view_pos->toString().c_str()
    );
}

ObjectVec3Sptr Object::getViewVector()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVec3Sptr     view_vector;
    ObjectVec3Sptr     cur_pos;
    ObjectVec3Sptr     cur_view_pos;

    PROP_VEC3(view_vector, 0.0f, 0.0f, 0.0f);

    cur_pos         = getPos();
    cur_view_pos    = getViewPos();

    // calculate view vector
    view_vector->set0(cur_view_pos->get0() - cur_pos->get0());
    view_vector->set1(cur_view_pos->get1() - cur_pos->get1());
    view_vector->set2(cur_view_pos->get2() - cur_pos->get2());

    view_vector = ObjectVec3::normalize(view_vector);

    return view_vector;
}

void Object::rotateViewVector(
    ObjectDoubleSptr  a_yaw,
    ObjectDoubleSptr  a_pitch)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVec3Sptr    view_vector;
    ObjectVec3Sptr    cur_pos;
    ObjectVec3Sptr    cur_view_pos;
    ObjectDoubleSptr  cur_view_zoom;
    ObjectVec3Sptr    cur_view_ryp;
    ObjectVec3Sptr    new_view_pos;
    ObjectVec3Sptr    new_view_ryp;

    if (    !a_yaw
        &&  !a_pitch)
    {
        PFATAL("at least one argument must be defined:"
            " 'a_yaw' or 'a_pitch'\n"
        );
    }

    cur_pos         = getPos();
    cur_view_pos    = getViewPos();
    cur_view_zoom   = getViewZoom();
    cur_view_ryp    = getViewRYP();
    view_vector     = getViewVector();

    PWARN("rotateViewVector:\n"
        "  yaw:             '%s'\n"
        "  pitch:           '%s'\n"
        "  view_vector:     '%s'\n"
        "  cur_pos:         '%s'\n"
        "  cur_view_pos:    '%s'\n"
        "  cur_view_zoom:   '%s'\n"
        "  cur_view_ryp:    '%s'\n",
        a_yaw   ? a_yaw->toString().c_str()   : "",
        a_pitch ? a_pitch->toString().c_str() : "",
        view_vector->toString().c_str(),
        cur_pos->toString().c_str(),
        cur_view_pos->toString().c_str(),
        cur_view_zoom->toString().c_str(),
        cur_view_ryp->toString().c_str()
    );

    PROP_VEC3(
        new_view_ryp,
        cur_view_ryp->getPitch(),
        cur_view_ryp->getYaw(),
        cur_view_ryp->getRoll()
    );

    if (a_yaw){
        double yaw = a_yaw->getVal();
        new_view_ryp->addYaw(yaw);
    }

    if (a_pitch){
        double pitch = a_pitch->getVal();
        new_view_ryp->addPitch(pitch);
    }

    // setup new Roll, Yaw, Pitch
    setViewRYP(new_view_ryp);

    // reread for debug
    new_view_pos = getViewPos();
    new_view_ryp = getViewRYP();

    PWARN("rotateViewVector:\n"
        "  yaw:             '%s'\n"
        "  pitch:           '%s'\n"
        "  new_view_pos:    '%s'\n"
        "  new_view_ryp:    '%s'\n",
        a_yaw   ? a_yaw->toString().c_str()   : "",
        a_pitch ? a_pitch->toString().c_str() : "",
        new_view_pos->toString().c_str(),
        new_view_ryp->toString().c_str()
    );
}

void Object::registerAction(
    const string    &a_name,
    ApiHandler      a_cb)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_action_handlers[a_name] = a_cb;
}

/*
void Object::registerSignal(
    const string    &a_name,
    void            (*a_cb)(
        Object *,
        ObjectHttpAnswerSptr,
        const ObjectMapSptr,  // a_req_props
        const ObjectMapSptr   // a_req_params
    ))
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_signal_handlers[a_name] = a_cb;
}
*/

/*
void Object::signal(
    const string    &a_method,
    ObjectMapSptr   a_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectHttpAnswerSptr    answer;
    ObjectMapSptr           req_props;
    ObjectMapSptr           req_params = a_params;

    PROP_HTTP_ANSWER(answer);
    PROP_MAP(req_props);

    if (!req_params){
        PROP_MAP(req_params);
    }

    req_props->add("name", a_method);

    signal(
        answer,
        req_props,
        req_params
    );
}
*/

/*
void Object::signal(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string              name;
    ObjectSptr          prop_name;
    ObjectContextSptr   context;
    ApiHandlersIt       it;

    void (*handler)(
        Object *,
        ObjectHttpAnswerSptr,
        const ObjectMapSptr,
        const ObjectMapSptr
    ) = NULL;

    // setup default values
    a_answer->setAnswerStatusCode(500);
    a_answer->setAnswerStatusString("Internal error");

    // get prop 'name'
    prop_name = a_req_props->get("name");
    if (prop_name){
        name = prop_name->toString();
    }

    // search context
//    context = Thread::getCurContext();

    // search handler
    it = m_signal_handlers.find(name);
    if (m_signal_handlers.end() != it){
        handler = it->second;
    }

    // process method
    if (handler){
        (*handler)(
            this,
            a_answer,
            a_req_props,
            a_req_params
        );
    } else {
        string answer;
        string status = "failed";
        string descr  = "unsupported signal";

        PERROR("unsupported signal: '%s'"
            " for object ID: '%s'\n",
            name.c_str(),
            getId().c_str()
        );

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }
}
*/

/*
void Object::registerEvent(
    const string    &a_name,
    void            (*a_cb)(
        Object *,
        ObjectHttpAnswerSptr,
        const ObjectMapSptr,  // a_req_props
        const ObjectMapSptr   // a_req_params
    ))
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_event_handlers[a_name] = a_cb;
}
*/

/*
void Object::event(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string              name;
    ObjectSptr          prop_name;
    ObjectContextSptr   context;
    ApiHandlersIt       it;

    void (*handler)(
        Object *,
        ObjectHttpAnswerSptr,
        const ObjectMapSptr,
        const ObjectMapSptr
    ) = NULL;

    // setup default values
    a_answer->setAnswerStatusCode(500);
    a_answer->setAnswerStatusString("Internal error");

    // get prop 'name'
    prop_name = a_req_props->get("name");
    if (prop_name){
        name = prop_name->toString();
    }

    // search context
//    context = Thread::getCurContext();

    // search handler
    it = m_event_handlers.find(name);
    if (m_event_handlers.end() != it){
        handler = it->second;
    }

    // process method
    if (handler){
        (*handler)(
            this,
            a_answer,
            a_req_props,
            a_req_params
        );
    } else {
        string answer;
        string status = "failed";
        string descr  = "unsupported event";

        PERROR("unsupported event: '%s'"
            " for object ID: '%s'\n",
            name.c_str(),
            getId().c_str()
        );

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }
}
*/

bool Object::hasLink(
    const string &a_object_id)
{
    bool ret = false;
    ret = m_LinksInfo->has(a_object_id);
    return ret;
}

void Object::getLinks(
    ObjectVectorSptr    a_out,
    const string        &a_name,
    const int32_t       &a_get_removed,
    const int32_t       &a_db_lock,
    ObjectUniqueSptr    a_link_types,
    ObjectUniqueSptr    a_skip)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr       links_info;
    ObjectMapIt         links_info_it;
    ObjectContextSptr   context   = Thread::getCurContext();
    string              object_id = getId();

    PTIMING(0.030f, "get links for: '%s'\n",
        object_id.c_str()
    );

//again:
    links_info = getLinksInfo();
    for (links_info_it = links_info->begin();
        links_info_it != links_info->end();
        links_info_it++)
    {
        string              link_id;
        ObjectSptr          link;
        string              link_type;
        ObjectStringSptr    prop_link_name;
        string              link_name;
        ObjectMapSptr       link_info;
        ObjectUniqueIt      link_types_it;
        string              neigh_type;

        link_id   = links_info_it->first->toString();
        link_info = dynamic_pointer_cast<ObjectMap>(
            links_info_it->second
        );

        if (a_skip){
            ObjectUniqueIt skip_it;
            skip_it = a_skip->find(link_id);
            if (a_skip->end() != skip_it){
                continue;
            }
        }

        // load object
        link = context->getObjectById(
            link_id,
            "",             // type
            a_get_removed,
            a_db_lock
        );
        if (!link){
            // link not found
            PWARN("object ID: '%s'"
                " cannot get link by ID: '%s',"
                " link will be skipped"
                "\n",
                object_id.c_str(),
                link_id.c_str()
            );
            continue;
        }

        // get link info
        link_type      = link->getType();
        prop_link_name = link->getName();
        if (prop_link_name){
            link_name = prop_link_name->toString();
        }

        if (a_link_types){
            int32_t found = 0;

            // we have requested neighs with some link type
            // check what we have such types
            for (link_types_it = a_link_types->begin();
                link_types_it != a_link_types->end();
                link_types_it++)
            {
                ObjectStringSptr    get_prop_link_type;
                string              get_link_type;

                get_prop_link_type = dynamic_pointer_cast<ObjectString>(
                    *link_types_it
                );
                if (!get_prop_link_type){
                    PWARN("link type is not ObjectString,"
                        " a_link_types dump below: '%s'\n",
                        a_link_types->toString().c_str()
                    );
                    continue;
                }
                get_link_type = get_prop_link_type->toString();

                // TODO XXX FIXME
                // add support match * here
                // maybe via perl?
                // 20141103 morik@
                if (get_link_type
                    == link_type.substr(0, get_link_type.size()))
                {
                    // type found
                    found = 1;
                    break;
                }
            }
            if (!found){
                continue;
            }
        }

        if (    link->isRemoved()
            &&  !a_get_removed)
        {
            // skip removed (as default)
            continue;
        }

        // get and check link name
        if (    a_name.size()
            &&  a_name != link_name)
        {
            continue;
        }

        // collect
        a_out->add(link);

        PDEBUG(50, "getLinks,"
            " link_name: '%s' (%s)"
            " neigh_id: '%s'\n",
            link_name.c_str(),
            link ? "found" : "not found",
            link_id.c_str()
        );
    }
}

void Object::getNeighs(
    ObjectVectorSptr    a_out,
    const string        &a_name,
    const int32_t       &a_get_removed,
    const int32_t       &a_db_lock,
    ObjectUniqueSptr    a_object_types,
    ObjectUniqueSptr    a_link_types,
    ObjectUniqueSptr    a_skip)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectContextSptr   context   = Thread::getCurContext();
    string              object_id = getId();
    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;

    PTIMING(0.030f, "get neighs for: '%s'\n",
        object_id.c_str()
    );

    PROP_VECTOR(links);

    getLinks(
        links,
        "",             // name
        a_get_removed,
        0,              // db_lock
        a_link_types,
        a_skip
    );

    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr          link = *links_it;
        ObjectVectorSptr    objects;
        ObjectSptr          object;
        size_t              size = 0;
        ObjectUniqueSptr    skip;
        string              object_type;

        PROP_VECTOR(objects);

        // skip ourself
        PROP_UNIQUE(skip);
        skip->add(object_id);

        link->getLinks(
            objects,
            a_name,
            a_get_removed,
            a_db_lock,
            a_link_types,
            skip
        );
        size = objects->size();

        if (0 == size){
            continue;
        }

        if (1 != size){
            PWARN("link: '%s'\n"
                "has invalid neighs count: '%d'"
                " and will be skipped\n",
                link->getId().c_str(),
                size
            );
            continue;
        }

        object      = *objects->begin();
        object_type = object->getType();

        if (    a_object_types
            &&  !a_object_types->empty())
        {
            if (!a_object_types->has(object_type)){
                // skip if object type not matched
                continue;
            }
        }

        // we have found it
        a_out->add(object);
    }
}

void Object::neighUnlink(
    ObjectSptr      a_object_src,
    const string    &a_neigh_id,
    const int32_t   &a_force)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectContextSptr   context = Thread::getCurContext();
    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    if (a_neigh_id.empty()){
        PFATAL("missing argument: 'a_neigh_id'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    PROP_VECTOR(links);
    a_object_src->getLinks(
        links,  // out
        "",     // name
        0,      // get removed
        1       // db_lock
    );

    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr  link    = *links_it;
        string      link_id = link->getId();
        ObjectSptr  object;

        if (link->hasLink(a_neigh_id)){
            // remove from links info
            //if (a_force){
            //    Object::removeLink(a_object_src, link_id);
            //}
            // remove link
            Object::remove(link, a_force);
        }
    }
}

ObjectSptr Object::_createNeigh(
    ObjectSptr      a_object_src,
    const string    &a_type,
    const string    &a_name,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectSptr          object_dst;
    ObjectContextSptr   context = Thread::getCurContext();

    PTIMING(0.030f, "'%s' create neigh"
        " with type: '%s',"
        " name: '%s'\n",
        a_object_src->getId().c_str(),
        a_type.c_str(),
        a_name.c_str()
    );

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    object_dst = Object::_createObject(
        a_type,
        a_do_autorun
    );
    if (!a_name.empty()){
        object_dst->setName(a_name);
    }

    Object::addNeigh(a_object_src, object_dst);

    context->transactionAddForSave(a_object_src);
    context->transactionAddForSave(object_dst);

    return object_dst;
}

ObjectSptr Object::_getNeigh(
    ObjectSptr          a_object_src,
    const string        &a_name,
    const string        &a_type,
    const int32_t       &a_get_removed,
    const int32_t       &a_db_lock,
    ObjectUniqueSptr    a_link_types,
    ObjectUniqueSptr    a_skip)
{
    ObjectSptr          ret;
    ObjectVectorSptr    res;
    ObjectUniqueSptr    object_types;
    uint32_t            size = 0;

    PTIMING(0.030f, "'%s' get neigh"
        " with type: '%s',"
        " name: '%s'\n",
        a_object_src->getId().c_str(),
        a_type.c_str(),
        a_name.c_str()
    );

    if (!a_type.empty()){
        PROP_UNIQUE(object_types);
        object_types->add(a_type);
    }

    PROP_VECTOR(res);

    a_object_src->getNeighs(
        res,
        a_name,
        a_get_removed,
        a_db_lock,
        object_types,
        a_link_types,
        a_skip
    );

    size = res->size();

    if (0 == size){
    } else if (1 == size){
        ret = res->at(0);
    } else {
        ret = res->at(0);
        PWARN("more than 1 neigh found (%d):\n"
            "  object_id:       '%s'\n"
            "  object_name:     '%s'\n"
            "  object_type:     '%s'\n"
            "  a_get_removed:   '%d'\n"
            "  a_db_lock:       '%d'\n",
            size,
            a_object_src->getId().c_str(),
            a_name.c_str(),
            a_type.c_str(),
            a_get_removed,
            a_db_lock
        );
    }

    return ret;
}

ObjectSptr Object::_getCreateNeigh(
    ObjectSptr      a_object_src,
    const string    &a_type,
    const string    &a_name,
    const int32_t   &a_get_removed,
    const int32_t   &a_db_lock,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectSptr  ret;

    PTIMING(0.030f, "'%s' get/create neigh"
        " with type: '%s',"
        " name: '%s'\n",
        a_object_src->getId().c_str(),
        a_type.c_str(),
        a_name.c_str()
    );

    ret = Object::_getNeigh(
        a_object_src,
        a_name,
        a_type,
        a_get_removed,
        a_db_lock
    );
    if (!ret){
        ret = Object::_createNeigh(
            a_object_src,
            a_type,
            a_name,
            a_do_autorun
        );
        if (!ret){
            PFATAL("cannot create neigh"
                " with type: '%s',"
                " name:'%s',"
                " get_removed: '%d'\n",
                a_type.c_str(),
                a_name.c_str(),
                a_get_removed
            );
        }
    }
    return ret;
}

ObjectLinkSptr Object::addNeigh(
    ObjectSptr      a_object_src,
    ObjectSptr      a_object_dst,
    const int32_t   &a_create_backlink,
    ObjectLinkSptr  a_link)
{
    lock(a_object_src->m_mutex, a_object_dst->m_mutex);
    lock_guard<recursive_mutex> guard1(a_object_src->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_object_dst->m_mutex, adopt_lock);

    ObjectLinkSptr  new_link = a_link;
    ObjectMapSptr   links_info_src;
    ObjectMapSptr   links_info_new_link;
    ObjectMapSptr   links_info_dst;

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    if (!a_object_dst){
        PFATAL("missing argument: 'a_object_dst'\n");
    }

    // check that objects locked in DB
    a_object_src->checkDbLock();
    a_object_dst->checkDbLock();

    links_info_src = a_object_src->getLinksInfo();
    links_info_dst = a_object_dst->getLinksInfo();

    if (!new_link){
        OBJECT_LINK(new_link);
    }
    links_info_new_link = new_link->getLinksInfo();

    if (a_create_backlink){
        // called from not ObjectLink*
        ObjectMapSptr   debug_info;

        // 1 <-> 2 <-> 3

        // 1 <-> 2
        PROP_MAP(debug_info);
        debug_info->add("dst_type", new_link->getType());
        links_info_src->add(new_link->getId(), debug_info);

        PROP_MAP(debug_info);
        debug_info->add("dst_type", a_object_src->getType());
        links_info_new_link->add(
            a_object_src->getId(),
            debug_info
        );

        // 2 <-> 3
        PROP_MAP(debug_info);
        debug_info->add("dst_type", a_object_dst->getType());
        links_info_new_link->add(
            a_object_dst->getId(),
            debug_info
        );

        PROP_MAP(debug_info);
        debug_info->add("dst_type",new_link->getType());
        links_info_dst->add(new_link->getId(), debug_info);

        Object::save(new_link);
        Object::save(a_object_src);
        Object::save(a_object_dst);
    } else {
        // called from ObjectLink*
        ObjectMapSptr debug_info;

        // 1 <-> 2 -> 3

        // 1 <-> 2
        PROP_MAP(debug_info);
        debug_info->add("dst_type", new_link->getType());
        links_info_src->add(new_link->getId(), debug_info);

        PROP_MAP(debug_info);
        debug_info->add("dst_type", a_object_src->getType());
        links_info_new_link->add(
            a_object_src->getId(),
            debug_info
        );

        // 2 -> 3
        PROP_MAP(debug_info);
        debug_info->add("dst_type", a_object_dst->getType());
        links_info_new_link->add(
            a_object_dst->getId(),
            debug_info
        );

        Object::save(new_link);
        Object::save(a_object_src);
    }

    return new_link;
}

void Object::neighUnlinkAll(
    ObjectSptr      a_object_src,
    const int32_t   &a_force)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    PROP_VECTOR(links);
    a_object_src->getLinks(links);

    // note, all removing must going via removeLink method,
    // because removeLink must handling
    // REMOVE_WHEN_LINKS_LOST* object flags
    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectSptr  link    = *links_it;
        string      link_id = link->getId();
        ObjectSptr  object;

        // remove from links info
        Object::removeLink(a_object_src, link_id);

        // remove link
        Object::remove(link, a_force);
    }

    if (!links->empty()){
        Object::save(a_object_src);
    }
}

void Object::removeLink(
    ObjectSptr      a_object_src,
    const string    &a_link_id)
{
    lock_guard<recursive_mutex> guard(a_object_src->m_mutex);

    ObjectContextSptr   context;
    ObjectMapSptr       links_info;
    int32_t             links_before = 0;
    int32_t             links_after  = 0;
    uint32_t            flags        = 0;

    if (!a_object_src){
        PFATAL("missing argument 'a_object_src'\n");
    }

    if (a_link_id.empty()){
        PFATAL("missing argument 'a_link_id'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    context      = Thread::getCurContext();
    links_info   = a_object_src->getLinksInfo();
    links_before = links_info->size();

    links_info->erase(a_link_id);

    links_after = links_info->size();
    flags       = a_object_src->getObjectFlags()->getVal();

    if (    links_before
        &&  !links_after)
    {
        if (flags & Object::REMOVE_WHEN_LINKS_LOST){
            PWARN("'%s' (type: '%s') all links lost,"
                " object will be removed (force = 0)\n",
                a_object_src->getId().c_str(),
                a_object_src->getType().c_str()
            );
            Object::remove(a_object_src);
        }
        if (flags & Object::REMOVE_WHEN_LINKS_LOST_FORCE){
            PWARN("'%s' (type: '%s') all links lost,"
                " object will be removed (force = 1)\n",
                a_object_src->getId().c_str(),
                a_object_src->getType().c_str()
            );
            Object::remove(a_object_src, 1);
        }
    } else {
        Object::save(a_object_src);
    }
}

void Object::remove(
    ObjectSptr      a_object_src,
    const int32_t   &a_force)
{
    ObjectContextSptr   context;
    context = Thread::getCurContext();

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    // check that object locked in DB
    a_object_src->checkDbLock();

    context->remove(a_object_src, a_force);
}

void Object::save(
    ObjectSptr  a_object_src)
{
    ObjectContextSptr   context;
    context = Thread::getCurContext();

    // check that object locked in DB
    a_object_src->checkDbLock();

    context->transactionAddForSave(a_object_src);
}

/*
ObjectLinkSptr Object::addNeigh(
    ObjectSptr      a_object_src,
    ObjectSptr      a_object_dst,
    const int32_t   &a_create_backlink,
    ObjectLinkSptr  a_link)
{
    lock(a_object_src->m_mutex, a_object_dst->m_mutex);
    lock_guard<recursive_mutex> guard1(a_object_src->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_object_dst->m_mutex, adopt_lock);

    ObjectContextSptr context = Thread::getCurContext();
    ObjectLinkSptr    link    = a_link;

    if (!a_object_src){
        PFATAL("missing argument: 'a_object_src'\n");
    }

    if (!a_object_dst){
        PFATAL("missing argument: 'a_object_dst'\n");
    }

    if (a_create_backlink){
        // called from not ObjectLink*
        if (!link){
            OBJECT_LINK(link);
        }

        ObjectMapSptr debug_info;
        PROP_MAP(debug_info);
        debug_info->add("dst_type", link->getType());
        m_LinksInfo->add(link->getId(), debug_info);

        link->setup(this, a_object);
        link->save();
    } else {
        // called from ObjectLink*
        ObjectMapSptr debug_info;
        PROP_MAP(debug_info);
        debug_info->add("dst_type", a_object->getType());
        m_LinksInfo->add(a_object->getId(), debug_info);
    }

    save();

    return link;
}

ObjectLinkSptr Object::addNeigh(
    ObjectSptr      a_object,
    ObjectLinkSptr  a_link,
    const int32_t   &a_create_backlink)
{
    ObjectLinkSptr link;

    link = addNeigh(
        a_object.get(),
        a_link,
        a_create_backlink
    );

    return link;
}

void Object::getParams(
    ObjectMapSptr   a_out,
    const string    &a_name,
    const int32_t   a_get_removed)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    res;
    ObjectVectorIt      res_it;
    ObjectUniqueSptr    object_types;

    PROP_VECTOR(res);
    PROP_UNIQUE(object_types);

    object_types->add("core.object.param");

    getNeighs(
        res,
        a_name,         // name
        a_get_removed,  // get removed
        object_types
    );

    for (res_it = res->begin();
        res_it != res->end();
        res_it++)
    {
        ObjectParamSptr cur_param;
        ObjectSptr      name;

        cur_param = dynamic_pointer_cast<ObjectParam>(*res_it);
        name      = cur_param->getName();
        if (name){
            a_out->add(name->toString(), cur_param);
        } else {
            PWARN("empty param name, for object"
                " with ID: '%s'\n",
                cur_param->getId().c_str()
            );
        }
    }
}

int32_t Object::saveParams()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         err = 0;
    ObjectMapSptr   params;
    ObjectMapIt     params_it;
    string          object_id = getId();

    PTIMING(0.010f, "'%s'\n",
        object_id.c_str()
    );

    // TODO XXX FIXME
    // params temporary disabled because of:
    // [ TIMING 4.77505 ms ] [core/object/object/main.cpp:784 (saveProps)] 'static.http.server'
    // [ TIMING 261.949 ms ] [core/object/object/main.cpp:2467 (saveParams)] 'static.http.server'
    // [ TIMING 271.387 ms ] [core/object/object/main.cpp:647 (_save)] save object: 'static.http.server'
    // [ TIMING 271.497 ms ] [core/object/transaction/main.cpp:221 (commit)] commit object: 'static.http.server'

//
//  int32_t res;
//
//  PROP_MAP(params);
//
//  // request all params
//  getParams(params);
//
//  for (params_it = params->begin();
//      params_it != params->end();
//      params_it++)
//  {
//      res = params_it->second->save();
//      if (res){
//          err = res;
//      }
//  }
//
    return err;
}
*/

void Object::beforeRequest(
    ObjectHttpAnswerSptr    a_answer,
    const ObjectMapSptr     a_req_props,
    const ObjectMapSptr     a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    //ObjectContextSptr   context;
    //ObjectUserSptr      cur_user;

    // get current user
    //cur_user = ObjectContext::getCurUser();

    // create shop object
    //cur_user->getCreateShopObject();
}

void Object::updateSphere()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVec3Sptr   cur_pos       = getPos();
    ObjectVec3Sptr   cur_view_pos  = getViewPos();
    ObjectVec3Sptr   cur_view_up   = getViewUp();
    ObjectDoubleSptr cur_view_zoom = getViewZoom();
    ObjectVec3Sptr   cur_view_ryp  = getViewRYP();
    double           pitch         = cur_view_ryp->getPitch();
    double           yaw           = cur_view_ryp->getYaw();
    double           zoom          = cur_view_zoom->getVal();

    ObjectVec3Sptr    cross;
    ObjectVec3Sptr    new_view_pos;

    // 1. update view
    PROP_VEC3(
        new_view_pos,
        cur_pos->getX(),
        cur_pos->getY(),
        cur_pos->getZ()
    );

    new_view_pos->addZ(zoom * cos(-yaw) * cos(pitch));
    new_view_pos->addX(zoom * sin(-yaw) * cos(pitch));
    new_view_pos->addY(zoom * sin(pitch));

    setViewPos(new_view_pos);

    // 2. update cross (only after view update);
    cross = ObjectVec3::cross(
        new_view_pos,
        cur_pos,
        cur_view_up
    );

    cross = ObjectVec3::normalize(cross);
    setViewStrafe(cross);
}

// ---------------- api ----------------

void Object::actionLoad(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*

    ObjectMapSptr answer_params;

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    a_object->getAllProps(answer_params);

    // get object info
    //a_object->serializeToObjectMap(
    //    answer_params,  // object
    //    1               // serialize params
    //);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was loaded successfully"
    );
*/
}

void Object::actionDelete(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*

    // TODO check perm here
    a_object->remove();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was deleted successfully"
    );
*/
}

void Object::actionBookmark(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
    ObjectSessionSptr   session;
    ObjectContextSptr   context;
    ObjectUserSptr      user;

    user = Context::getCurUser();
    if (!user){
        goto no_auth;
    }

    // all ok, link
    user->addNeigh(a_object);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "there are no errors"
    );

out:
    return;

no_auth:
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("failed");
    a_answer->setAnswerApiDescr(
        "you are not authorized"
    );
    goto out;
*/
}

void Object::actionGetEvents(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
    ObjectMapSptr   answer_params;
    ObjectMapSptr   events;
    ObjectSptr      object_req_id;
    string          req_id;

    // TODO check perms here

    PROP_MAP(events);

    // search req_id
    object_req_id = a_req_props->get("req_id");
    if (object_req_id){
        req_id = object_req_id->toString();
    }
    if (req_id.size()){
        // donot disconnect
        a_answer->setAnswerApiReqId(req_id);
        a_answer->setWaitingReqId(req_id);
    }
*/
}

void Object::actionGetNeighs(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
    ObjectVectorSptr                neighs_ids;
    ObjectMapSptr                   answer_params;
    ObjectSptr                      object_type;
    ObjectMapSptr                   object_types;
    string                          type;
    vector<ObjectSptr>              neighs;
    vector<ObjectSptr>::iterator    neighs_it;

    // TODO check perms here

    PROP_VECTOR(neighs_ids);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    if (a_req_params){
        object_type = a_req_params->get("type");
    }
    if (object_type){
        type = object_type->toString();
    }
    if (!type.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param 'type'"
        );
        goto fail;
    }

    PROP_MAP(object_types);
    object_types->add(type, "");

    a_object->getNeighs(
        neighs,
        "",                     // name
        0,                      // get_removed
        types                   // object types
    );

    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        ObjectSptr      cur_neigh = *neighs_it;
        string          id        = cur_neigh->getId();
        neighs_ids->push_back(id);
    }

    answer_params->add("neighs", neighs_ids);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");

out:
    return;
fail:
    goto out;
*/
}

void Object::actionGetCreateNeigh(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
//    int32_t             res;
    ObjectVectorSptr    neighs_ids;
    ObjectMapSptr       answer_params;
    ObjectSptr          object_type;
    string              type;
    ObjectSptr          object_name;
    string              name;
    ObjectSptr          neigh;
    ObjectMapSptr       neigh_objects;

    string              status;
    string              descr;

    // TODO check perms here

    PROP_VECTOR(neighs_ids);
    PROP_MAP(neigh_objects);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    // type
    if (a_req_params){
        object_type = a_req_params->get("type");
    }
    if (object_type){
        type = object_type->toString();
    }
    if (!type.size()){
        status = "failed";
        descr  = "missing param: 'type'";
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
        goto fail;
    }

    // name
    if (a_req_params){
        object_name = a_req_params->get("name");
    }
    if (object_name){
        name = object_name->toString();
    }

    neigh = a_object->getCreateNeigh(
        type,   // type
        name    // name
    );

    PWARN("actionGetCreateNeigh:\n"
        " type:     '%s'\n"
        " name:     '%s'\n"
        " is_new:   '%d'\n"
        " neigh ID: '%s'\n",
        type.c_str(),
        name.c_str(),
        neigh->isNew(),
        neigh->getId().c_str()
    );

    if (neigh->isNew()){
        neigh->actionUpdate(
            a_object,
            a_answer,
            a_req_props,
            a_req_params
        );
    }

    {
        string                  html;
        ObjectHttpAnswerSptr    tmp_answer;

        PROP_HTTP_ANSWER(tmp_answer);

        neigh->renderEmbedded(
            tmp_answer,
            a_req_props,
            a_req_params
        );

        html = base64_encode(tmp_answer->getAnswerBody());
        answer_params->add("html", html);
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");

out:
    return;
fail:
    goto out;
*/
}

void Object::actionNeighUnlink(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    string          neigh_id;
    ObjectSptr      object_neigh_id;
    ObjectMapSptr   answer_params;
    ObjectMapSptr   redirect_info;

    PROP_MAP(redirect_info);

    answer_params = a_answer->getCreateAnswerParams();

    if (a_req_params){
        object_neigh_id = a_req_params->get("neigh_id");
    }
    if (object_neigh_id){
        neigh_id = object_neigh_id->toString();
    }
    if (!neigh_id.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param 'neigh_id'"
        );
        goto out;
    }
    a_object->neighUnlink(neigh_id);

    // refresh page
    redirect_info->add("object_id", a_object->getId());
    answer_params->add("redirect",  redirect_info);
    answer_params->add("neigh_id",  object_neigh_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "neigh was unlinked successfully"
    );

out:
    return;
*/
}

void Object::actionRender(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    string          html;
    ObjectMapSptr   answer_params;
    ObjectMapSptr   redirect_info;

    PROP_MAP(redirect_info);

    a_object->render(
        a_answer,
        a_req_props,
        a_req_params
    );

    html = base64_encode(a_answer->getAnswerBody());

    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("html", html);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");
*/
}

void Object::actionRenderEmbedded(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    string          html;
    ObjectMapSptr   answer_params;
    ObjectMapSptr   redirect_info;

    PROP_MAP(redirect_info);

    a_object->renderEmbedded(
        a_answer,
        a_req_props,
        a_req_params
    );

    html = base64_encode(a_answer->getAnswerBody());

    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("html", html);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");
*/
}

void Object::actionUpdateEventsSubscr(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    string              answer;
    ObjectSptr          tmp;
    ObjectMapSptr       params;
    ObjectMapSptr       events;
    ObjectMapIt         events_it;
    string              status;
    string              descr;
    ObjectContextSptr   context;
    ObjectSessionSptr   session;

    status = "failed";
    descr  = "internal error";

    a_object->lock();

    // get context and session
    context = Thread::getCurContext();
    session = context->getSession();

    if (a_req_params){
        tmp = a_req_params->get("events");
    }
    events = dynamic_pointer_cast<ObjectMap>(tmp);
    if (!events){
        status = "failed";
        descr  = "missing argument 'events'";
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
        goto fail;
    }

    for (events_it = events->begin();
        events_it != events->end();
        events_it++)
    {
        ObjectStringSptr    event_name;
        ObjectInt32Sptr     event_subscr;
        ObjectLinkActionSptr link;

        PROP_LINK_EVENT(link);

        // get event name
        event_name = dynamic_pointer_cast<ObjectString>(
            events_it->first
        );
        if (!event_name){
            // TODO inform client about it
            PWARN("invalid event name: '%s'\n",
                events_it->first->serialize().c_str()
            );
            continue;
        }

        // get event subscr
        event_subscr = dynamic_pointer_cast<ObjectInt32>(
            events_it->second
        );
        if (!event_subscr){
            // TODO inform client about it
            PWARN("invalid event subscr: '%s'\n",
                events_it->second->serialize().c_str()
            );
            continue;
        }

        Object::addNeighAndSave(a_object, session, link);
    }

    status = "success";
    descr  = "all ok";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);

out:
    a_object->unlock();
    return;
fail:
    goto out;
*/
}

void Object::actionUpdate(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectStringSptr  prop_name;
    ObjectStringSptr  prop_descr;
    ObjectStringSptr  prop_title;
    ObjectStringSptr  prop_url;
    ObjectVectorSptr  prop_pos;
    ObjectVectorSptr  prop_pos_add;
    ObjectVec3Sptr    prop_view_pos;
    ObjectVec3Sptr    prop_view_pos_add;
    ObjectVec3Sptr    prop_view_up;
    ObjectVec3Sptr    prop_view_up_add;
    ObjectVec3Sptr    prop_view_strafe;
    ObjectVec3Sptr    prop_view_strafe_add;

    // search name
    prop_name = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("name")
    );
    a_object->setName(prop_name);

    // search descr
    prop_descr = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("descr")
    );;
    a_object->setDescr(prop_descr);

    // search title
    prop_title = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("title")
    );
    a_object->setTitle(prop_title);

    // search url
    prop_url = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("url")
    );
    a_object->setUrl(prop_url);

    // ------------ pos, pos_add ------------
    prop_pos = dynamic_pointer_cast<ObjectVector>(
        a_req_params->get("pos")
    );
    if (    prop_pos
        &&  3 == prop_pos->size())
    {
        ObjectVec3Sptr      cur_pos;
        ObjectDoubleSptr    obj_pos_x;
        ObjectDoubleSptr    obj_pos_y;
        ObjectDoubleSptr    obj_pos_z;
        double              pos_x = 0.0f;
        double              pos_y = 0.0f;
        double              pos_z = 0.0f;

        // convert to double
        obj_pos_x = dynamic_pointer_cast<ObjectDouble>(
            prop_pos->get(0)
        );
        obj_pos_y = dynamic_pointer_cast<ObjectDouble>(
            prop_pos->get(1)
        );
        obj_pos_z = dynamic_pointer_cast<ObjectDouble>(
            prop_pos->get(2)
        );

        pos_x = obj_pos_x->getVal();
        pos_y = obj_pos_y->getVal();
        pos_z = obj_pos_z->getVal();

        PROP_VEC3(cur_pos, pos_x, pos_y, pos_z);

        a_object->setPos(cur_pos);
    }
    prop_pos_add = dynamic_pointer_cast<ObjectVector>(
        a_req_params->get("pos_add")
    );
    if (    prop_pos_add
        &&  3 == prop_pos_add->size())
    {
        ObjectVec3Sptr      cur_pos = a_object->getPos();
        ObjectVec3Sptr      new_pos;

        ObjectDoubleSptr    obj_pos_add_x;
        ObjectDoubleSptr    obj_pos_add_y;
        ObjectDoubleSptr    obj_pos_add_z;

        double  pos_add_x   = 0.0f;
        double  pos_add_y   = 0.0f;
        double  pos_add_z   = 0.0f;
        double  cur_pos_x   = cur_pos->getX();
        double  cur_pos_y   = cur_pos->getY();
        double  cur_pos_z   = cur_pos->getZ();

        // convert to double
        obj_pos_add_x = dynamic_pointer_cast<ObjectDouble>(
            prop_pos_add->get(0)
        );
        obj_pos_add_y = dynamic_pointer_cast<ObjectDouble>(
            prop_pos_add->get(1)
        );
        obj_pos_add_z = dynamic_pointer_cast<ObjectDouble>(
            prop_pos_add->get(2)
        );

        pos_add_x = obj_pos_add_x->getVal();
        pos_add_y = obj_pos_add_y->getVal();
        pos_add_z = obj_pos_add_z->getVal();

        PROP_VEC3(
            new_pos,
            cur_pos_x + pos_add_x,
            cur_pos_y + pos_add_y,
            cur_pos_z + pos_add_z
        );

        a_object->setPos(new_pos);
    }

    // ------------ view_up, view_up_add ------------
    prop_view_up = Object::getObjectVec3(
        a_req_params,
        "view_up"
    );
    if (prop_view_up){
        a_object->setViewUp(prop_view_up);
    }
    prop_view_up_add = Object::getObjectVec3(
        a_req_params,
        "view_up_add"
    );
    if (prop_view_up_add){
        ObjectVec3Sptr  cur_pos = a_object->getViewUp();
        ObjectVec3Sptr  new_pos;

        // convert to double
        double pos_x = prop_view_up_add->getX();
        double pos_y = prop_view_up_add->getY();
        double pos_z = prop_view_up_add->getZ();

        double cur_pos_x = cur_pos->getX();
        double cur_pos_y = cur_pos->getY();
        double cur_pos_z = cur_pos->getZ();

        PROP_VEC3(
            new_pos,
            cur_pos_x + pos_x,
            cur_pos_y + pos_y,
            cur_pos_z + pos_z
        );

        a_object->setViewUp(new_pos);
    }

    a_object->save();
*/
}

void Object::actionCreate(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
    ObjectSptr      obj_object_type;
    string          object_type;
    ObjectSptr      new_object;
    ObjectMapSptr   answer_params;
    ObjectMapSptr   redirect;
    string          object_id;

    // TODO
    // check perms here

    PROP_MAP(redirect);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    // search type
    obj_object_type = a_req_params->get("type");
    if (!obj_object_type){
        goto out;
    }

    // get object type
    object_type = obj_object_type->toString();

    // check what type is valid
    // PROP_TYPE(object_object_type);
    // res = object_object_type->isTypeValid(object_type);
    // if (!res){
    //     a_answer->setAnswerStatusCode(200);
    //     a_answer->setAnswerStatusString("OK");
    //     a_answer->setAnswerApiStatus("failed");
    //     a_answer->setAnswerApiDescr(
    //         "invalid object type"
    //     );
    //     goto fail;
    // }

    PFATAL("FIXME\n");

    // create new object
    new_object = ThreadDb::get()->createObject(object_type);
    if (!new_object){
        a_answer->setAnswerStatusCode(500);
        a_answer->setAnswerStatusString("Internal error");
        a_answer->setAnswerBody("");
        goto fail;
    }
    addNeigh(new_object);

    // get object ID
    object_id = new_object->getId();

    // create redirect info
    redirect->add("object_id",        object_id);
    answer_params->add("object_id",   object_id);
    answer_params->add("redirect",    redirect);

    // create answer
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "new object was created successfully"
    );
out:
    return;
fail:
    goto out;
*/
}

void Object::actionAccel(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ObjectContextSptr context = Thread::getCurContext();

    PFATAL("FIXME\n");

/*
    string              status;
    string              descr;
    ObjectVec3Sptr      object_accel;
    ObjectDoubleSptr    object_accel_strafe;
    double              accel_strafe = 0.0f;
    ObjectVec3Sptr      cur_pos;
    ObjectVec3Sptr      cur_view_pos;
    ObjectVec3Sptr      view_vector;
    ObjectVec3Sptr      new_pos;
    ObjectVec3Sptr      new_view_pos;

    cur_pos         = a_object->getPos();
    cur_view_pos    = a_object->getViewPos();
    view_vector     = a_object->getViewVector();

    status          = "failed";
    descr           = "internal error";

    // get accel
    object_accel = Object::getObjectVec3(
        a_req_params,
        "accel"
    );

    // get strafe
    object_accel_strafe = dynamic_pointer_cast<ObjectDouble>(
        a_req_params->get("accel_strafe")
    );

    if (    !object_accel
        &&  !object_accel_strafe)
    {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "at least one argument must be defined:"
            " 'accel' or 'accel_strafe'\n"
        );
        goto fail;
    }

    if (object_accel_strafe){
        accel_strafe = object_accel_strafe->getVal();
    }

    PWARN("MORIK request accel:\n"
        "  accel:           '%s'\n"
        "  accel_strafe:    '%s'\n",
        object_accel
            ? object_accel->toString().c_str()
            : "",
        object_accel_strafe
            ? object_accel_strafe->toString().c_str()
            : ""
    );

    if (object_accel_strafe){
        a_object->strafe(accel_strafe);
    } else {
        a_object->accel(object_accel);
    }

    context->transactionAddForSave(a_object);

    status = "success";
    descr  = "all ok";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);

out:
    return;
fail:
    goto out;
*/
}

void Object::actionRotateViewVector(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ObjectContextSptr   context = Thread::getCurContext();

    PFATAL("FIXME\n");

/*
    string          status;
    string          descr;
    ObjectDoubleSptr  object_yaw;
    ObjectDoubleSptr  object_pitch;

    status = "failed";
    descr  = "internal error";

    object_yaw = dynamic_pointer_cast<ObjectDouble>(
        a_req_params->get("yaw")
    );
    object_pitch = dynamic_pointer_cast<ObjectDouble>(
        a_req_params->get("pitch")
    );

    if (    !object_yaw
        &&  !object_pitch)
    {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "at least one param must be defined:"
            " 'h_speed' or 'v_speed'"
        );
        goto fail;
    }

    PWARN("MORIK request rotateViewVector:\n"
        "  object_yaw: '%s'\n"
        "  object_pitch: '%s'\n",
        object_yaw
            ? object_yaw->toString().c_str()
            : "",
        object_pitch
            ? object_pitch->toString().c_str()
            : ""
    );

    a_object->rotateViewVector(
        object_yaw,
        object_pitch
    );

    context->transactionAddForSave(a_object);

    status = "success";
    descr  = "all ok";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);

out:
    return;
fail:
    goto out;
*/
}

void Object::actionMove(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ObjectContextSptr   context = Thread::getCurContext();

    PFATAL("FIXME\n");

/*
    string            object_id;
    ObjectSptr        object_object_id;
    ObjectSptr        object;
    ObjectVec3Sptr    object_pos;
    ObjectVec3Sptr    object_view_ryp;
    ObjectDoubleSptr  object_view_zoom;
    ObjectVec3Sptr    new_pos;
    ObjectVec3Sptr    new_view_ryp;
    ObjectDoubleSptr  new_view_zoom;

    // TODO XXX
    // 1. check permissions here
    // 2. only 3dprinter can call it,
    // generic users must call 'accel' method instead
    // 20140925 morik@

    if (a_req_params){
        object_object_id = a_req_params->get("object_id");
    }
    if (object_object_id){
        object_id = object_object_id->toString();
    }
    if (!object_id.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param 'object_id'"
        );
        goto fail;
    }

    object = ThreadDb::get()->getObjectById(object_id);
    if (!object){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "object not found"
        );
        goto fail;
    }

    object_pos          = object->getPos();
    object_view_ryp     = object->getViewRYP();
    object_view_zoom    = object->getViewZoom();

    PROP_VEC3(
        new_pos,
        object_pos->getX(),
        object_pos->getY(),
        object_pos->getZ()
    );

    PROP_VEC3(
        new_view_ryp,
        object_view_ryp->getX(),
        object_view_ryp->getY(),
        object_view_ryp->getZ()
    );

    PROP_DOUBLE(
        new_view_zoom,
        object_view_zoom->getVal()
    );

    a_object->setPos(new_pos);
    a_object->setViewRYP(new_view_ryp);
    a_object->setViewZoom(new_view_zoom);

    //a_object->save();
    context->transactionAddForSave(a_object);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");

out:
    return;
fail:
    goto out;
*/
}

// ---------------- static ----------------

string Object::generateId()
{
    char    buf[16] = { 0x00 };
    int32_t byte, i;
    string  id;

    for (i = 0; i < 20; i++){
        byte = int((double(rand())/RAND_MAX)*255);
        snprintf(buf, sizeof(buf), "%2.2x", byte);
        id += buf;
    }

    return id;
}

void Object::createEvent(
    const string    &a_name,
    ObjectMapSptr   a_out_props,
    ObjectMapSptr   a_out_params)
{
    if (a_name.empty()){
        PFATAL("missing argument 'a_name'\n");
    }

//    PROP_MAP(a_out_props);
//    PROP_MAP(a_out_params);

    a_out_props->add("name",    a_name);
    a_out_props->add("params",  a_out_params);
}

// ---------------- modules ----------------

ModuleInfoSptr Object::getModuleInfoByType(
    const string &a_type)
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    ModuleInfoSptr  ret;
    ModulesIt       modules_it;

    if (!g_modules){
        g_modules = ModulesSptr(new Modules);
    }

    modules_it = g_modules->find(a_type);
    if (g_modules->end() != modules_it){
        ret = modules_it->second;
    } else {
        PWARN("Unsupported object type: '%s'\n",
            a_type.c_str()
        );
    }

    return ret;
}

int32_t Object::register_module(
    const ModuleInfo &a_info)
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    int32_t         ret  = 0;
    string          type = a_info.type();
    ModuleInfoSptr  info;

    if (!type.size()){
        PFATAL("missing module type\n");
    }

    if (!g_modules){
        g_modules = ModulesSptr(new Modules);
    }

    PDEBUG(50, "register module_name: '%s'\n",
        type.c_str()
    );

    info = ModuleInfoSptr(new ModuleInfo(a_info));

    (*g_modules.get())[type] = info;

    return ret;
}

void Object::moduleInfoToMap(
    ModuleInfoSptr  a_info,
    ObjectMapSptr   a_out)
{
    string  type      = a_info->type();
    string  type_perl = a_info->type_perl();

    a_out->add("type",      type);
    a_out->add("type_perl", type_perl);
}

void Object::getModulesInfo(
    ObjectMapSptr a_out)
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    ModulesIt   it;

    if (!g_modules){
        g_modules = ModulesSptr(new Modules);
        goto out;
    }

    for (it = g_modules->begin();
        it != g_modules->end();
        it++)
    {
        string          type = it->first;
        ModuleInfoSptr  info = it->second;
        ObjectMapSptr   prop_info;

        PROP_MAP(prop_info);

        Object::moduleInfoToMap(info, prop_info);

        a_out->add(type, prop_info);
    }

out:
    return;
}

ObjectMapSptr Object::getModuleInfo(
    const string    &a_module_type)
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    ObjectMapSptr   ret;
    ModuleInfoSptr  info;
    ModulesIt       it;

    if (!g_modules){
        goto out;
    }

    it = g_modules->find(a_module_type);
    if (g_modules->end() == it){
        goto out;
    }
    info = it->second;

    PROP_MAP(ret);
    Object::moduleInfoToMap(info, ret);

out:
    return ret;
}


int32_t Object::initModules()
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    int32_t     err = -1;
    ModulesIt   it;

    int32_t     init_ok       = 0;
    int32_t     init_failed   = 0;

    PINFO("init modules:\n");

    if (!g_modules){
        g_modules = ModulesSptr(new Modules);
    }

    for (it = g_modules->begin();
        it != g_modules->end();
        it++)
    {
        ModuleInfoSptr info = it->second;
        int32_t        res  = info->init();

        PINFO("  '%s': '%s'\n",
            info->type().c_str(),
            res ? "FAILED" : "OK"
        );

        if (res){
            init_failed++;
        } else {
            init_ok++;
        }
    }
    PINFO("\n");

    PINFO("'%d' ok,"
        " '%d' failed,"
        " '%d' total\n",
        init_ok,
        init_failed,
        (init_ok + init_failed)
    );

    if (init_failed){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 >= err){
        err = -1;
    }
    goto out;
}

void Object::listModules()
{
    lock_guard<recursive_mutex> guard(g_modules_mutex);

    ModulesIt it;

    PINFO("modules list:\n");

    if (!g_modules){
        g_modules = ModulesSptr(new Modules);
    }

    for (it = g_modules->begin();
        it != g_modules->end();
        it++)
    {
        ModuleInfoSptr info = it->second;

        PINFO("module name: '%s'\n",
            it->first.c_str()
        );
    }
    PINFO("\n");
}

ObjectVec3Sptr Object::getObjectVec3(
    const ObjectMapSptr a_params,
    const string        &a_param_name)
{
    ObjectVec3Sptr    object_pos;
    ObjectVectorSptr  object_pos_vector;

    if (!a_param_name.size()){
        goto out;
    }

    object_pos_vector = dynamic_pointer_cast<ObjectVector>(
        a_params->get(a_param_name)
    );

    if (    object_pos_vector
        &&  3 == object_pos_vector->size())
    {
        ObjectDoubleSptr    obj_pos_x;
        ObjectDoubleSptr    obj_pos_y;
        ObjectDoubleSptr    obj_pos_z;
        double              pos_x = 0.0f;
        double              pos_y = 0.0f;
        double              pos_z = 0.0f;

        obj_pos_x = dynamic_pointer_cast<ObjectDouble>(
            object_pos_vector->get(0)
        );
        obj_pos_y = dynamic_pointer_cast<ObjectDouble>(
            object_pos_vector->get(1)
        );
        obj_pos_z = dynamic_pointer_cast<ObjectDouble>(
            object_pos_vector->get(2)
        );

        pos_x = obj_pos_x->getVal();
        pos_y = obj_pos_y->getVal();
        pos_z = obj_pos_z->getVal();

        PROP_VEC3(object_pos, pos_x, pos_y, pos_z);
    }

out:
    return object_pos;
}

ObjectSptr Object::_createObject(
    const string    &a_type,
    const int32_t   &a_do_autorun,
    const string    &a_id)
{
    ObjectSptr          ret;
    ModuleInfoSptr      module_info;
    ObjectStringSptr    prop_type;

    PTIMING(3.0f, "Object::_createObject,"
        " type: '%s',"
        " do_autorun: '%d'"
        "\n",
        a_type.c_str(),
        a_do_autorun
    );

    if (a_type.empty()){
        PFATAL("missing argument: 'a_type'\n");
    }

    PROP_STRING(prop_type, a_type);

    module_info = Object::getModuleInfoByType(
        a_type
    );
    if (module_info){
        // do_init_props, do_init_api will called inside
        // function object_create (in macros)
        ret = module_info->object_create();
    }

    if (!ret){
        PFATAL("cannot create object"
            " with type: '%s'\n",
            a_type.c_str()
        );
    }

    // reset dirty flag
    ret->setDirty(0);

    if (!a_id.empty()){
        ret->setId(a_id);
    }

    // don't add to transaction here,
    // because parseBSON may be still not called
    // don't call autorun here too (same reasons)

    return ret;
}

/*
ObjectLinkSptr Object::addNeigh(
    Object          *a_object,
    Object          *b_object,
    ObjectLinkSptr  a_link)
{
    lock(a_object->m_mutex, b_object->m_mutex);
    lock_guard<recursive_mutex> guard1(
        a_object->m_mutex,
        adopt_lock
    );
    lock_guard<recursive_mutex> guard2(
        b_object->m_mutex,
        adopt_lock
    );

    ObjectLinkSptr link;

    link = Object::addNeigh(
        a_object,
        b_object,
        a_link
    );

    return link;
}

ObjectLinkSptr Object::addNeighAndSave(
    Object          *a_object,
    Object          *b_object,
    ObjectLinkSptr  a_link)
{
    lock(a_object->m_mutex, b_object->m_mutex);
    lock_guard<recursive_mutex> guard1(
        a_object->m_mutex,
        adopt_lock
    );
    lock_guard<recursive_mutex> guard2(
        b_object->m_mutex,
        adopt_lock
    );

    ObjectContextSptr   context = Thread::getCurContext();
    ObjectLinkSptr      link;

    link = Object::addNeigh(
        a_object,
        b_object,
        a_link
    );

    // save all
    context->transactionAddForSave(link);
    context->transactionAddForSave(a_object);
    context->transactionAddForSave(b_object);
    //link->save();
    //a_object->save();
    //b_object->save();

    return link;
}
*/

// ---------------- module ----------------

string Object::_getType()
{
    return "core.object";
}

string Object::_getTypePerl()
{
    return "core::object::object::main";
}

int32_t Object::_init()
{
    int32_t err = 0;
    return err;
}

void Object::_destroy()
{
}

ObjectSptr Object::_object_create()
{
    ObjectSptr object;
    _OBJECT(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = Object::_getType,
    .type_perl      = Object::_getTypePerl,
    .init           = Object::_init,
    .destroy        = Object::_destroy,
    .object_create  = Object::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

