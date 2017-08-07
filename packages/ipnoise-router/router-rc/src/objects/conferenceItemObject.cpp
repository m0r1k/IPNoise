/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "objects/conferenceItemObject.h"

ConferenceItemObject * ConferenceItemObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferenceItemObject(a_node, a_doc, a_tagname);
}

ConferenceItemObject::ConferenceItemObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    string pwd = createPwd();
    setAttributeSafe("pwd", pwd);
};

ConferenceItemObject::~ConferenceItemObject()
{
};

int32_t ConferenceItemObject::autorun()
{
    int err         = 0;
    int priveleges  = 0;

    priveleges = getDocument()->getPriveleges();

    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this,
            "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

ItemObject * ConferenceItemObject::getItem()
{
    UserObject  *user   = NULL;
    ItemObject  *item   = NULL;
    string      huid    = getHuid();

    if (not huid.size()){
        goto out;
    }

    user = getOwner();
    if (not user){
        goto out;
    }

    item = user->getItem(huid);

out:
    return item;
}

string ConferenceItemObject::createPwd()
{
    return getRand(20);
}

void ConferenceItemObject::setPwd(const string &a_pwd)
{
    setAttribute("pwd", a_pwd);
}

string ConferenceItemObject::getPwd()
{
    return getAttribute("pwd");
}

ConferenceObject * ConferenceItemObject::getConferenceObject()
{
    ConferenceItemsObject *conference_items = NULL;
    conference_items = getConferenceItemsObject();
    return conference_items->getConferenceObject();
}

ConferenceItemsObject * ConferenceItemObject::getConferenceItemsObject()
{
    ConferenceItemsObject *conference_items = NULL;

    conference_items = (ConferenceItemsObject *)getParentNode();
    assert ("ipn_conference_items" == conference_items->getTagName());

    return conference_items;
}

ConferenceItemObject * ConferenceItemObject::getConferenceItemByHuid(
    const string &a_huid)
{
    ConferenceItemsObject *conference_items = NULL;
    conference_items = getConferenceItemsObject();
    return conference_items->getConferenceItemByHuid(a_huid);
}

void ConferenceItemObject::setHuid(const string &huid)
{
    setAttribute("huid", huid);
}

string ConferenceItemObject::getHuid()
{
    return getAttribute("huid");
}

void ConferenceItemObject::setSrcPwd(const string &src_pwd)
{
    setAttribute("src_pwd", src_pwd);
}

string ConferenceItemObject::getSrcPwd()
{
    return getAttribute("src_pwd");
}

void ConferenceItemObject::setSrcConfId(const string &src_conf_id)
{
    setAttribute("src_conf_id", src_conf_id);
}

string ConferenceItemObject::getSrcConfId()
{
    return getAttribute("src_conf_id");
}

ssize_t ConferenceItemObject::queue(DomElement *a_data)
{
    ssize_t err = -1;
    ItemObject      *item   = NULL;
    UserObject      *user   = NULL;

    // get conference item huid
    string huid = getHuid();
    if (huid.empty()){
        goto out;
    }

    // get owner
    user = getOwner();

    // 1. may be it owner?
    if (huid == user->getHuid()){
        SkBuffObject    *skb        = NULL;
        DomElement      *out_dom    = NULL;

        skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
        skb->setAttributeSafe("to",             user->getHuid());
        skb->setAttributeSafe("from",           user->getHuid());
        skb->setAttributeSafe("outdev_name",    "lo");

        out_dom = skb->getOutDom(1);
        out_dom->appendChildSafe(a_data->clone());
        getDocument()->emitSignalAndDelObj("packet_outcoming", skb);

        // all ok
        err = 0;
        goto out;
    }

    // 2. may be it item?
    item = user->getItem(huid);
    if (not item){
        goto out;
    }

    do {
        DomElement *commands_el = NULL;
        vector<DomElement *>            commands;
        vector<DomElement *>::iterator  commands_it;

        commands_el = a_data->getElementByTagName("commands");
        if (not commands_el){
            break;
        }
        commands_el->getElementsByTagName("command", commands);
        for (commands_it = commands.begin();
            commands_it != commands.end();
            commands_it++)
        {
            DomElement *cur_command = *commands_it;
            DomElement *cur_msg     = NULL;
            if ("conferenceMsg" == cur_command->getAttribute("type")){
                cur_msg = cur_command->getElementByTagName("message");
                if (cur_msg){
                    // remove msg 'type' element
                    cur_msg->delAttributeSafe("type");
                }
            }
        }

        item->queue(a_data);
        // all ok
        err = 0;
    } while (0);

out:
    return err;
}

ssize_t ConferenceItemObject::queue(const string &a_data)
{
    ssize_t err = -1;
    DomElement *element = NULL;

    if (not a_data.empty()){
        element = getDocument()->createElementFromContent(a_data);
        // TODO XXX
        // temporary add us to DomModel else getElementsByXpath will not works
        // 20120109 morik@
        appendChildSafe(element);
    }

    if (element){
        err = queue(element);
        DomDocument::deleteElement(element);
    }

    return err;
}

void ConferenceItemObject::addLinks(
    const string    &a_huid,
    DomElement      *a_out)
{
    IPNoiseObject       *ipnoise        = NULL;
    ServerObject        *server         = NULL;

    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());
    server = ipnoise->getServerObject();

    server->addLinks(a_huid, a_out);
}

