#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/dialog/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dialogs/main.hpp"

ObjectDialogs::ObjectDialogs(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDialogs::~ObjectDialogs()
{
};

void ObjectDialogs::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDialogs::getType()
{
    return ObjectDialogs::_getType();
}

void ObjectDialogs::do_init_props()
{
    Object::do_init_props();
}

void ObjectDialogs::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction(
        "add_new_dialog",
        ObjectDialogs::actionAddNewDialog
    );
}

int32_t ObjectDialogs::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDialogs::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDialogs::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectDialogs::actionAddNewDialog(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectDialogs       *dialogs = NULL;
    ObjectMapSptr       answer_params;
    ObjectDialogSptr    object_dialog;
    ObjectStringSptr    prop_dialog_id;
    string              dialog_id;

    // TODO check perms here

    dialogs = static_cast<ObjectDialogs *>(a_object);

    // create dialog
    object_dialog = dialogs->createDialog();
    dialog_id     = object_dialog->getId();
    PROP_STRING(prop_dialog_id, dialog_id);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", dialog_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "dialog was created successfully"
    );
*/
}

// ---------------- dialog ---------------

ObjectDialogSptr ObjectDialogs::createDialog()
{
    ObjectDialogSptr    object_dialog;
    object_dialog = Object::createObject<ObjectDialog>(
        "core.object.dialog"
    );
    addNeigh(object_dialog);
    return object_dialog;
}

// ---------------- module ----------------

string ObjectDialogs::_getType()
{
    return "core.object.dialogs";
}

string ObjectDialogs::_getTypePerl()
{
    return "core::object::dialogs::main";
}

int32_t ObjectDialogs::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDialogs::_destroy()
{
}

ObjectSptr ObjectDialogs::_object_create()
{
    ObjectSptr object;
    OBJECT_DIALOGS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDialogs::_getType,
    .type_perl      = ObjectDialogs::_getTypePerl,
    .init           = ObjectDialogs::_init,
    .destroy        = ObjectDialogs::_destroy,
    .object_create  = ObjectDialogs::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

