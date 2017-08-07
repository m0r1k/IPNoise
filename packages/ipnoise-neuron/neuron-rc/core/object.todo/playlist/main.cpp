#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/playlist/main.hpp"

ObjectPlaylist::ObjectPlaylist(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectPlaylist::~ObjectPlaylist()
{
};

void ObjectPlaylist::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectPlaylist::getType()
{
    return ObjectPlaylist::_getType();
}

void ObjectPlaylist::do_init_props()
{
    Object::do_init_props();

    ObjectStringSptr    prop_status;
    ObjectStringSptr    prop_play_order;

    PROP_STRING( prop_status,     "stop");
    PROP_STRING( prop_play_order, "listen_time");

    INIT_PROP(this, Status,     prop_status);
    INIT_PROP(this, PlayOrder,  prop_play_order);
}

void ObjectPlaylist::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "setStatus",
        ObjectPlaylist::actionSetStatus
    );
    registerAction(
        "setPlayOrder",
        ObjectPlaylist::actionSetPlayOrder
    );

    // register events
}

int32_t ObjectPlaylist::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectPlaylist::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("status",         getStatus);
    SAVE_PROP("play_order",     getPlayOrder);

    Object::getAllProps(a_props);
}

int32_t ObjectPlaylist::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // status
    if (a_obj.hasField("status")){
        ObjectStringSptr    prop_status;
        string              status;

        status = a_obj.getStringField("status");
        PROP_STRING(prop_status, status);

        INIT_PROP(this, Status, prop_status);
    }

    // play_order
    if (a_obj.hasField("play_order")){
        ObjectStringSptr    prop_play_order;
        string              play_order;

        play_order = a_obj.getStringField("play_order");
        PROP_STRING(prop_play_order, play_order);

        INIT_PROP(this, PlayOrder, prop_play_order);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectPlaylist::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    PFATAL("FIXME\n");
/*
    ObjectPlaylist      *object = NULL;
    ObjectStringSptr    object_status;
    ObjectStringSptr    object_play_order;

    // TODO XXX validate params before set!
    // 20140708 morik@

    object = static_cast<ObjectPlaylist *>(a_object);

    // search status
    object_status = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("status")
    );
    object->setStatus(object_status);

    // search play order
    object_play_order = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("play_order")
    );
    object->setPlayOrder(object_play_order);

    // process upper objects
    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

// ---------------- api -------------------

void ObjectPlaylist::actionSetStatus(
    Object          *a_object,
    ObjectAction    *a_action)
{
    PFATAL("FIXME\n");
/*
    ObjectPlaylist      *object = NULL;
    ObjectStringSptr    prop_status;
    string              status;
    ObjectMapSptr       answer_params;

    object = dynamic_cast<ObjectPlaylist *>(a_object);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    prop_status = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("status")
    );
    if (prop_status){
        status = prop_status->toString();
    }

    if (!status.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param: 'status'"
        );
        goto fail;
    }

    if (    "play"  == status
        ||  "pause" == status
        ||  "stop"  == status)
    {
        object->setStatus(prop_status);
        object->save();
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "unsupported param value: '"+status+"'"
            " for param: 'status'"
        );
        goto fail;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "status was updated"
    );

out:
    return;
fail:
    goto out;
*/
}

void ObjectPlaylist::actionSetPlayOrder(
    Object          *a_object,
    ObjectAction    *a_action)
{
    PFATAL("FIXME\n");
/*
    ObjectPlaylist      *object = NULL;
    ObjectStringSptr    prop_play_order;
    string              play_order;
    ObjectMapSptr       answer_params;

    object = dynamic_cast<ObjectPlaylist *>(a_object);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    prop_play_order = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("play_order")
    );
    if (prop_play_order){
        play_order = prop_play_order->toString();
    }

    if (!play_order.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param: 'play_order'"
        );
        goto fail;
    }

    if ("listen_time" == play_order){
        object->setPlayOrder(prop_play_order);
        object->save();
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "unsupported param value: '"+play_order+"'"
            " for param: 'play_order'"
        );
        goto fail;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "play_order was updated"
    );

out:
    return;
fail:
    goto out;
*/
}

// ---------------- module ----------------

string ObjectPlaylist::_getType()
{
    return "core.object.playlist";
}

string ObjectPlaylist::_getTypePerl()
{
    return "core::object::playlist::main";
}

int32_t ObjectPlaylist::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectPlaylist::_destroy()
{
}

ObjectSptr ObjectPlaylist::_object_create()
{
    ObjectSptr object;
    OBJECT_PLAYLIST(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectPlaylist::_getType,
    .type_perl      = ObjectPlaylist::_getTypePerl,
    .init           = ObjectPlaylist::_init,
    .destroy        = ObjectPlaylist::_destroy,
    .object_create  = ObjectPlaylist::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

