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
#include "apiCommandClientUpdateItem.hpp"

ApiCommandClientUpdateItem::ApiCommandClientUpdateItem()
    :   ApiCommandClient(API_COMMAND_CLIENT_UPDATEITEM)
{
}

ApiCommandClientUpdateItem::~ApiCommandClientUpdateItem()
{
}

int ApiCommandClientUpdateItem::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user                   = NULL;
    ItemObject      *item                   = NULL;
    DomElement      *element                = NULL;
    NeighsObject    *neighs                 = NULL;
    DomElement      *item4update            = NULL;
    DomElement      *contacts4del           = NULL;
    DomElement      *links4add              = NULL;
    DomElement      *link4add               = NULL;
    DomElement      *contacts4add           = NULL;
    DomElement      *contact4add            = NULL;
    DomElement      *contact4del            = NULL;
    SessionObject   *session                = NULL;
    string          sessid                  = "";
    string          item4update_huid        = "";
    string          answer                  = "";
    string          contacts_errors         = "";

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

    // search "item" for update
    item4update = nc->getElementByTagName("item");
    if (!item4update){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get item's huid (item for update)
    item4update_huid = item4update->getAttribute("huid");
    if (!item4update_huid.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // check, what item exist?
    item = user->getItem(item4update_huid);
    if (!item){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"updateItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4update_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"updateItemFailed.1\"";
        answer +=         ">";
        answer +=           "no such item";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // get "ipn_neighs" object
    neighs = server->getCreateNeighsObject();
    if (!neighs){
        err = nc->errorInternalError(skb);
        goto out;
    }

    // --------------------------- check on errors --------------------------80

    // search contacts for add (check errors)
    contacts4add = nc->getElementByTagName("contacts_add");
    if (contacts4add){
        res.clear();
        contacts4add->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            string              contact_id_str  = "";
            ContactObject       *contact        = NULL;
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
                    " id=\""    + contact_id_str +"\""
                    " "         + err_string
                    + "/>\n";

            }
        }
    }

    // search contacts for del (check errors)
    contacts4del = nc->getElementByTagName("contacts_del");
    if (contacts4del){
        contacts4del->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            string              contact_id_str  = "";
            ContactObject       *contact        = NULL;
            string              err_string      = "";
            contact4del                         = *res_it;

            // get contact ID for del
            contact_id_str = contact4del->getAttribute("id");

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
                    " id=\""    + contact_id_str +"\""
                    " "         + err_string
                    + "/>";
            }
        }
    }

    if (contacts_errors.size()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"updateItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4update_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"updateItemFailed.2\"";
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

    // ------------------------------ all ok --------------------------------80

    // update item properties if exist
    element = item4update->getElementByTagName("nickname");
    if (element){
        string nickname = element->getData();
        item->setAPINickName(nickname);
    }
    element = item4update->getElementByTagName("icon");
    if (element){
        string icon = element->getData();
        item->setAPIIcon(icon);
    }

    // add item to requested contacts
    contacts4add = nc->getElementByTagName("contacts_add");
    if (contacts4add){
        contacts4add->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            string              contact_id_str  = "";
            ContactObject       *contact        = NULL;
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
                contact->getCreateContactItem(item4update_huid);
            }
        }
    }

    // remove item from requested contacts
    contacts4del = nc->getElementByTagName("contacts_del");
    if (contacts4del){
        contacts4del->getElementsByTagName("contact", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            ContactObject::Id   contact_id;
            string              contact_id_str  = "";
            ContactObject       *contact        = NULL;
            contact4del                         = *res_it;

            // get contact ID for del
            contact_id_str = contact4del->getAttribute("id");

            // check what contact exist
            if (contact_id_str.size()){
                contact_id = (ContactObject::Id)(
                    atoi(contact_id_str.c_str())
                );
                contact = user->getContact(contact_id);
            }
            if (contact){
                contact->delContactItem(item4update_huid);
            }
        }
    }

    // add neighs if exist
    links4add = nc->getElementByTagName("links");
    if (links4add){
        links4add->getElementsByTagName("link", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            string dev;
            string lladdr;

            link4add = *res_it;
            dev     = link4add->getAttribute("name") + "0";
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
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"updateItemSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " huid=\"" + item4update_huid + "\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"updateItemSuccess.1\"";
    answer +=         ">";
    answer +=           "item was updated successfull";
    answer +=        "</description>";
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer to current client
    session->queue(answer);

    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
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