int ConferenceItemObject::doInvite()
{
    int err = 0;
    ConferenceObject *conference = NULL;

    vector <ConferenceItemObject *>               conference_items;
    vector <ConferenceItemObject *>::iterator     conference_items_it;

    conference = getConferenceObject();
    conference->getConferenceItems(conference_items);

    string  our_huid    = getHuid();
    string  owner_huid  = getOwner()->getHuid();

    // inform all members about new party
    {
        for (conference_items_it = conference_items.begin();
            conference_items_it != conference_items.end();
            conference_items_it++)
        {
            DomElement           *ipnoise           = NULL;
            DomElement           *events            = NULL;
            DomElement           *event             = NULL;
            ConferenceItemObject *conference_item   = NULL;

            conference_item = *conference_items_it;

            if (our_huid == conference_item->getHuid()){
                // skip added item
                continue;
            }

            string conf_id;
            string conf_pwd1;

            if (conference->getOwner()->getHuid()
                == conference_item->getHuid())
            {
                // it is owner
                conf_id     = conference->getId();
                conf_pwd1   = conference_item->getPwd();
            } else {
                conf_id     = conference_item->getSrcConfId();
                conf_pwd1   = conference_item->getSrcPwd();
            }

            event = getDocument()->createElement("event");
            event->setAttributeSafe("type",         "conferenceInvited");
            event->setAttributeSafe("ver",          "0.01");
            event->setAttributeSafe("conf_id",      conf_id);
            event->setAttributeSafe("conf_pwd1",    conf_pwd1);
            event->setAttributeSafe("huid",         our_huid);

            // add information about links
            addLinks(our_huid, event);

            events = getDocument()->createElement("events");
            events->appendChildSafe(event);

            ipnoise = getDocument()->createElement("ipnoise");
            ipnoise->setAttributeSafe("ver", "0.01");
            ipnoise->appendChildSafe(events);

            // send
            conference_item->queue(ipnoise);

            // free
            DomDocument::deleteElement(ipnoise);
        }
    }

    {
        // do invite item
        SkBuffObject    *skb    = NULL;
        UserObject      *owner  = NULL;

        DomElement *ipnoise     = NULL;
        DomElement *commands    = NULL;
        DomElement *command     = NULL;
        DomElement *members     = NULL;
        DomElement *member      = NULL;
        DomElement *out_dom     = NULL;

        owner = conference->getOwner();

        skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
        skb->setAttributeSafe("from",           owner->getHuid());
        skb->setAttributeSafe("to",             getHuid());
        skb->setAttributeSafe("outdev_name",    "ipnoise");

        out_dom = skb->getOutDom();

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        commands = skb->getDocument()->createElement("commands");
        ipnoise->appendChildSafe(commands);

        command = skb->getDocument()->createElement("command");
        commands->appendChildSafe(command);

        // add information about conference members
        for (conference_items_it = conference_items.begin();
            conference_items_it != conference_items.end();
            conference_items_it++)
        {
            ConferenceItemObject *conference_item = NULL;
            string conf_item_huid;
            string conf_item_nickname;

            vector<NeighObject *>               neighs;
            vector<NeighObject *>::iterator     neighs_it;

            conference_item = *conference_items_it;

            if (our_huid == conference_item->getHuid()){
                // skip our self
                continue;
            }

            if (owner_huid == conference_item->getHuid()){
                // skip owner
                continue;
            }

            conf_item_huid     = conference_item->getHuid();
            conf_item_nickname = conference_item->getItem()
                ->getAPINickName();

            if (!members){
                members = skb->getDocument()->createElement(
                    "members");
                command->appendChildSafe(members);
            }

            member = skb->getDocument()->createElement("member");
            member->setAttributeSafe("huid",        conf_item_huid);
            member->setAttributeSafe("nickname",    conf_item_nickname);

            // add information about links
            addLinks(conf_item_huid, member);

            members->appendChildSafe(member);
        }

        command->setAttributeSafe("type",       "conferenceInvite");
        command->setAttributeSafe("ver",        "0.01");
        command->setAttributeSafe("conf_id",    conference->getId());
        command->setAttributeSafe("conf_pwd0",  conference->getPwd0());
        command->setAttributeSafe("conf_topic", conference->getTopic());

        getDocument()->emitSignalAndDelObj("packet_outcoming", skb);
    }

    return err;
}

