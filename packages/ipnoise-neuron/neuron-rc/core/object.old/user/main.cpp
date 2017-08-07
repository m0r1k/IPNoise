#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
//#include "core/object/basket/main.hpp"
//#include "core/object/bookmarks/main.hpp"
#include "core/object/camera/main.hpp"
//#include "core/object/dialog/main.hpp"
//#include "core/object/dialogs/main.hpp"
//#include "core/object/dream/main.hpp"
//#include "core/object/dreams/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/objectLink/main.hpp"
//#include "core/object/objectLinkInfo/main.hpp"
//#include "core/object/objectLinkAction/main.hpp"
#include "core/object/map/main.hpp"
//#include "core/object/order/main.hpp"
//#include "core/object/orders/main.hpp"
//#include "core/object/param/main.hpp"
//#include "core/object/paramGroup/main.hpp"
//#include "core/object/paramAddress/main.hpp"
//#include "core/object/shop/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"

#include "core/object/user/main.hpp"

ObjectUser::ObjectUser(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectUser::~ObjectUser()
{
};

void ObjectUser::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectUser::getType()
{
    return ObjectUser::_getType();
}

void ObjectUser::do_init_props()
{
    ObjectStringSptr    prop_password;

    Object::do_init_props();

    PROP_STRING(prop_password);

    INIT_PROP(this, Password, prop_password);
}

void ObjectUser::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "getCreateAddress",
        ObjectUser::actionGetCreateAddress
    );
}

int32_t ObjectUser::do_autorun()
{
    int32_t             err = -1;

 //   ObjectDialogsSptr   dialogs;
 //   ObjectBasketSptr    basket;
 //   ObjectCameraSptr    camera;
 //
 //   camera = getCreateNeigh<ObjectCamera>(
 //       "core.object.camera"
 //   );
 //   if (!camera){
 //       PWARN("cannot get/create object ObjectCamera\n");
 //       goto fail;
 //   }

    //dialogs = getCreateNeigh<ObjectDialogs>();
    //if (!dialogs){
    //    PWARN("cannot get/create object ObjectDialogs\n");
    //    goto fail;
    //}

    //basket = getCreateNeigh<ObjectBasket>();
    //if (!basket){
    //    PWARN("cannot get/create object ObjectBasket\n");
    //    goto fail;
    //}

    // all ok
    err = 0;

//out:
    return err;
//fail:
//    if (0 <= err){
//        err = -1;
//    }
//    goto out;
}

void ObjectUser::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("password", getPassword);

    Object::getAllProps(a_props);
}

int32_t ObjectUser::parseBSON(
    mongo::BSONObj  a_obj)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;

    // Password
    if (a_obj.hasField("password")){
        ObjectStringSptr    prop_val;
        string              val;

        val = a_obj.getStringField("password");
        PROP_STRING(prop_val, val);

        INIT_PROP(this, Password, prop_val);
    }

    err = Object::parseBSON(a_obj);
    if (err){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void ObjectUser::beforeRequest(
    ObjectHttpAnswerSptr    a_answer,
    const ObjectMapSptr     a_req_props,
    const ObjectMapSptr     a_req_params)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    // create dialogs object
    //getCreateDialogsObject();

    // process supper
    Object::beforeRequest(
        a_answer,
        a_req_props,
        a_req_params
    );
}

// ---------------- generic ----------------

bool ObjectUser::isAdmin()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    bool ret = false;

/*
    ObjectParamGroupSptr   param_group;

    ObjectMapSptr   groups;
    ObjectMapIt     groups_it;
*/

    if ("god" == getId()){
        ret = true;
        goto out;
    }

/*
    getParams(groups, "group");
    for (groups_it = groups->begin();
        groups_it != groups->end();
        groups_it++)
    {
        ObjectParamGroupSptr cur_group;
        cur_group = dynamic_pointer_cast<ObjectParamGroup>(
            groups_it->second
        );
        ret = cur_group->isAdmin();
        if (ret){
            break;
        }
    }
*/

out:
    return ret;
}


