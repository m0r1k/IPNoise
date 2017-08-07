/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "apiCommandClientAddItem.hpp"

ApiCommandClientAddItem::ApiCommandClientAddItem()
    :   ApiCommandClient(API_COMMAND_CLIENT_ADDITEM)
{
}

ApiCommandClientAddItem::~ApiCommandClientAddItem()
{
}

int ApiCommandClientAddItem::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user           = NULL;
    ItemObject      *item           = NULL;
    DomElement      *element        = NULL;
    NeighsObject    *neighs         = NULL;
    DomElement      *item4add       = NULL;
    DomElement      *contacts4add   = NULL;
    DomElement      *links4add      = NULL;
    DomElement      *link4add       = NULL;
    DomElement      *contact4add    = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = "";
    string          item4add_huid   = "";
    string          answer          = "";
    string          contacts_errors = "";

    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    ServerObject    *server     = NULL;
    IPNoiseObject   *ipnoise    = NULL;

    // get ipnoise
    ipnoise = (IPNoiseObject *)nc->getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();

    // search session
    session = nc->getSession(skb);
    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session ID
    sessid = session->getSessId();

    // search user for this session
    user = session->getOwner();
    if (!user){
        err = nc->errorNotLogged(skb);
        goto out;
    }

    // search "item" for add
    item4add = nc->getElementByTagName("item");
    if (!item4add){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get item's huid (item for add)
    item4add_huid = item4add->getAttribute("huid");
    if (!item4add_huid.size()){
        HUID huid = server->generateLocalHuid();
        item4add_huid = huid.toString();
    }

    // check, maybe item already exist?
    item = user->getItem(item4add_huid);
    if (item){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"addItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4add_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"addItemFailed.1\"";
        answer +=         ">";
        answer +=           "item exist";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // search contacts for add (check errors)
    contacts4add = item4add->getElementByTagName("contacts");
    if (contacts4add){
        res.clear();
        contacts4add->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            ContactObject       *contact        = NULL;
            string              contact_id_str  = "";
            string              err_string      = "";
            contact4add                         = *res_it;

            // get contact ID for add
            contact_id_str = contact4add->getAttribute("id");

            // check what contact exist
            if (contact_id_str.size()){
                contact_id = (ContactObject::Id)(
                    atoi(contact_id_str.c_str())
                );
                contact = user->getContact(contact_id);
            }
            if (!contact){
                err_string += " not_found=\"1\"";
            } else {
                if (contact->isSystem()){
                    err_string += " is_system=\"1\"";
                }
            }
            if (err_string.size()){
                contacts_errors += "<contact"
                    " id=\""    + contact_id_str + "\""
                    " "         + err_string
                    + "/>\n";
            }
        }
    }

    if (contacts_errors.size()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"addItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4add_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"addItemFailed.2\"";
        answer +=         ">";
        answer +=           "error(s) while work with contact(s)";
        answer +=         "</description>";
        answer +=         "<contacts>";
        answer +=           contacts_errors;
        answer +=         "</contacts>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // ok, looks like good, now add item
    item = user->addCreateItem(item4add_huid);

    // get "ipn_neighs" object
    neighs = server->getCreateNeighsObject();
    if (!neighs){
        err = nc->errorInternalError(skb);
        goto out;
    }

    // add item to requested contacts
    contacts4add = item4add->getElementByTagName("contacts");
    if (contacts4add){
        res.clear();
        contacts4add->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            ContactObject       *contact        = NULL;
            string              contact_id_str  = "";
            contact4add                         = *res_it;

            // get contact ID for add
            contact_id_str = contact4add->getAttribute("id");

            // check what contact exist
            if (contact_id_str.size()){
                contact_id = (ContactObject::Id)(
                    atoi(contact_id_str.c_str())
                );
                contact = user->getContact(contact_id);
            }
            if (contact){
                contact->getCreateContactItem(item4add_huid);
            }
        }
    }

    // add item properties if exist
    element = item4add->getElementByTagName("nickname");
    if (element){
        string nickname = element->getData();
        item->setAPINickName(nickname);
    }
    element = item4add->getElementByTagName("icon");
    if (element){
        string icon = element->getData();
        item->setAPIIcon(icon);
    }

    // add neighs if exist
    links4add = item4add->getElementByTagName("links");
    if (links4add){
        res.clear();
        links4add->getElementsByTagName("link", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            string dev;
            string lladdr;

            link4add = *res_it;
            dev     = link4add->getAttribute("name") + "_0";
            lladdr  = link4add->getAttribute("address");

            // TODO report about errors here
            // (empty or incorrect dev or lladdr name)
            // 20110803 morik@
            neighs->addNeigh(item->getHuid(), dev, lladdr);
        }
    }

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"addItemSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " huid=\"" + item4add_huid + "\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"addItemSuccess.1\"";
    answer +=         ">";
    answer +=           "item was added successful";
    answer +=         "</description>";
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer to current client
    session->queue(answer);

    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"updateContactList\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    user->getContactList(answer);
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer to all clients
    user->queue(answer);

out:
    return err;
}

