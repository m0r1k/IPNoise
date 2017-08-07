#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/dream/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dreams/main.hpp"

ObjectDreams::ObjectDreams(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDreams::~ObjectDreams()
{
};

void ObjectDreams::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDreams::getType()
{
    return ObjectDreams::_getType();
}

void ObjectDreams::do_init_props()
{
    Object::do_init_props();
}

void ObjectDreams::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("add_new_dream", ObjectDreams::actionAddNewDream);
}

int32_t ObjectDreams::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDreams::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDreams::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

ObjectDreamSptr ObjectDreams::createDream()
{
    ObjectDreamSptr    object_dream;
    object_dream = Object::createObject<ObjectDream>(
        "core.object.dream"
    );
    addNeigh(object_dream);
    return object_dream;
}

void ObjectDreams::actionAddNewDream(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
    ObjectDreams        *dreams = (ObjectDreams *)a_object;
    ObjectMapSptr       answer_params;
    ObjectDreamSptr     object_dream;
    ObjectStringSptr    prop_dream_id;
    string              dream_id;

    // TODO check perms here

    // create dream
    object_dream = dreams->createDream();
    dream_id     = object_dream->getId();
    PROP_STRING(prop_dream_id, dream_id);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", prop_dream_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "dream was created successfully"
    );
*/
}

// ---------------- module ----------------

string ObjectDreams::_getType()
{
    return "core.object.dreams";
}

string ObjectDreams::_getTypePerl()
{
    return "core::object::dreams::main";
}

int32_t ObjectDreams::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDreams::_destroy()
{
}

ObjectSptr ObjectDreams::_object_create()
{
    ObjectSptr object;
    OBJECT_DREAMS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDreams::_getType,
    .type_perl      = ObjectDreams::_getTypePerl,
    .init           = ObjectDreams::_init,
    .destroy        = ObjectDreams::_destroy,
    .object_create  = ObjectDreams::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

