#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/order/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/orders/main.hpp"

ObjectOrders::ObjectOrders(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectOrders::~ObjectOrders()
{
};

void ObjectOrders::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectOrders::getType()
{
    return ObjectOrders::_getType();
}

void ObjectOrders::do_init_props()
{
    Object::do_init_props();
}

void ObjectOrders::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectOrders::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectOrders::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectOrders::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- order ----------------

ObjectOrderSptr ObjectOrders::createOrder()
{
    ObjectOrderSptr    object_order;
    object_order = Object::createObject<ObjectOrder>(
        "core.object.order"
    );
    addNeigh(object_order);
    return object_order;
}

// ---------------- module ----------------

string ObjectOrders::_getType()
{
    return "core.object.orders";
}

string ObjectOrders::_getTypePerl()
{
    return "core::object::orders::main";
}

int32_t ObjectOrders::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectOrders::_destroy()
{
}

ObjectSptr ObjectOrders::_object_create()
{
    ObjectSptr object;
    OBJECT_ORDERS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectOrders::_getType,
    .type_perl      = ObjectOrders::_getTypePerl,
    .init           = ObjectOrders::_init,
    .destroy        = ObjectOrders::_destroy,
    .object_create  = ObjectOrders::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

