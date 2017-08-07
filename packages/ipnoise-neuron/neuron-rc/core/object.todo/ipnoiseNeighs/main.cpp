#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/ipnoiseNeigh/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/ipnoiseNeighs/main.hpp"

ObjectIPNoiseNeighs::ObjectIPNoiseNeighs(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoiseNeighs::~ObjectIPNoiseNeighs()
{
};

void ObjectIPNoiseNeighs::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectIPNoiseNeighs::getType()
{
    return ObjectIPNoiseNeighs::_getType();
}

void ObjectIPNoiseNeighs::do_init_props()
{
    Object::do_init_props();

    ObjectStringSptr    prop_huid;

    PROP_STRING(prop_huid);

    INIT_PROP(this, Huid, prop_huid);
}

void ObjectIPNoiseNeighs::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoiseNeighs::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseNeighs::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("huid",  getHuid);

    Object::getAllProps(a_props);
}

int32_t ObjectIPNoiseNeighs::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // huid
    if (a_obj.hasField("huid")){
        ObjectStringSptr prop_huid;
        string           huid;

        huid = a_obj.getStringField("huid");
        PROP_STRING(prop_huid, huid);

        INIT_PROP(this, Huid, prop_huid);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

ObjectIPNoiseNeighSptr ObjectIPNoiseNeighs::createIPNoiseNeigh()
{
    ObjectIPNoiseNeighSptr    object_neigh;
    object_neigh = Object::createObject<ObjectIPNoiseNeigh>(
        "core.ipnoise.neigh"
    );
    addNeigh(object_neigh);
    return object_neigh;
}

void ObjectIPNoiseNeighs::actionAddNewNeigh(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectIPNoiseNeighs     *neighs = (ObjectIPNoiseNeighs *)a_object;
    ObjectMapSptr           answer_params;
    ObjectIPNoiseNeighSptr  object_neigh;
    ObjectStringSptr        prop_neigh_id;
    string                  neigh_id;

    // TODO check perms here

    // create neigh
    object_neigh = neighs->createIPNoiseNeigh();
    neigh_id     = object_neigh->getId();
    PROP_STRING(prop_neigh_id, neigh_id);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", prop_neigh_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "neigh was created successfully"
    );
*/
}

void ObjectIPNoiseNeighs::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectIPNoiseNeighs *object  = NULL;
    ObjectStringSptr    prop_huid;

    object = static_cast<ObjectIPNoiseNeighs *>(a_object);

    // search huid
    prop_huid = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("huid")
    );
    object->setHuid(prop_huid);

    // process upper objects
    Object::actionUpdate(
        a_object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

// ---------------- module ----------------

string ObjectIPNoiseNeighs::_getType()
{
    return "core.object.ipnoise.neighs";
}

string ObjectIPNoiseNeighs::_getTypePerl()
{
    return "core::object::ipnoiseNeighs::main";
}

int32_t ObjectIPNoiseNeighs::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseNeighs::_destroy()
{
}

ObjectSptr ObjectIPNoiseNeighs::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE_NEIGHS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectIPNoiseNeighs::_getType,
    .type_perl      = ObjectIPNoiseNeighs::_getTypePerl,
    .init           = ObjectIPNoiseNeighs::_init,
    .destroy        = ObjectIPNoiseNeighs::_destroy,
    .object_create  = ObjectIPNoiseNeighs::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

