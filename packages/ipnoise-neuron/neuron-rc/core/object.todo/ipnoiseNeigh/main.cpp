#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/ipnoiseNeigh/main.hpp"

ObjectIPNoiseNeigh::ObjectIPNoiseNeigh(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoiseNeigh::~ObjectIPNoiseNeigh()
{
};

void ObjectIPNoiseNeigh::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectIPNoiseNeigh::getType()
{
    return ObjectIPNoiseNeigh::_getType();
}

void ObjectIPNoiseNeigh::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr      prop_type;
    ObjectStringSptr    prop_dst_huid;
    ObjectStringSptr    prop_src_huid;
    ObjectStringSptr    prop_ll_addr;
    ObjectStringSptr    prop_dev_name;

    PROP_STRING(prop_dst_huid);
    PROP_STRING(prop_src_huid);
    PROP_STRING(prop_ll_addr);
    PROP_STRING(prop_dev_name);

    INIT_PROP(this, DstHuid, prop_dst_huid);
    INIT_PROP(this, SrcHuid, prop_src_huid);
    INIT_PROP(this, LLAddr,  prop_ll_addr);
    INIT_PROP(this, DevName, prop_dev_name);
}

void ObjectIPNoiseNeigh::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoiseNeigh::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseNeigh::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("dst_huid",  getDstHuid);
    SAVE_PROP("src_huid",  getSrcHuid);
    SAVE_PROP("lladdr",    getLLAddr);
    SAVE_PROP("dev_name",  getDevName);

    Object::getAllProps(a_props);
}

int32_t ObjectIPNoiseNeigh::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // dst_huid
    if (a_obj.hasField("dst_huid")){
        ObjectStringSptr    prop_dst_huid;
        string              dst_huid;

        dst_huid = a_obj.getStringField("dst_huid");
        PROP_STRING(prop_dst_huid, dst_huid);

        INIT_PROP(this, DstHuid, prop_dst_huid);
    }

    // src_huid
    if (a_obj.hasField("src_huid")){
        ObjectStringSptr    prop_src_huid;
        string              src_huid;

        src_huid = a_obj.getStringField("src_huid");
        PROP_STRING(prop_src_huid, src_huid);

        INIT_PROP(this, SrcHuid, prop_src_huid);
    }

    // lladdr
    if (a_obj.hasField("lladdr")){
        ObjectStringSptr    prop_lladdr;
        string              lladdr;

        lladdr = a_obj.getStringField("lladdr");
        PROP_STRING(prop_lladdr, lladdr);

        INIT_PROP(this, LLAddr, prop_lladdr);
    }

    // dev_name
    if (a_obj.hasField("dev_name")){
        ObjectStringSptr    prop_dev_name;
        string              dev_name;

        dev_name = a_obj.getStringField("dev_name");
        PROP_STRING(prop_dev_name, dev_name);

        INIT_PROP(this, DevName, prop_dev_name);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectIPNoiseNeigh::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectIPNoiseNeigh  *object  = NULL;

    ObjectStringSptr    object_dst_huid;
    ObjectStringSptr    object_src_huid;
    ObjectStringSptr    object_lladdr;
    ObjectStringSptr    object_dev_name;

    object = static_cast<ObjectIPNoiseNeigh *>(a_object);

    // search dst_huid
    object_dst_huid = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("dst_huid")
    );
    object->setDstHuid(object_dst_huid);

    // search src_huid
    object_src_huid = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("src_huid")
    );
    object->setSrcHuid(object_src_huid);

    // search lladdr
    object_lladdr = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("lladdr")
    );
    object->setLLAddr(object_lladdr);

    // search dev_name
    object_dev_name = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("dev_name")
    );
    object->setDevName(object_dev_name);

    // process upper objects
    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

string ObjectIPNoiseNeigh::getLinkAddress()
{
    return getLLAddr()->toString();
}

string ObjectIPNoiseNeigh::getLinkName()
{
    string dev = getDevName()->toString();
    string ret = dev;

    if ("udp_v4_" == dev.substr(0, 7)){
        ret = "udp_v4";
    } else if ("tcp_v4_" == dev.substr(0, 7)){
        ret = "tcp_v4";
    } else if ("icmp_v4_" == dev.substr(0, 8)){
        ret = "icmp_v4";
    } else {
        ret = "internal_error";
    }

    return ret;
}

