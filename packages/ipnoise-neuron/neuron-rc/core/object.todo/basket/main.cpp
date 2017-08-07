#include "core/object/object/__BEGIN__.hpp"
#include "core/object/objectLink/main.hpp"
#include "core/object/objectLinkInfo/main.hpp"
#include "core/object/objectLinkAction/main.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/order/main.hpp"
#include "core/object/orders/main.hpp"
#include "core/object/product/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/basket/main.hpp"

ObjectBasket::ObjectBasket(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectBasket::~ObjectBasket()
{
};

void ObjectBasket::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectBasket::getType()
{
    return ObjectBasket::_getType();
}

void ObjectBasket::do_init_props()
{
    Object::do_init_props();
}

void ObjectBasket::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "createOrder",
        ObjectBasket::actionCreateOrder
    );
}

int32_t ObjectBasket::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectBasket::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectBasket::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectBasket::actionCreateOrder(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectBasket        *basket = (ObjectBasket *)a_object;
    ObjectMapSptr       answer_params;
    ObjectOrdersSptr    object_orders;
    ObjectOrderSptr     object_order;
    ObjectUserSptr      cur_user;
    string              order_id;

    ObjectVectorSptr    products;
    ObjectVectorIt      products_it;
    ObjectUniqueSptr    object_types;
    ObjectStringSptr    empty;

    // get cur user
    cur_user = Context::getCurUser();

    // TODO check perms here

    // create order
    object_orders   = cur_user->getCreateOrdersObject();
    object_order    = object_orders->createOrder();
    order_id        = object_order->getId();

    PROP_STRING(empty);
    PROP_UNIQUE(object_types);
    object_types->add("core.object.product");

    basket->getNeighs(
        products,       // out
        "",             // name
        0,              // get removed
        object_types
    );

    for (products_it = products->begin();
        products_it != products->end();
        products_it++)
    {
        ObjectProductSptr product;
        //ObjectSptr        object;

        product = dynamic_pointer_cast<ObjectProduct>(
            *products_it
        );

        // link product to order
        //object = dynamic_pointer_cast<Object>(product);
        object_order->addNeigh(product);

        // unlink product from basket
        product->neighUnlink(basket->getId());
        basket->neighUnlink(product->getId());

        // save product
        product->save();
    }

    // save order
    object_order->save();

    // save basket
    basket->save();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", order_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "order was created successfully"
    );
*/
}

// ---------------- module ----------------

string ObjectBasket::_getType()
{
    return "core.object.basket";
}

string ObjectBasket::_getTypePerl()
{
    return "core::object::basket::main";
}

int32_t ObjectBasket::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectBasket::_destroy()
{
}

ObjectSptr ObjectBasket::_object_create()
{
    ObjectSptr object;
    OBJECT_BASKET(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectBasket::_getType,
    .type_perl      = ObjectBasket::_getTypePerl,
    .init           = ObjectBasket::_init,
    .destroy        = ObjectBasket::_destroy,
    .object_create  = ObjectBasket::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

