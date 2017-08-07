#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "main.hpp"
#include "module.hpp"
#include "db.hpp"
#include <core/object/action/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/links/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/vector/main.hpp>
#include <core/object/uint32/main.hpp>
#include <core/object/unique/main.hpp>

int32_t     g_debug_level   = 0;
EngineInfo  *g_engine_info  = NULL;

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
    const CreateFlags &a_create_flags)
{
    char    buffer[65535] = { 0x00 };
    string  object_id     = Object::s_generateId();

    m_create_flags  = a_create_flags;
    m_is_dirty      = 0;

    // calculate object path
    snprintf(buffer, sizeof(buffer),
        "%s%s/",
        IPNOISE_DB_OBJECTS,
        object_id.c_str()
    );

    m_object_path = buffer;
}

Object::~Object()
{
}

string Object::getType()
{
    return Object::s_getType();
}

int32_t Object::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    return 0;
}

int32_t Object::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    m_links         = OBJECT<ObjectLinks>(CREATE_PROP);
    m_actions_out   = OBJECT<ObjectUnique>(CREATE_PROP);
    m_action_params = OBJECT<ObjectMap>(CREATE_PROP);

    return err;
}

ObjectSptr Object::copy()
{
    ObjectSptr ret;

    ret = OBJECT<Object>();
    ret->setObjectPath(m_object_path.c_str());
    ret->setObjectName(m_name.c_str());
    ret->setObjectLinks(
        dynamic_pointer_cast<ObjectLinks>(
            m_links->copy()
        )
    );
    ret->setActionParams(
        dynamic_pointer_cast<ObjectMap>(
            m_action_params->copy()
        )
    );

    return ret;
}

void Object::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        // don't forget update 'copy' method
        a_out->add("object_name",           m_name);
        a_out->add("object_links",          m_links);
        a_out->add("object_action_params",  m_action_params);
    }
}

void Object::getAllPropsDirty(
    ObjectMapSptr   a_out)
{
    lock(m_rmutex, a_out->m_rmutex);
    lock_guard<recursive_mutex> guard1(m_rmutex,        adopt_lock);
    lock_guard<recursive_mutex> guard2(a_out->m_rmutex, adopt_lock);

    ObjectMapSptr   props;
    ObjectMapIt     props_it;
    string          object_path = getObjectPath();

    props = OBJECT<ObjectMap>();
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
        } else {
            PWARN("object: '%s' skip prop: '%s' while save,"
                " because prop not dirty, prop val below:\n"
                "%s\n",
                object_path.c_str(),
                cur_prop_name->toString()->c_str(),
                cur_prop_val->toString()->c_str()
            );
        }
    }
}

void Object::setDirty(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

/*
    if (CREATE_OBJECT & m_create_flags){
        // object
        ObjectMapSptr props;
        ObjectMapIt   it;

        props = PROP<ObjectMap>();
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
*/
    m_is_dirty = a_val;
}

bool Object::isDirty()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    bool ret = false;

/*
    if (CREATE_OBJECT & m_create_flags){
        // object
        ObjectMapSptr props;
        props = PROP<ObjectMap>();
        getAllPropsDirty(props);
        if (!props->empty()){
            ret = true;
        }
    } else {
        // prop
        ret = m_is_dirty;
    }
*/
        ret = m_is_dirty;

    return ret;
}

void Object::setObjectPath(
    const char *a_path)
{
    string path;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    path = a_path;
    if ('/' != path.at(path.size() - 1)){
        // add '/' to end if not exist
        path += "/";
    }

    m_object_path = path;
}

string Object::getObjectPath()
{
    return m_object_path;
}

int32_t Object::loadAsProp(
    const char  *a_dir)
{
    return 0;
}

int32_t Object::load(
    const char  *a_dir)
{
    int32_t         err     = -1;
    string          dir     = m_object_path.c_str();
    ObjectMapSptr   props;
    ObjectMapIt     props_it;
    string          props_path;

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot load object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    // add '/' to end if not exist
    if ('/' != dir.at(dir.size() - 1)){
        dir += "/";
    }

    if (m_object_path.empty()){
        // if path empty, set as default
        m_object_path = dir;
    }

    props_path = dir;
    props_path += OBJECT_DIR_PROPS;

    PDEBUG(50, "object type: '%s',"
        " load from dir: '%s'\n",
        getType().c_str(),
        a_dir
    );

    // load as prop value
    loadAsProp(dir.c_str());

    // get all props
    props = OBJECT<ObjectMap>();
    getAllProps(props);
    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectStringSptr    prop_name;
        ObjectSptr          prop_val;
        string              prop_path;

        prop_name = dynamic_pointer_cast<ObjectString>(
            props_it->first
        );
        prop_val  = props_it->second;
        prop_path = props_path + prop_name->c_str();

        PDEBUG(15, "load prop: '%s' from: '%s'\n",
            prop_name->c_str(),
            prop_path.c_str()
        );

        prop_val->loadAsProp(prop_path.c_str());
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    goto out;
}

