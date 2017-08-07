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
#include "apiCommandClientAddContact.hpp"

ApiCommandClientAddContact::ApiCommandClientAddContact()
    :   ApiCommandClient(API_COMMAND_CLIENT_ADD_CONTACT)
{
}

ApiCommandClientAddContact::~ApiCommandClientAddContact()
{
}

int ApiCommandClientAddContact::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject          *user               = NULL;
    ContactObject       *contact            = NULL;
    DomElement          *element            = NULL;
    DomElement          *contact4add        = NULL;
    DomElement          *groups4add         = NULL;
    DomElement          *items4add          = NULL;
    DomElement          *item4add           = NULL;
    DomElement          *group4add          = NULL;
    SessionObject       *session            = NULL;
    string              sessid              = "";
    string              contact4add_id_str  = "";
    string              answer              = "";
    string              groups_errors       = "";

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

    // search "contact" for add
    contact4add = nc->getElementByTagName("contact");
    if (!contact4add){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // search groups for add (check errors)
    groups4add = contact4add->getElementByTagName("groups");
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

    if (groups_errors.size()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"addContactFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " contact_id=\""
                            + contact4add_id_str + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"addItemFailed.1\"";
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

    // ok, looks like good, now add contact
    contact = user->getCreateContact();

    // add contact to requested groups
    groups4add = contact4add->getElementByTagName("groups");
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
                group->addGroupItem(contact->getContactId());
            }
        }
    }

    // add contact properties if exist
    element = contact4add->getElementByTagName("nickname");
    if (element){
        string nickname = element->getData();
        contact->setAPINickName(nickname);
    }
    element = contact4add->getElementByTagName("icon");
    if (element){
        string icon = element->getData();
        contact->setAPIIcon(icon);
    }

    // add items if exist
    items4add = contact4add->getElementByTagName("items");
    if (items4add){
        res.clear();
        items4add->getElementsByTagName("item", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            string huid;
            item4add = *res_it;
            huid = item4add->getAttribute("huid");

            // TODO report about errors here
            // (empty or incorrect huid)
            // 20120904 morik@

            if (not huid.size()){
                PERROR("Attempt to add contact with item"
                    " with empty huid\n");
                continue;
            }

            contact->getCreateContactItem(huid);
        }
    }

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"addContactSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " huid=\"" + contact4add_id_str + "\"";
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

