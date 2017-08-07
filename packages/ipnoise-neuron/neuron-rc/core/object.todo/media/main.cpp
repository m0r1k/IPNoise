#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/media/main.hpp"

ObjectMedia::ObjectMedia(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectMedia::~ObjectMedia()
{
};

void ObjectMedia::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectMedia::getType()
{
    return ObjectMedia::_getType();
}

void ObjectMedia::do_init_props()
{
    Object::do_init_props();

    ObjectStringSptr    prop_status;

    PROP_STRING(prop_status, "stop");

    INIT_PROP(this, Status, prop_status);
}

void ObjectMedia::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "setStatus",
        ObjectMedia::actionSetStatus
    );
}

int32_t ObjectMedia::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectMedia::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("status", getStatus);

    Object::getAllProps(a_props);
}

int32_t ObjectMedia::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // status
    if (a_obj.hasField("status")){
        string              status;
        ObjectStringSptr    prop_status;

        status = a_obj.getStringField("status");
        PROP_STRING(prop_status, status);

        INIT_PROP(this, Status, prop_status);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectMedia::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectMedia         *object = NULL;
    ObjectStringSptr    object_status;

    object = static_cast<ObjectMedia *>(a_object);

    // search status
    object_status = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("status")
    );
    object->setStatus(object_status);

    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

// ---------------- api -------------------

void ObjectMedia::actionSetStatus(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectMedia         *object = NULL;
    ObjectStringSptr    prop_status;
    string              status;
    ObjectMapSptr       answer_params;

    object = dynamic_cast<ObjectMedia *>(a_object);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    prop_status = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("status")
    );
    if (prop_status){
        status = prop_status->toString();
    }
    if (    "play"  == status
        ||  "pause" == status
        ||  "stop"  == status)
    {
        object->setStatus(prop_status);
        object->save();
    } else if (status.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"unsupported value"
                    " for param 'status'\""
            "}"
        );
        goto out;
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"missing or empty param 'status'\""
            "}"
        );
        goto out;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "status was updated"
    );

out:
    return;
*/
}

// ---------------- module ----------------

string ObjectMedia::_getType()
{
    return "core.object.media";
}

string ObjectMedia::_getTypePerl()
{
    return "core::object::media::main";
}

int32_t ObjectMedia::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectMedia::_destroy()
{
}

ObjectSptr ObjectMedia::_object_create()
{
    ObjectSptr object;
    OBJECT_MEDIA(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectMedia::_getType,
    .type_perl      = ObjectMedia::_getTypePerl,
    .init           = ObjectMedia::_init,
    .destroy        = ObjectMedia::_destroy,
    .object_create  = ObjectMedia::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

