#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/ipnoiseLink/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"

#include "core/object/ipnoiseLinks/main.hpp"

ObjectIPNoiseLinks::ObjectIPNoiseLinks(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoiseLinks::~ObjectIPNoiseLinks()
{
};

void ObjectIPNoiseLinks::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.ipnoise.links");
}

string ObjectIPNoiseLinks::getType()
{
    return ObjectIPNoiseLinks::_getType();
}

void ObjectIPNoiseLinks::do_init_props()
{
    Object::do_init_props();

//    ObjectIdSptr        prop_id;
    ObjectStringSptr    prop_huid;

//    PROP_ID(     prop_id,   "static.ipnoise.links");
    PROP_STRING( prop_huid);

//    INIT_PROP(this, Id,   prop_id);
    INIT_PROP(this, Huid, prop_huid);
}

void ObjectIPNoiseLinks::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoiseLinks::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseLinks::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("huid",  getHuid);

    Object::getAllProps(a_props);
}

int32_t ObjectIPNoiseLinks::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // huid
    if (a_obj.hasField("huid")){
        string              huid;
        ObjectStringSptr    prop_huid;

        huid = a_obj.getStringField("huid");
        PROP_STRING(prop_huid, huid);

        INIT_PROP(this, Huid, prop_huid);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

ObjectIPNoiseLinkSptr ObjectIPNoiseLinks::createIPNoiseLink()
{
    ObjectIPNoiseLinkSptr    object_link;
    object_link = Object::createObject<ObjectIPNoiseLink>(
        "core.ipnoise.link"
    );
    addNeigh(object_link);
    return object_link;
}

void ObjectIPNoiseLinks::actionAddNewLink(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectIPNoiseLinks      *links = (ObjectIPNoiseLinks *)a_object;
    ObjectMapSptr           answer_params;
    ObjectIPNoiseLinkSptr   object_link;
    ObjectStringSptr        prop_link_id;
    string                  link_id;

    // TODO check perms here

    // create link
    object_link = links->createIPNoiseLink();
    link_id     = object_link->getId();
    PROP_STRING(prop_link_id, link_id);

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", prop_link_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "link was created successfully"
    );
*/
}

void ObjectIPNoiseLinks::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectIPNoiseLinks  *object  = NULL;
    ObjectStringSptr    prop_huid;

    object = static_cast<ObjectIPNoiseLinks *>(a_object);

    // search huid
    prop_huid = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("huid")
    );
    object->setHuid(prop_huid);

    // process upper objects
    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

/*
void ObjectIPNoiseLinks::timerSlot()
{
    vector<InterfaceInfoSptr>               ifs;
    vector<InterfaceInfoSptr>::iterator     ifs_it;

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;
    ObjectUniqueSptr    object_types;
    ObjectTimeSptr      cur_time;

    PROP_TIME_NOW(cur_time);

    // get interfaces info
    ObjectIPNoiseLink::getInterfacesInfo(ifs);

    // add new
    for (ifs_it = ifs.begin();
        ifs_it != ifs.end();
        ifs_it++)
    {
        InterfaceInfoSptr       info = *ifs_it;
        ObjectIPNoiseLinkSptr   dev;
        string                  dev_name  = info->getName();
        int32_t                 dev_index = info->getIndex();

        PDEBUG(50, "check interface:"
            " dev_name: '%s',"
            " dev_index: '%d'\n",
            dev_name.c_str(),
            dev_index
        );

        dev = getNeigh<ObjectIPNoiseLink>(dev_name);
        if (!dev){
            dev = createNeigh<ObjectIPNoiseLink>(
                "core.object.ipnoise.link",
                dev_name
            );
            dev->setupLink(dev_name, dev_index);
            PDEBUG(10, "device: '%s'"
                " (index: '%d') was added"
                " for watching\n",
                dev_name.c_str(),
                dev_index
            );
        }
        dev->setCheckTime(cur_time);
        dev->save();

        //dev->send(ObjectIPNoiseSkbSptr());
    }

    PROP_UNIQUE(object_types);
    object_types->add("object.ipnoise.link");

    // remove old
    getNeighs(
        links,
        "",             // name
        0,              // get removed
        object_types
    );
    for (links_it = links->begin();
        links_it != links->end();
        links_it++)
    {
        ObjectIPNoiseLinkSptr   cur_link;
        ObjectTimeSptr          cur_link_time;
        ObjectStringSptr        cur_link_name;
        ObjectInt32Sptr         cur_link_index;

        cur_link = dynamic_pointer_cast<ObjectIPNoiseLink>(
            *links_it
        );

        cur_link_time  = cur_link->getCheckTime();
        cur_link_name  = cur_link->getName();
        cur_link_index = cur_link->getIfIndex();

        PWARN("cur link:\n"
            "  dev name:   '%s'\n"
            "  dev index:  '%d'\n"
            "  check time: '%s'\n"
            "  cur time:   '%s'\n",
            cur_link_name->toString()->c_str(),
            cur_link_index->getVal(),
            cur_link_time->toString()->c_str(),
            cur_time->toString()->c_str()
        );

        if (cur_link_time->getVal() != cur_time->getVal()){
            PDEBUG(10, "device: '%s' was removed"
                " from watching\n",
                cur_link_name->toString()->c_str()
            );
            cur_link->remove();
        }
    }
}
*/

// ---------------- module ----------------

static ModuleInfo info = {
    .type           = ObjectIPNoiseLinks::_getType,
    .type_perl      = ObjectIPNoiseLinks::_getTypePerl,
    .init           = ObjectIPNoiseLinks::_init,
    .destroy        = ObjectIPNoiseLinks::_destroy,
    .object_create  = ObjectIPNoiseLinks::_object_create,
    .flags          = 0
};

string ObjectIPNoiseLinks::_getType()
{
    return "core.object.ipnoise.links";
}

string ObjectIPNoiseLinks::_getTypePerl()
{
    return "core::object::ipnoiseLinks::main";
}

int32_t ObjectIPNoiseLinks::_init()
{
    int32_t err = 0;
/*
    DB not available while modules initing

    DbSptr db   = Thread::getCurThreadDb();

    // should never happen, but sometimes shit happens
    if (info.object){
        PWARN("info.object not empty\n");
        info.object = ObjectIPNoiseLinksSptr();
    }

    // create static object
    info.object = db->getCreateObjectById(
        "static.ipnoise.links",
        "core.object.ipnoise.links"
    );
*/
    return err;
}

void ObjectIPNoiseLinks::_destroy()
{
}

ObjectSptr ObjectIPNoiseLinks::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE_LINKS(object);
    return object;
}

REGISTER_MODULE(info);

