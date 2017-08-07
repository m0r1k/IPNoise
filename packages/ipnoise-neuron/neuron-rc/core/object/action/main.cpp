#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/links/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>

#include "db.hpp"

#include "main.hpp"

ObjectAction::ObjectAction(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectAction::~ObjectAction()
{
}

string ObjectAction::getType()
{
    return ObjectAction::s_getType();
}

int32_t ObjectAction::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectAction::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_src           = PROP<ObjectString>();
    m_action_params = PROP<ObjectMap>();

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

ObjectSptr ObjectAction::copy()
{
    ObjectActionSptr    ret;

    ret = OBJECT<ObjectAction>();

    ret->setSrc(m_src->c_str());
    ret->setActionParams(dynamic_pointer_cast<ObjectMap>(
        m_action_params->copy()
    ));

    return ret;
}

void ObjectAction::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        a_out->add("action_src",       m_src);
        a_out->add("action_params",    m_action_params);
        Object::getAllProps(a_out);
    }
}

void ObjectAction::setActionParams(
    ObjectMapSptr a_params)
{
    m_action_params = a_params;
}

ObjectMapSptr ObjectAction::getParams()
{
    return m_action_params;
}

ObjectSptr  ObjectAction::getParam(
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

ObjectSptr  ObjectAction::getParam(
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

    param_name = PROP<ObjectString>(
        a_param_name,
        param_name_len
    );

    ret = getParam(param_name);

    return ret;
}

void  ObjectAction::setActionParam(
    ObjectStringSptr    a_param_name,
    ObjectSptr          a_param_val)
{
    m_action_params->add(a_param_name, a_param_val);

    setDirty(1);
}

void  ObjectAction::setActionParam(
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

    param_name = PROP<ObjectString>(
        a_param_name,
        param_name_len
    );

    setActionParam(param_name, a_param_val);
}

void  ObjectAction::setActionParam(
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

    param_name = PROP<ObjectString>(
        a_param_name,
        param_name_len
    );

    param_val = PROP<ObjectInt32>(a_param_val);

    setActionParam(param_name, param_val);
}

void  ObjectAction::setActionParam(
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

    param_name = PROP<ObjectString>(
        a_param_name,
        param_name_len
    );

    param_val = PROP<ObjectString>(
        a_param_val,
        a_param_val_size
    );

    setActionParam(param_name, param_val);
}

void ObjectAction::run(
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

    PWARN("'%s' run\n", object_path.c_str());

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

void ObjectAction::setSrc(
    const char *a_src)
{
    m_src = PROP<ObjectString>(a_src);

    setDirty(1);
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectAction::s_getType()
{
    return "core.object.action";
}

int32_t ObjectAction::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectAction::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectAction::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectAction(a_create_flags));

    return object;
}

void ObjectAction::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_action = {
    .getType        = ObjectAction::s_getType,
    .init           = ObjectAction::s_init,
    .shutdown       = ObjectAction::s_shutdown,
    .objectCreate   = ObjectAction::s_objectCreate,
    .getTests       = ObjectAction::s_getTests
};

