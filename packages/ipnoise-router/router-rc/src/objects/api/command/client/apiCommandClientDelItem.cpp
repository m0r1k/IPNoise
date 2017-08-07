/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientDelItem.hpp"

ApiCommandClientDelItem::ApiCommandClientDelItem()
    :   ApiCommandClient(API_COMMAND_CLIENT_DELITEM)
{
}

ApiCommandClientDelItem::~ApiCommandClientDelItem()
{
}

int ApiCommandClientDelItem::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user           = NULL;
    ItemObject      *item           = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = "";
    string          item4del_huid   = "";
    string          answer          = "";

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

    // get item's huid (item for del)
    item4del_huid = nc->getAttribute("huid");
    if (!item4del_huid.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // check, what item exist?
    item = user->getItem(item4del_huid);
    if (!item){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"delItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4del_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"delItemFailed.1\"";
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

    user->delItem(item4del_huid);

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"delItemSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " huid=\"" + item4del_huid + "\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"delItemSuccess.1\"";
    answer +=         ">";
    answer +=           "item was deleted successful";
    answer +=         "</description>";
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer only to current client
    session->queue(answer);

out:
    return err;
}

