/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "apiCommandClientUpdateContact.hpp"

ApiCommandClientUpdateContact::ApiCommandClientUpdateContact()
    :   ApiCommandClient(API_COMMAND_CLIENT_UPDATE_CONTACT)
{
}

ApiCommandClientUpdateContact::~ApiCommandClientUpdateContact()
{
}

int ApiCommandClientUpdateContact::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject          *user                   = NULL;
    ContactObject       *contact                = NULL;
    DomElement          *element                = NULL;
    DomElement          *contact4update         = NULL;
    DomElement          *groups4del             = NULL;
    DomElement          *items4add              = NULL;
    DomElement          *item4add               = NULL;
    DomElement          *groups4add             = NULL;
    DomElement          *group4add              = NULL;
    DomElement          *group4del              = NULL;
    SessionObject       *session                = NULL;
    string              sessid                  = "";
    string              contact4update_id_str   = "";
    string              answer                  = "";
    string              groups_errors           = "";
    ContactObject::Id   contact4update_id;

    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

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

    // search "contact" for update
    contact4update = nc->getElementByTagName("contact");
    if (!contact4update){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get contact's ID (contact for update)
    contact4update_id_str = contact4update->getAttribute(
        "id"
    );
    if (!contact4update_id_str.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }
    contact4update_id = (ContactObject::Id)(
        atoi(contact4update_id_str.c_str())
    );

    // check, what contact exist?
    contact = user->getContact(contact4update_id);
    if (!contact){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"updateContactFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " contact_id=\""
                            + contact4update_id_str + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"updateContactFailed.1\"";
        answer +=         ">";
        answer +=           "no such contact";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // --------------------------- check on errors --------------------------80

    // search groups for add (check errors)
    groups4add = contact4update->getElementByTagName("groups_add");
    if (groups4add){
        res.clear();
        groups4add->getElementsByTagName("group", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            GroupObject *group      = NULL;
            string      group_id    = "";
            string      err_string  = "";
            group4add               = *res_it;

            // get group ID for add
            group_id = group4add->getAttribute("id");

            // check what group exist
            group = user->getGroupById(group_id);
            if (!group){
                err_string += " not_found=\"1\"";
            } else {
                if (group->isSystemGroup()){
                    err_string += " is_system=\"1\"";
                }
            }
            if (err_string.size()){
                groups_errors += "<group id=\"" + group_id +"\""
                    " " + err_string + "/>\n";

            }
        }
    }

    // search groups for del (check errors)
    groups4del = contact4update->getElementByTagName("groups_del");
    if (groups4del){
        res.clear();
        groups4del->getElementsByTagName("group", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            GroupObject *group      = NULL;
            string      group_id    = "";
            string      err_string  = "";
            group4del               = *res_it;

            // get group ID for del
            group_id = group4del->getAttribute("id");

            // check what group exist
            group = user->getGroupById(group_id);
            if (!group){
                err_string += " not_found=\"1\"";
            } else {
                if (group->isSystemGroup()){
                    err_string += " is_system=\"1\"";
                }
            }
            if (err_string.size()){
                groups_errors += "<group id=\"" + group_id +"\""
                    " " + err_string + "/>\n";

            }
        }
    }

    if (groups_errors.size()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"updateContactFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " contact_id=\"" + contact4update_id_str + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"updateContactFailed.2\"";
        answer +=         ">";
        answer +=           "error(s) while work with group(s)";
        answer +=         "</description>";
        answer +=         "<groups>";
        answer +=           groups_errors;
        answer +=         "</groups>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // ------------------------------ all ok --------------------------------80

    // update contact properties if exist
    element = contact4update->getElementByTagName("nickname");
    if (element){
        string nickname = element->getData();
        contact->setAPINickName(nickname);
    }
    element = contact4update->getElementByTagName("icon");
    if (element){
        string icon = element->getData();
        contact->setAPIIcon(icon);
    }

    // add contact to requested groups
    groups4add = contact4update->getElementByTagName("groups_add");
    if (groups4add){
        res.clear();
        groups4add->getElementsByTagName("group", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            GroupObject *group      = NULL;
            string      group_id    = "";
            group4add               = *res_it;

            // get group ID for add
            group_id = group4add->getAttribute("id");

            // check what group exist
            group = user->getGroupById(group_id);
            if (group){
                group->addGroupItem(contact4update_id);
            }
        }
    }

    // remove contact from requested groups
    groups4del = contact4update->getElementByTagName("groups_del");
    if (groups4del){
        res.clear();
        groups4del->getElementsByTagName("group", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            GroupObject *group      = NULL;
            string      group_id    = "";
            group4del               = *res_it;

            // get group ID for del
            group_id = group4del->getAttribute("id");

            // check what group exist
            group = user->getGroupById(group_id);
            if (group){
                group->delGroupItem(contact4update_id);
            }
        }
    }

    // add/del items if exist
    items4add = contact4update->getElementByTagName("items");
    if (items4add){
        res.clear();
        items4add->getElementsByTagName("item", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            string huid;
            string action;

            item4add    = *res_it;
            huid        = item4add->getAttribute("huid");
            action      = item4add->getAttribute("action");

            // TODO report about errors here
            // (empty or incorrect huid)
            // 20120904 morik@

            if (not huid.size()){
                PERROR("Attempt to add/del item with empty huid"
                    " from contact with id '%d'\n",
                    contact4update_id
                );
                continue;
            }

            if ("add" == action){
                contact->getCreateContactItem(huid);
            } else if ("del" == action){
                contact->delContactItem(huid);
            } else {
                PERROR("Unsupported action: '%s' for update contact"
                    " with ID: '%d'\n",
                    action.c_str(),
                    contact4update_id
                );
            }
        }
    }

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"updateContactSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " contact_id=\"" + contact4update_id_str + "\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"updateContactSuccess.1\"";
    answer +=         ">";
    answer +=           "contact was updated successfull";
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