int32_t Object::saveAsProp(
    const char *a_path)
{
    // object doesn't support save as prop
    return 0;
}

int32_t Object::save(
    const char  *a_dir)
{
    int32_t         res, err    = -1;
    string          dir         = m_object_path;
    ObjectMapSptr   props;
    ObjectMapIt     props_it;
    string          props_path;

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot save object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    // add '/' to end if not exist
    if ('/' != dir.at(dir.size() - 1)){
        dir += "/";
    }

    if (m_object_path.empty()){
        // if path empty, set as default
        m_object_path = dir;
    }

    PDEBUG(15, "attempt to save object to dir: '%s'\n",
        dir.c_str()
    );

    props_path = dir + OBJECT_DIR_PROPS;

    props = OBJECT<ObjectMap>();
    getAllPropsDirty(props);

    // create dirs
    {
        ObjectVectorSptr    dirs;

        dirs = OBJECT<ObjectVector>();
        dirs->add(dir);
        if (!props->empty()){
            dirs->add(props_path);
        }
        res = ObjectDir::s_mkdir(dirs, 1);
        if (res){
            PFATAL("cannot create one or more dir(s),"
                " dir(s) dump below:\n"
                "%s\n",
                dirs->toString()->c_str()
            );
        }
    }

    // save type
    {
        string  full_path;
        FILE    *file = NULL;
        string  mode  = "w";
        string  val   = getType();

        full_path = dir;
        full_path += "/type";

        file = fopen(full_path.c_str(), mode.c_str());
        if (!file){
            PFATAL("cannot open: '%s' for mode: '%s'\n",
                full_path.c_str(),
                mode.c_str()
            )
        }

        fwrite(val.c_str(), 1, val.size(), file);

        fclose(file);
    }

    res = saveAsProp(dir.c_str());
    if (res){
        PERROR("cannot save prop part of object\n");
        goto fail;
    }

    // save props
    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectStringSptr    prop_name;
        ObjectSptr          prop_val;
        string              prop_path;

        prop_name = dynamic_pointer_cast<ObjectString>(
            props_it->first
        );
        prop_val  = props_it->second;
        prop_path = props_path + prop_name->c_str();

        PDEBUG(15, "save prop: '%s' to: '%s'\n",
            prop_name->c_str(),
            prop_path.c_str()
        );

        prop_val->saveAsProp(prop_path.c_str());
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    goto out;
}

