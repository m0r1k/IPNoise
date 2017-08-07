/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientAddGroup.hpp"

ApiCommandClientAddGroup::ApiCommandClientAddGroup()
    :   ApiCommandClient(API_COMMAND_CLIENT_ADDGROUP)
{
}

ApiCommandClientAddGroup::~ApiCommandClientAddGroup()
{
}

int ApiCommandClientAddGroup::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user               = NULL;
    DomElement      *element            = NULL;
    DomElement      *group4add          = NULL;
    SessionObject   *session            = NULL;
    GroupObject     *parent_group       = NULL;
    GroupObject     *new_group          = NULL;
    GroupsObject    *groups             = NULL;
    string          sessid              = "";
    string          group4add_name      = "";
    string          group4add_parent_id = "";
    string          answer              = "";
    int             parent_id           = -1;

    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

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

    // search "ipn_group" for add
    group4add = nc->getElementByTagName("group");
    if (!group4add){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get group's name (group for add)
    group4add_name = group4add->getAttribute("name");
    if (!group4add_name.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get group's parent ID (group for add)
    group4add_parent_id = group4add->getAttribute("parent");
    if (group4add_parent_id.size()){
        parent_id = atoi(group4add_parent_id.c_str());
    }

    // check parent group if need
    if (parent_id >= 0){
        parent_group = user->getGroupById(parent_id);
        if (!parent_group){
            answer  = "<ipnoise";
            answer +=   " ver=\"0.01\"";
            answer +=   ">";
            answer +=     "<events>";
            answer +=       "<event";
            answer +=         " type=\"addGroupFailed\"";
            answer +=         " ver=\"0.01\"";
            answer +=       ">";
            answer +=         "<description";
            answer +=           " msg_id=\"addGroupFailed.1\"";
            answer +=         ">";
            answer +=           "no such parent";
            answer +=         "</description>";
            answer +=       "</event>";
            answer +=     "</events>";
            answer += "</ipnoise>";

            // send answer only to current client
            session->queue(answer);
            goto out;
        }
    }

    // get GroupsObject
    groups = user->getCreateGroupsObject();

    // create new group
    new_group = groups->smartCreateGroup(
        GroupsObject::GROUP_TYPE_USER_DEFINED,
        parent_id
    );
    if (!new_group){
        err = nc->errorInternalError(skb);
        goto out;
    }

    // set new group name
    new_group->setGroupName(group4add_name);

    // check parent
    element = new_group->getParentNode();
    if (element
        && ("ipn_group" == element->getTagName()))
    {
        parent_group = (GroupObject *)element;
    }

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"addGroupSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"addGroupSuccess.1\"";
    answer +=         ">";
    answer +=           "group was added successful";
    answer +=         "</description>";
    answer +=         "<group";
    answer +=           " name=\""+ new_group->getGroupName() +"\"";
    answer +=           " id=\""  + new_group->getGroupId()   +"\"";
    answer +=           " parent_id=\"" + parent_group->getGroupId()
                            + "\"";
    answer +=         "/>";
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

