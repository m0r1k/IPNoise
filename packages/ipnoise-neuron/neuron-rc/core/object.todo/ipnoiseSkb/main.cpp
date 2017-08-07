#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/ipnoiseSkb/main.hpp"

ObjectIPNoiseSkb::ObjectIPNoiseSkb(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoiseSkb::~ObjectIPNoiseSkb()
{
};

void ObjectIPNoiseSkb::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectIPNoiseSkb::getType()
{
    return ObjectIPNoiseSkb::_getType();
}

void ObjectIPNoiseSkb::do_init_props()
{
    Object::do_init_props();

    ObjectStringSptr    prop_in_link_name;
    ObjectInt32Sptr     prop_in_link_index;
    ObjectStringSptr    prop_in_content;
    ObjectStringSptr    prop_from;
    ObjectStringSptr    prop_top;

    PROP_STRING(prop_in_link_name);
    PROP_INT32(prop_in_link_index);
    PROP_STRING(prop_in_content);
    PROP_STRING(prop_from);
    PROP_STRING(prop_top);

//    INIT_PROP(this, Type,        prop_type);
    INIT_PROP(this, InLinkName,  prop_in_link_name);
    INIT_PROP(this, InLinkIndex, prop_in_link_index);
    INIT_PROP(this, InContent,   prop_in_content);
    INIT_PROP(this, From,        prop_from);
    INIT_PROP(this, To,          prop_top);
}

void ObjectIPNoiseSkb::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoiseSkb::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseSkb::getAllProps(
    ObjectMapSptr a_props)
{
 //   SAVE_PROP("dst_huid",  getDstHuid);
 //   SAVE_PROP("src_huid",  getSrcHuid);
 //   SAVE_PROP("lladdr",    getLLAddr);
 //   SAVE_PROP("dev_name",  getDevName);

    Object::getAllProps(a_props);
}

int32_t ObjectIPNoiseSkb::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    PFATAL("FIXME\n");

 //   string  dst_huid;
 //   string  src_huid;
 //   string  lladdr;
 //   string  dev_name;
 //
 //   // dst_huid
 //   if (a_obj.hasField("dst_huid")){
 //       dst_huid = a_obj.getStringField("dst_huid");
 //       setDstHuid(PROP_STRING(
 //           dst_huid
 //       ));
 //   }
 //
 //   // src_huid
 //   if (a_obj.hasField("src_huid")){
 //       src_huid = a_obj.getStringField("src_huid");
 //       setSrcHuid(PROP_STRING(
 //           src_huid
 //       ));
 //   }
 //
 //   // lladdr
 //   if (a_obj.hasField("lladdr")){
 //       lladdr = a_obj.getStringField("lladdr");
 //       setLLAddr(PROP_STRING(
 //           lladdr
 //       ));
 //   }
 //
 //   // dev_name
 //   if (a_obj.hasField("dev_name")){
 //       dev_name = a_obj.getStringField("dev_name");
 //       setDevName(PROP_STRING(
 //           dev_name
 //       ));
 //   }

    err = Object::parseBSON(a_obj);
    return err;
}

 //       string ObjectIPNoiseSkb::getLinkAddress()
 //       {
 //           return getLLAddr()->toString();
 //       }
 //
 //       string ObjectIPNoiseSkb::getLinkName()
 //       {
 //           string dev = getDevName()->toString();
 //           string ret = dev;
 //
 //           if ("udp_v4_" == dev.substr(0, 7)){
 //               ret = "udp_v4";
 //           } else if ("tcp_v4_" == dev.substr(0, 7)){
 //               ret = "tcp_v4";
 //           } else if ("icmp_v4_" == dev.substr(0, 8)){
 //               ret = "icmp_v4";
 //           } else {
 //               ret = "internal_error";
 //           }
 //
 //           return ret;
 //       }

// ---------------- module ----------------

string ObjectIPNoiseSkb::_getType()
{
    return "core.object.ipnoise.skb";
}

string ObjectIPNoiseSkb::_getTypePerl()
{
    return "core::object::ipnoiseSkb::main";
}

int32_t ObjectIPNoiseSkb::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseSkb::_destroy()
{
}

ObjectSptr ObjectIPNoiseSkb::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE_SKB(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectIPNoiseSkb::_getType,
    .type_perl      = ObjectIPNoiseSkb::_getTypePerl,
    .init           = ObjectIPNoiseSkb::_init,
    .destroy        = ObjectIPNoiseSkb::_destroy,
    .object_create  = ObjectIPNoiseSkb::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

