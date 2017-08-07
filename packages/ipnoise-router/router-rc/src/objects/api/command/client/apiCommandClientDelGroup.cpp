/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientDelGroup.hpp"

ApiCommandClientDelGroup::ApiCommandClientDelGroup()
    :   ApiCommandClient(API_COMMAND_CLIENT_DELGROUP)
{
}

ApiCommandClientDelGroup::~ApiCommandClientDelGroup()
{
}

int ApiCommandClientDelGroup::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    GroupsObject::GroupDelStrategy  group4del_strategy;
    UserObject      *user               = NULL;
    DomElement      *group4del          = NULL;
    SessionObject   *session            = NULL;
    GroupObject     *group              = NULL;
    GroupsObject    *groups             = NULL;
    string          sessid              = "";
    string          group4del_id        = "";
    group4del_strategy                  = GroupsObject::GROUP_DEL_STRATEGY_1;
    string          answer              = "";
    string          tmp                 = "";

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

    // get groups object
    groups = user->getCreateGroupsObject();

    // search "group" for del
    group4del = nc->getElementByTagName("group");
    if (!group4del){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get group's name (group for add)
    group4del_id = group4del->getAttribute("id");
    if (!group4del_id.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get group's name (group for add)
    tmp = group4del->getAttribute("strategy");
    if (tmp.size()){
        group4del_strategy = (GroupsObject::GroupDelStrategy)atoi(tmp.c_str());
    }

    // search group
    group = user->getGroupById(group4del_id);
    if (!group){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"delGroupFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"delGroupFailed.1\"";
        answer +=         ">";
        answer +=           "group not exist";
        answer +=         "</description>";
        answer +=         "<group";
        answer +=           " id=\"" + group4del_id + "\"";
        answer +=         "/>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // check what strategy supported
    if (!groups->isStrategySupported(group4del_strategy)){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"delGroupFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"delGroupFailed.3\"";
        answer +=         ">";
        answer +=           "unsupported strategy";
        answer +=         "</description>";
        answer +=         "<group";
        answer +=           " id=\"" + group4del_id + "\"";
        answer +=         "/>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // disable delete system's groups
    if (group->isSystemGroup()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"delGroupFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"delGroupFailed.2\"";
        answer +=         ">";
        answer +=           "cannot delete system group";
        answer +=         "</description>";
        answer +=         "<group";
        answer +=           " id=\"" + group4del_id + "\"";
        answer +=         "/>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // disable delete root group
    if (group->isRootGroup()){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"delGroupFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"delGroupFailed.4\"";
        answer +=         ">";
        answer +=           "cannot delete root group";
        answer +=         "</description>";
        answer +=         "<group";
        answer +=           " id=\"" + group4del_id + "\"";
        answer +=         "/>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // delete group
    groups->delGroup(group->getGroupIdInt(), group4del_strategy);

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"delGroupSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    answer +=         "<description ";
    answer +=           " msg_id=\"delGroupSuccess.1\"";
    answer +=         ">";
    answer +=           "group was deleted successful";
    answer +=         "</description>";
    answer +=         "<group";
    answer +=           " id=\"" + group4del_id + "\"";
    answer +=         "/>";
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

