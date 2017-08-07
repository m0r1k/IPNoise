#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/dialogMsg/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"
#include "base64.hpp"

#include "core/object/dialog/main.hpp"

ObjectDialog::ObjectDialog(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDialog::~ObjectDialog()
{
};

void ObjectDialog::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDialog::getType()
{
    return ObjectDialog::_getType();
}

void ObjectDialog::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.dialog");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectDialog::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("message_add", ObjectDialog::actionMessageAdd);

    // register events
}

int32_t ObjectDialog::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDialog::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDialog::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectDialog::actionMessageAdd(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectDialog        *dialog     = (ObjectDialog *)a_object;
    string              dialog_id   = dialog->getId();
    ObjectMapSptr       answer_params;
    ObjectDialogMsgSptr object_dialog_msg;
    ObjectStringSptr    prop_msg_id;
    string              msg_id;
    ObjectSptr          object_text_base64;
    ObjectStringSptr    prop_msg_text;
    string              msg_text_base64;
    string              msg_text;
    ObjectUserSptr      cur_user;
    ObjectActionSptr    action;

    // get curent user
    cur_user = Context::getCurUser();

    // TODO check perms here

    if (a_req_params){
        object_text_base64 = a_req_params->get("text_base64");
    }
    if (object_text_base64){
        msg_text_base64 = object_text_base64->toString();
    }

    // decode base64
    msg_text = base64_decode(msg_text_base64);

    // search text_base64
    if (!msg_text.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "empty or missing argument 'text_base64'"
            " or it is not Base64 string"
            " or base64 string is empty"
        );
        goto fail;
    }

    // create message
    object_dialog_msg   = dialog->createMessage();
    msg_id              = object_dialog_msg->getId();
    PROP_STRING(prop_msg_id, msg_id);

    // setup message
    PROP_STRING(prop_msg_text, msg_text);
    object_dialog_msg->setText(prop_msg_text);

    // save message
    object_dialog_msg->save();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", prop_msg_id);

    // create action
    OBJECT_ACTION(action, "message_added");

    action->addParam("msg_id",       msg_id);
    action->addParam("object_id",    dialog_id);

    cur_user->addAction(action);
    cur_user->schedule();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "message was created successfully"
    );

out:
    return;
fail:
    goto out;
*/
}

// ---------------- message --------------

ObjectDialogMsgSptr ObjectDialog::createMessage()
{
    ObjectDialogMsgSptr    object_dialog_msg;
    object_dialog_msg = Object::createObject<ObjectDialogMsg>(
        "core.object.dialog.msg"
    );
    addNeigh(object_dialog_msg);
    return object_dialog_msg;
}

// ---------------- module ----------------

string ObjectDialog::_getType()
{
    return "core.object.dialog";
}

string ObjectDialog::_getTypePerl()
{
    return "core::object::dialog::main";
}

int32_t ObjectDialog::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDialog::_destroy()
{
}

ObjectSptr ObjectDialog::_object_create()
{
    ObjectSptr object;
    OBJECT_DIALOG(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDialog::_getType,
    .type_perl      = ObjectDialog::_getTypePerl,
    .init           = ObjectDialog::_init,
    .destroy        = ObjectDialog::_destroy,
    .object_create  = ObjectDialog::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

