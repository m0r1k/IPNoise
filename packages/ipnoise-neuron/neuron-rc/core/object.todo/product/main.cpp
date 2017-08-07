#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/basket/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/product/main.hpp"

ObjectProduct::ObjectProduct(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectProduct::~ObjectProduct()
{
};

void ObjectProduct::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectProduct::getType()
{
    return ObjectProduct::_getType();
}

void ObjectProduct::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.product");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectProduct::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "addToBasket",
        ObjectProduct::actionAddToBasket
    );
}

int32_t ObjectProduct::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectProduct::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectProduct::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectProduct::actionAddToBasket(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectUserSptr      cur_user;
    ObjectBasketSptr    basket;

    cur_user = Context::getCurUser();

    // TODO check perms here

    // add to basket
    basket = cur_user->getCreateBasket();
    a_object->addNeigh(basket);

    // prepare event
    {
        ObjectMapSptr event(new ObjectMap);
        event->add("type",      "rerender");
        event->add("object_id", basket->getId());
        // send event to all sessions
        cur_user->addEvent(event);
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was added to basket successfully"
    );
*/
}

// ---------------- module ----------------

string ObjectProduct::_getType()
{
    return "core.object.product";
}

string ObjectProduct::_getTypePerl()
{
    return "core::object::product::main";
}

int32_t ObjectProduct::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectProduct::_destroy()
{
}

ObjectSptr ObjectProduct::_object_create()
{
    ObjectSptr object;
    OBJECT_PRODUCT(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectProduct::_getType,
    .type_perl      = ObjectProduct::_getTypePerl,
    .init           = ObjectProduct::_init,
    .destroy        = ObjectProduct::_destroy,
    .object_create  = ObjectProduct::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