/*
// ---------------- camera ----------------

ObjectCameraSptr ObjectUser::getCamera()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectCameraSptr    object_camera;

    // TODO check perms here
    // 20140820 rchechnev@

    object_camera = getNeigh<ObjectCamera>();

    return object_camera;
}

ObjectCameraSptr ObjectUser::getCreateCamera()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectCameraSptr    object_camera;

    // TODO check perms here
    // 20130724 rchechnev@

    object_camera = getCamera();
    if (!object_camera){
        // create new
        object_camera = Object::createObject<ObjectCamera>(
            "core.object.camera"
        );
        addNeigh(object_camera);
    }

    return object_camera;
}

// ---------------- address ----------------

ObjectParamAddressSptr ObjectUser::getAddress(
    const string    &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectParamAddressSptr  object_address;
    PFATAL("FIXME\n");

    ObjectSptr              tmp;

    // TODO check perms here
    // 20130724 rchechnev@

    tmp             = ThreadDb::get()->getObjectById(a_object_id);
    object_address  = dynamic_pointer_cast<ObjectParamAddress>(tmp);

    return object_address;
}

ObjectParamAddressSptr ObjectUser::getCreateAddress(
    const string &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectParamAddressSptr  object_address;
    PFATAL("FIXME\n");

    // TODO check perms here
    // 20130724 rchechnev@

    object_address = getAddress(a_object_id);
    if (!object_address){
        // create new
        object_address = ThreadDb::get()
            ->createObject<ObjectParamAddress>();
        LINK_AND_SAVE(this, object_address);
    }

    return object_address;
}

// ---------------- orders ----------------

ObjectOrdersSptr ObjectUser::getOrdersObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectOrdersSptr    object_orders;

    // TODO check perms here
    // 20130902 rchechnev@

    object_orders = getNeigh<ObjectOrders>();
    return object_orders;
}

ObjectOrdersSptr ObjectUser::getCreateOrdersObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectOrdersSptr    object_orders;

    PFATAL("FIXME\n");
    object_orders = getOrdersObject();
    if (!object_orders){
        object_orders = ThreadDb::get()
            ->createObject<ObjectOrders>();
        LINK_AND_SAVE(this, object_orders.get());
    }
    return object_orders;
}

// ---------------- dialogs ---------------

ObjectDialogsSptr ObjectUser::getDialogsObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDialogsSptr   object_dialogs;

    // TODO check perms here
    // 20131120 rchechnev@

    object_dialogs = getNeigh<ObjectDialogs>(
        "",                     // name
        "core.object.dialogs"   // type
    );
    return object_dialogs;
}

ObjectDialogsSptr ObjectUser::getCreateDialogsObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDialogsSptr   object_dialogs;
    ObjectContextSptr   context = Thread::getCurContext();

    object_dialogs = getDialogsObject();
    if (!object_dialogs){
        object_dialogs = context->createObject<ObjectDialogs>(
            "core.object.dialogs"
        );
        addNeigh(object_dialogs);
    }

    return object_dialogs;
}

ObjectBookmarksSptr ObjectUser::getBookmarks()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectBookmarksSptr object;
    object = getNeigh<ObjectBookmarks>();
    return object;
}

ObjectBookmarksSptr ObjectUser::getCreateBookmarks()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectBookmarksSptr object;
    PFATAL("FIXME\n");
    object = getBookmarks();
    if (!object){
        object = ThreadDb::get()
            ->createObject<ObjectBookmarks>();
        LINK_AND_SAVE(this, object.get());
    }
    return object;
}

void ObjectUser::addToBookmarks(
    Object  *a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr          object;
    ObjectBookmarksSptr bookmarks;

    bookmarks  = getCreateBookmarks();
    bookmarks->addNeigh(a_object, 0);
    bookmarks->save();
}

void ObjectUser::delFromBookmarks(
    const string &a_neigh_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr          object;
    ObjectBookmarksSptr bookmarks;

    bookmarks  = getCreateBookmarks();
    bookmarks->neighUnlink(a_neigh_id);
    bookmarks->save();
}

// basket
ObjectBasketSptr ObjectUser::getBasket()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectBasketSptr object;
    object = getNeigh<ObjectBasket>();
    return object;
}

ObjectBasketSptr ObjectUser::getCreateBasket()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectBasketSptr object;
    object = getBasket();
    if (!object){
        object = createNeigh<ObjectBasket>(
            "core.object.basket"
        );
    }
    return object;
}

// dreams
ObjectDreamsSptr ObjectUser::getDreamsObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDreamsSptr object;
    object = getNeigh<ObjectDreams>();
    return object;
}

ObjectDreamsSptr ObjectUser::getCreateDreamsObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDreamsSptr object;
    PFATAL("FIXME\n");

    object = getDreamsObject();
    if (!object){
        object = ThreadDb::get()
            ->createObject<ObjectDreams>();
        LINK_AND_SAVE(this, object.get());
    }

    return object;
}

// shop
ObjectShopSptr ObjectUser::getShopObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectShopSptr object;

    object = getNeigh<ObjectShop>(
        "",                 // name
        "core.object.shop"  // type
    );

    return object;
}

ObjectShopSptr ObjectUser::getCreateShopObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectShopSptr      object;
    ObjectContextSptr   context = Thread::getCurContext();

    object = getShopObject();
    if (!object){
        object = context->createObject<ObjectShop>(
            "core.object.shop"
        );
        addNeigh(object);
    }

    return object;
}
*/