int32_t Object::remove(
    const int32_t &a_force)
{
    int32_t res, err    = -1;
    string  object_path = getObjectPath();

/*
    TODO XXX FIXME

    if (!a_force){
        // just mark as deleted
        m_removed = ObjectTime::now();
        setDirty(1);
        err = save();
        goto out;
    }
*/

    // force delete
    res = ObjectDir::s_rm(
        object_path.c_str(),
        1,  // recursive
        1   // remove_top_dir
    );
    if (res){
        err = res;
        PERROR("cannot remove: '%s', res: '%d'\n",
            object_path.c_str(),
            res
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void Object::run(
    Object *a_thread)
{
    ObjectMapSptr   links;
    ObjectMapIt     links_it;
    string          object_path;
    string          thread_id;
    string          thread_path;

    if (!a_thread){
        PFATAL("missing argument: 'a_thread'\n");
    }

    thread_path = a_thread->getObjectPath();
    thread_id   = a_thread->getId();
    object_path = getObjectPath();
    links       = getObjectLinks()->getVals();

    while (links->size()){
        ObjectStringSptr    object_link_id;
        ObjectStringSptr    object_link_path;
        string              link_id;
        string              link_path;

        links_it = links->begin();

        // get link id
        object_link_id = dynamic_pointer_cast<ObjectString>(
            links_it->first
        );
        if (!object_link_id){
            PFATAL("link ID is not ObjectString\n");
        }
        link_id = object_link_id->toStringStd();

        // get link path
        object_link_path = dynamic_pointer_cast<ObjectString>(
            links_it->second
        );
        if (!object_link_path){
            PFATAL("link path is not ObjectString\n");
        }
        link_path = object_link_path->toStringStd();

        // remove from links
        links->erase(links_it);

        if (link_id == thread_id){
            // if destination is thread, then
            // use already loaded object
            //
            // processAction can add new links to action
            // and they will be processed immediately
            a_thread->processAction(this);
        } else {
            // load new object
            ObjectSptr  object;
            object = g_engine_info->objectLoad(
                link_path.c_str()
            );
            if (!object){
                PERROR("cannot load object from path: '%s'\n",
                    link_path.c_str()
                );
                continue;
            }
            // processAction can add new links to action
            // and they will be processed immediately
            object->processAction(this);
        }
    }

    // remove action
    remove();
}

ObjectStringSptr Object::toString()
{
    ObjectStringSptr ret;

    ret = OBJECT<ObjectString>(getId());

    return ret;
}

string Object::toStringStd()
{
    ObjectStringSptr    res;

    res = toString();

    return res->toStringStd();
}

ObjectStringSptr Object::serializeAsProp(
    const char *a_delim)
{
    // object doesn't support serialize as prop
    ObjectStringSptr ret;
    return ret;
}

ObjectStringSptr Object::serialize(
    const char  *a_delim)
{
    ObjectMapSptr       props;
    ObjectStringSptr    prop_val;
    ObjectStringSptr    ret;

    ret = OBJECT<ObjectString>();

    ret->add("{");
    ret->add("\"type\"");
    ret->add(a_delim);
    ret->add("\"" + getType() + "\"");

    // add prop val
    prop_val = serializeAsProp(a_delim);
    if (prop_val){
        ret->add(", \"val\"");
        ret->add(a_delim);
        ret->add(prop_val);
    }

    // add props
    props = OBJECT<ObjectMap>();
    getAllProps(props);
    if (!props->empty()){
        prop_val = props->serializeAsProp(a_delim);
        if (prop_val){
            ret->add(", \"props\"");
            ret->add(a_delim);
            ret->add(prop_val);
        }
    }

    ret->add("}");

    return ret;
}

void Object::serialize(
    string      &a_out,
    const char  *a_delim)
{
    ObjectStringSptr res;

    res = serialize(a_delim);

    a_out = res->toStringStd();
}

void Object::processAction(
    Object  *a_action)
{
    ObjectStringSptr    action_name;
    string              object_path;
    ActionsHandlersIt   actions_handlers_it;

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = getObjectPath();
    action_name = a_action->getActionName();

    PDEBUG(10, "object: '%s', process action: '%s'\n",
        object_path.c_str(),
        action_name->c_str()
    );

    actions_handlers_it = m_actions_in.find(
        action_name->toStringStd()
    );
    if (m_actions_in.end() != actions_handlers_it){
        actions_handlers_it->second(this, a_action);
    } else {
        PERROR("object: '%s',"
            " unsupported action: '%s',"
            " action dump below:\n"
            "%s\n",
            object_path.c_str(),
            action_name->c_str(),
            a_action->toString()->c_str()
        );
    }
}

string Object::getId(
    const char *a_path)
{
    string      ret;
    const char  *path = m_object_path.c_str();

    if (a_path){
        path = a_path;
    }

    ret = getIdFromPath(path);

    return ret;
}

/*
void Object::getInTranslateTable(
    TranslateTable  &a_out)
{
    a_out["core.action.thread.started"] = "core.action.http.server.listen";
}

void Object::getOutTranslateTable(
    TranslateTable  &a_out)
{
}
*/

CreateFlags Object::getObjectFlags()
{
    return m_create_flags;
}

string Object::getObjectName()
{
    return m_name;
}

void Object::setObjectName(
    const char *a_name)
{
    if (!a_name){
        PFATAL("missing argument 'a_name'\n");
    }
    m_name = a_name;
}

void Object::setObjectLinks(
    ObjectLinksSptr a_val)
{
    m_links = a_val;
}

ObjectLinksSptr Object::getObjectLinks()
{
    return m_links;
}

/*
ObjectSptr Object::getPropVal()
{
    ObjectSptr ret;
    return ret;
}
*/

void Object::emit(
    ObjectSptr a_action)
{
    int32_t             res;
    ObjectMapSptr       links;
    ObjectMapIt         links_it;
    string              object_id = getId();
    string              action_id;
    ObjectStringSptr    action_name;
    string              action_path;

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    action_id   = a_action->getId();
    action_name = a_action->getActionName();
    action_path = a_action->getObjectPath();

    if (    !action_name
        ||  action_name->empty())
    {
        PFATAL("object: '%s',"
            " attempt to emit action with empty name\n",
            object_id.c_str()
        );
    }

    if (!m_actions_out->has(action_name)){
        PFATAL("object: '%s',"
            " attempt to emit unregistered action: '%s'\n",
            object_id.c_str(),
            action_name->c_str()
        );
    }

    // setup source
    a_action->setActionSrc(object_id.c_str());

    PDEBUG(100, "emit action ID: '%s':\n"
        "%s\n",
        action_id.c_str(),
        a_action->toString()->c_str()
    );

    // search links
    links = m_links->getVals();
    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectStringSptr    link_id;
        ObjectStringSptr    link_path;
        ObjectStringSptr    action_link_path;

        link_id = dynamic_pointer_cast<ObjectString>(
            links_it->first
        );
        link_path = dynamic_pointer_cast<ObjectString>(
            links_it->second
        );

        action_link_path = OBJECT<ObjectString>();
        action_link_path->add(link_path);
        action_link_path->add(OBJECT_DIR_ACTIONS);
        action_link_path->add(action_id);

        PDEBUG(90, "object: '%s' send action:\n"
            " link_id:          '%s'\n"
            " link_path:        '%s'\n"
            " action_path:      '%s'\n"
            " action_link_path: '%s'\n",
            object_id.c_str(),
            link_id->c_str(),
            link_path->c_str(),
            action_path.c_str(),
            action_link_path->c_str()
        );

        a_action->linkAdd(link_path->c_str());
    }

    // save action
    a_action->save();

    // schedule action for start
    {
        string action_start_path;
        action_start_path   = IPNOISE_DB_ACTIONS;
        action_start_path   += action_id;
        res = symlink(
            action_path.c_str(),
            action_start_path.c_str()
        );
        if (res){
            PERROR("symlink '%s' to '%s' failed,"
                " res: '%d'\n",
                action_start_path.c_str(),
                action_path.c_str(),
                res
            );
        }
    }
}

int32_t Object::linkAdd(
    const char *a_path)
{
    int32_t err = -1;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    PDEBUG(50, "object '%s', add link: '%s'\n",
        getObjectPath().c_str(),
        a_path
    );

    m_links->add(a_path);

    return err;
}

int32_t Object::linkErase(
    const char *a_path)
{
    int32_t err = -1;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    PDEBUG(50, "object '%s', erase link: '%s'\n",
        getObjectPath().c_str(),
        a_path
    );

    m_links->erase(a_path);

    return err;
}

// ---------------- object as action ----------------

void Object::registerActionIn(
    const char      *a_name,
    ActionHandler   handler)
{
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    m_actions_in[a_name] = handler;
}

void Object::registerActionOut(
    const char  *a_name)
{
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    m_actions_out->add(a_name);
}

void Object::setActionParams(
    ObjectMapSptr a_params)
{
    m_action_params = a_params;
}

ObjectMapSptr Object::getActionParams()
{
    return m_action_params;
}

ObjectSptr  Object::getActionParam(
    ObjectStringSptr    a_param_name)
{
    ObjectMapIt it;
    ObjectSptr  ret;

    it = m_action_params->find(a_param_name);
    if (m_action_params->end() != it){
        ret = it->second;
    }

    return ret;
}

ObjectSptr  Object::getActionParam(
    const char *a_param_name)
{
    ObjectSptr          ret;
    ObjectStringSptr    param_name;
    int32_t             param_name_len = 0;

    if (!a_param_name){
        PFATAL("missing argument: 'a_param_name'\n");
    }

    param_name_len = strlen(a_param_name);
    if (0 >= param_name_len){
        PFATAL("empty argument: 'a_param_name'\n");
    }

    param_name = OBJECT<ObjectString>(
        a_param_name,
        param_name_len
    );

    ret = getActionParam(param_name);

    return ret;
}

void  Object::setActionParam(
    ObjectStringSptr    a_param_name,
    ObjectSptr          a_param_val)
{
    m_action_params->add(a_param_name, a_param_val);

    setDirty(1);
}

void  Object::setActionParam(
    const char  *a_param_name,
    ObjectSptr  a_param_val)
{
    ObjectSptr          ret;
    ObjectStringSptr    param_name;
    int32_t             param_name_len = 0;

    if (!a_param_name){
        PFATAL("missing argument: 'a_param_name'\n");
    }

    param_name_len = strlen(a_param_name);
    if (0 >= param_name_len){
        PFATAL("empty argument: 'a_param_name'\n");
    }

    param_name = OBJECT<ObjectString>(
        a_param_name,
        param_name_len
    );

    setActionParam(param_name, a_param_val);
}

void  Object::setActionParam(
    const char      *a_param_name,
    const int32_t   &a_param_val)
{
    ObjectSptr          ret;
    ObjectStringSptr    param_name;
    ObjectInt32Sptr     param_val;
    int32_t             param_name_len = 0;

    if (!a_param_name){
        PFATAL("missing argument: 'a_param_name'\n");
    }

    param_name_len = strlen(a_param_name);
    if (0 >= param_name_len){
        PFATAL("empty argument: 'a_param_name'\n");
    }

    param_name = OBJECT<ObjectString>(
        a_param_name,
        param_name_len
    );

    param_val = OBJECT<ObjectInt32>(a_param_val);

    setActionParam(param_name, param_val);
}

void  Object::setActionParam(
    const char  *a_param_name,
    const char  *a_param_val)
{
    ObjectSptr          ret;
    ObjectStringSptr    param_name;
    ObjectStringSptr    param_val;
    int32_t             param_name_len = 0;

    if (!a_param_name){
        PFATAL("missing argument: 'a_param_name'\n");
    }

    param_name_len = strlen(a_param_name);
    if (0 >= param_name_len){
        PFATAL("empty argument: 'a_param_name'\n");
    }

    param_name = OBJECT<ObjectString>(
        a_param_name,
        param_name_len
    );

    param_val = OBJECT<ObjectString>(a_param_val);

    setActionParam(param_name, param_val);
}

void  Object::setActionParam(
    const char      *a_param_name,
    const char      *a_param_val,
    const uint64_t  a_param_val_size)
{
    ObjectSptr          ret;
    ObjectStringSptr    param_name;
    ObjectStringSptr    param_val;
    int32_t             param_name_len = 0;

    if (!a_param_name){
        PFATAL("missing argument: 'a_param_name'\n");
    }

    param_name_len = strlen(a_param_name);
    if (0 >= param_name_len){
        PFATAL("empty argument: 'a_param_name'\n");
    }

    if (!a_param_val){
        PFATAL("missing argument: 'a_param_val'\n");
    }

    param_name = OBJECT<ObjectString>(
        a_param_name,
        param_name_len
    );

    param_val = OBJECT<ObjectString>(
        a_param_val,
        a_param_val_size
    );

    setActionParam(param_name, param_val);
}

void Object::setActionName(
    const char *a_val)
{
    setActionParam("name", a_val);
}

ObjectStringSptr Object::getActionName()
{
    ObjectStringSptr ret;
    ret = dynamic_pointer_cast<ObjectString>(
        getActionParam("name")
    );
    return ret;
}

void Object::setActionSrc(
    const char *a_val)
{
    setActionParam("src", a_val);
}

ObjectStringSptr Object::getActionSrc()
{
    ObjectStringSptr ret;
    ret = dynamic_pointer_cast<ObjectString>(
        getActionParam("src")
    );
    return ret;
}

void Object::setActionDst(
    const char *a_val)
{
    setActionParam("dst", a_val);
}

ObjectStringSptr Object::getActionDst()
{
    ObjectStringSptr ret;
    ret = dynamic_pointer_cast<ObjectString>(
        getActionParam("dst")
    );
    return ret;
}

// ---------------- static ----------------

string Object::s_generateId()
{
    char        buff[4] = { 0x00 };
    string      id;
    int32_t     i;

    for (i = 0; i < OBJECT_ID_LEN_BYTES; i++){
        int32_t byte = int32_t(
            (double(rand()) / RAND_MAX) * 255
        );
        snprintf(buff, sizeof(buff), "%2.2x", byte);
        id += buff;
    }

    return id;
}

// ---------------- module ----------------

string Object::s_getType()
{
    return "core.object.object";
}

int32_t Object::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t Object::s_shutdown()
{
    return 0;
}

ObjectSptr Object::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new Object(a_create_flags));

    return object;
}

int32_t Object::s_testLoad(
    const char *a_tmp_path)
{
    int32_t err = 0;
    return err;
}

int32_t Object::s_testSave(
    const char *a_tmp_path)
{
    int32_t err = 0;
    return err;
}

void Object::s_getTests(
    Tests &a_out)
{
    TestInfo test;

    test = TestInfo();
    test.name           = "test load";
    test.run            = NULL; // Object::s_testLoad;
    test.success_code   = 0;
    a_out.push_back(test);

    test = TestInfo();
    test.name           = "test save";
    test.run            = NULL; // Object::s_testSave;
    test.success_code   = 0;
    a_out.push_back(test);
}

ModuleInfo core_object_object = {
    .getType        = Object::s_getType,
    .init           = Object::s_init,
    .shutdown       = Object::s_shutdown,
    .objectCreate   = Object::s_objectCreate,
    .getTests       = Object::s_getTests
};

