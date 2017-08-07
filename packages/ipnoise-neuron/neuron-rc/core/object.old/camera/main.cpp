#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/vec3/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/camera/main.hpp"

ObjectCamera::ObjectCamera(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectCamera::~ObjectCamera()
{
};

void ObjectCamera::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectCamera::getType()
{
    return ObjectCamera::_getType();
}

void ObjectCamera::do_init_props()
{
    Object::do_init_props();
}

void ObjectCamera::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("watchStart", ObjectCamera::actionWatchStart);
}

int32_t ObjectCamera::do_autorun()
{
    int32_t err = 0;

    return err;
}

void ObjectCamera::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectCamera::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- api ----------------

void ObjectCamera::actionWatchStart(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectCamera      *camera = (ObjectCamera *)a_object;
    ObjectVec3Sptr    cur_pos = camera->getPos();
    string            cur_id  = camera->getId();
    ObjectVectorSptr  objects;
    ObjectVectorIt    objects_it;
    ObjectMapSptr     answer_params;
    ObjectVec3Sptr    pos_min;
    ObjectVec3Sptr    pos_max;
    ObjectVectorSptr  ids;

    // TODO check perms here

    // init
    PROP_VECTOR(objects);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    PROP_VEC3(pos_min,
        cur_pos->getX() - 100*ELECTRON_RADIUS,
        cur_pos->getY() - 100*ELECTRON_RADIUS,
        cur_pos->getZ() - 100*ELECTRON_RADIUS
    );

    PROP_VEC3(pos_max,
        cur_pos->getX() + 100*ELECTRON_RADIUS,
        cur_pos->getY() + 100*ELECTRON_RADIUS,
        cur_pos->getZ() + 100*ELECTRON_RADIUS
    );

    ThreadDb::get()->getObjectsByPos<ObjectMedia>(
        pos_min,
        pos_max,
        objects
    );

    ThreadDb::get()->getObjectsByPos<ObjectCamera>(
        pos_min,
        pos_max,
        objects
    );

    PWARN("after search objects near of camera with ID: '%s',"
        " found size: '%d'\n",
        camera->getId().c_str(),
        objects->size()
    );

    PROP_VECTOR(ids);

    for (objects_it = objects->begin();
        objects_it != objects->end();
        objects_it++)
    {
        ObjectSptr  object = *objects_it;
//        string          object_id;
//        object_id   = object->getId()->toString();
//        if (object_id == cur_id){
//            // skip ourself
//            continue;
//        }

        ObjectStringSptr    prop_id;
        PROP_STRING(prop_id, object->getId());

        ids->add(prop_id);
    }

    answer_params->add("draw_ids", ids);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "all ok"
    );
*/
}

// ---------------- module ----------------

string ObjectCamera::_getType()
{
    return "core.object.camera";
}

string ObjectCamera::_getTypePerl()
{
    return "core::object::camera::main";
}

int32_t ObjectCamera::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectCamera::_destroy()
{
}

ObjectSptr ObjectCamera::_object_create()
{
    ObjectSptr object;
    _OBJECT_CAMERA(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectCamera::_getType,
    .type_perl      = ObjectCamera::_getTypePerl,
    .init           = ObjectCamera::_init,
    .destroy        = ObjectCamera::_destroy,
    .object_create  = ObjectCamera::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