// sessions
void ObjectUser::getSessions(
    ObjectVectorSptr    a_out,
    const int32_t       &a_db_lock)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUniqueSptr    object_types;

    PROP_UNIQUE(object_types);
    object_types->add("core.object.session");

    getNeighs(
        a_out,
        "",             // name
        0,              // get removed
        a_db_lock,      // db_lock
        object_types
    );
}

/*
void ObjectUser::addAction(
    ObjectActionSptr a_action)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    sessions;
    ObjectVectorIt      sessions_it;

    PROP_VECTOR(sessions);

    getSessions(sessions);

    for (sessions_it = sessions->begin();
        sessions_it != sessions->end();
        sessions_it++)
    {
        ObjectSessionSptr session;
        session = dynamic_pointer_cast<ObjectSession>(
            *sessions_it
        );
        session->addAction(a_action);
    }
}
*/

// ---------------- static ----------------

void ObjectUser::actionUpdate(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectParamSptr     param_password;
    ObjectStringSptr    prop_password;
    ObjectUser          *object = NULL;

    object = static_cast<ObjectUser *>(a_object);

    // get password
    prop_password = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("password")
    );
    object->setPassword(prop_password);

    // process upper objects
    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/

}

void ObjectUser::actionGetCreateAddress(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectUser              *user = (ObjectUser *)a_object;
    ObjectParamAddressSptr  object_address;
    ObjectMapSptr           answer_params;
    ObjectStringSptr        prop_address_id;
    string                  address_id;

    // get request params
    if (a_req_params){
        prop_address_id = dynamic_pointer_cast<ObjectString>(
            a_req_params->get("address_id")
        );
    }
    if (prop_address_id){
        address_id = prop_address_id->toString();
    }

    if (!address_id.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "missing or empty param: 'address_id'"
        );
        goto fail;
    }

    // get or create address
    object_address = user->getCreateAddress(address_id);

    // create answer
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("address_id", object_address->getId());

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "address was created successfully"
    );

out:
    return;
fail:
    goto out;
*/
}

// ---------------- module ----------------

string ObjectUser::_getType()
{
    return "core.object.user";
}

string ObjectUser::_getTypePerl()
{
    return "core::object::user::main";
}

int32_t ObjectUser::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectUser::_destroy()
{
}

ObjectSptr ObjectUser::_object_create()
{
    ObjectSptr object;
    _OBJECT_USER(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectUser::_getType,
    .type_perl      = ObjectUser::_getTypePerl,
    .init           = ObjectUser::_init,
    .destroy        = ObjectUser::_destroy,
    .object_create  = ObjectUser::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