void ObjectIPNoiseNeigh::timer()
{
 //   int32_t recheck_fail_max    = getRecheckFailMax();
 //   int32_t recheck_after       = getRecheckAfter();
 //   int32_t recheck_time        = getRecheckTime();
 //   int32_t down_time           = getDownTime();
 //
 //   recheck_after--;
 //   if (recheck_after <= 0){
 //       vector<DomElement *> res;
 //       getElementsByTagName("ipn_neigh_checkings", res);
 //       vector<>
 //       if (recheck_fail_max <= int32_t(res.size())){
 //           // reset src huid (going to slow path)
 //           setSrcHuid("");
 //           // max fail count reached
 //           deleteChildsSafe();
 //           // update recheck_time
 //           recheck_time *= 2;
 //           setRecheckTime(recheck_time);
 //           // update recheck_after
 //           recheck_after = recheck_time;
 //           // update down time
 //           down_time += recheck_time * res.size();
 //           setDownTime(down_time);
 //           // set neigh down
 //           setStateDown();
 //       } else {
 //           // reset recheck_after
 //           recheck_after = recheck_time;
 //           // check neigh
 //           doCheck();
 //       }
 //   }
 //
 //   // update recheck after
 //   setRecheckAfter(recheck_after);
 //
 //   // setup timer
 //   setTimer("+1");
}

 //       void ObjectIPNoiseNeigh::doCheck()
 //       {
 //           vector<UserObject *>            users;
 //           vector<UserObject *>::iterator  users_it;
 //           map<const string, int>          src_huids;
 //
 //           string src_huid = getSrcHuid();
 //
 //           // maybe fast path?
 //           if (src_huid.size()){
 //               // yea! we known working source huid
 //               doCheck(src_huid);
 //               goto out;
 //           }
 //
 //           // we have not source huid :(
 //           // so send searchRoute commands from all local users names
 //           server->getCreateUsersObject()->getUsers(users);
 //           for (users_it = users.begin();
 //               users_it != users.end();
 //               users_it++)
 //           {
 //               // start checking
 //               UserObject  *user   = *users_it;
 //               string      huid    = user->getHuid();
 //
 //               // skip multicast user
 //               if (MULTICAST_USER_HUID == huid){
 //                   continue;
 //               }
 //
 //               // store huid
 //               src_huids[huid] = 1;
 //           }
 //
 //           // check
 //           doCheck(src_huids);
 //
 //
 //       out:
 //           return;
 //       }
 //
 //       void ObjectIPNoiseNeigh::doCheck(
 //           const string &a_src_huid)
 //       {
 //           map<const string, int> src_huids;
 //           src_huids[a_src_huid] = 1;
 //           doCheck(src_huids);
 //       }
 //
 //       void ObjectIPNoiseNeigh::doCheck(
 //           map<const string, int> &a_src_huids)
 //       {
 //           map<const string, int>::iterator src_huids_it;
 //
 //           string dst_huid = getDstHuid();
 //           string lladdr   = getLLAddr();
 //           string dev      = getDev();
 //
 //           DomElement *ipn_neigh_checkings = NULL;
 //
 //           if (not a_src_huids.size()){
 //               goto out;
 //           }
 //
 //           ipn_neigh_checkings = getDocument()->createElement(
 //               "ipn_neigh_checkings"
 //           );
 //           appendChildSafe(ipn_neigh_checkings);
 //
 //           for (src_huids_it = a_src_huids.begin();
 //               src_huids_it != a_src_huids.end();
 //               src_huids_it++)
 //           {
 //               string          src_huid    = src_huids_it->first;
 //               SkBuffObject    *skb        = NULL;
 //               DomElement      *ipnoise    = NULL;
 //               DomElement      *command    = NULL;
 //               DomElement      *commands   = NULL;
 //               DomElement      *out_dom    = NULL;
 //
 //               skb = getDocument()->createElement<SkBuffObject>(
 //                   "ipn_skbuff"
 //               );
 //               skb->setFrom(src_huid);
 //               skb->setTo(dst_huid);
 //               skb->setToLL(lladdr);
 //               skb->setOutDevName(dev);
 //
 //               // create command
 //               out_dom = skb->getOutDom(1);
 //
 //               ipnoise = skb->getDocument()->createElement("ipnoise");
 //               out_dom->appendChildSafe(ipnoise);
 //
 //               command = skb->getDocument()->createElement("command");
 //               command->setAttributeSafe("type", "searchRoute");
 //               commands = skb->getDocument()->createElement("commands");
 //               commands->appendChildSafe(command);
 //               ipnoise->appendChildSafe(commands);
 //
 //               // append "up" neighs
 //               NetCommandObject::appendNeighsUp(command);
 //
 //               // create "ipn_neigh_checking" item
 //               {
 //                   string      checking_id  = getRand(20);
 //                   DomElement  *checking    = NULL;
 //
 //                   checking = getDocument()->createElement(
 //                       "ipn_neigh_checking"
 //                   );
 //
 //                   // delete element after exit
 //                   checking->setExpired("0");
 //
 //                   // set checking ID
 //                   checking->setAttributeSafe("id",        checking_id);
 //                   checking->setAttributeSafe("src_huid",  src_huid);
 //                   command->setAttributeSafe("id",         checking_id);
 //
 //                   // add checking item
 //                   ipn_neigh_checkings->appendChildSafe(checking);
 //               }
 //
 //               // send packet,
 //               // NOTE: packet will be deleted after sendPacket call
 //               // tx packet (emitSignal will create clone of element)
 //               skb->prepare();
 //               getDocument()->emitSignalAndDelObj(
 //                   "packet_outcoming",
 //                   skb
 //               );
 //           }
 //
 //       out:
 //           return;
 //       }

// ---------------- module ----------------

string ObjectIPNoiseNeigh::_getType()
{
    return "core.object.ipnoise.neigh";
}

string ObjectIPNoiseNeigh::_getTypePerl()
{
    return "core::object::ipnoiseNeigh::main";
}

int32_t ObjectIPNoiseNeigh::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoiseNeigh::_destroy()
{
}

ObjectSptr ObjectIPNoiseNeigh::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE_NEIGH(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectIPNoiseNeigh::_getType,
    .type_perl      = ObjectIPNoiseNeigh::_getTypePerl,
    .init           = ObjectIPNoiseNeigh::_init,
    .destroy        = ObjectIPNoiseNeigh::_destroy,
    .object_create  = ObjectIPNoiseNeigh::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

