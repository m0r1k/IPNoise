/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientGetItem.hpp"

ApiCommandClientGetItem::ApiCommandClientGetItem()
    :   ApiCommandClient(API_COMMAND_CLIENT_GETITEM)
{
}

ApiCommandClientGetItem::~ApiCommandClientGetItem()
{
}

int ApiCommandClientGetItem::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user           = NULL;
    ItemObject      *item           = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = "";
    string          item4info_huid  = "";
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

    // get item's huid (item for info)
    item4info_huid = nc->getAttribute("huid");
    if (!item4info_huid.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // check, what item exist?
    item = user->getItem(item4info_huid);
    if (!item){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"getItemFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " huid=\"" + item4info_huid + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"getItemFailed.1\"";
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

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"getItemSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " huid=\"" + item4info_huid + "\"";
    answer +=       ">";
    answer +=         "<item";
    answer +=           " huid=\""   + item4info_huid       + "\"";
    answer +=           " status=\"" + item->getAPIStatus() + "\"";
    answer +=         ">";
    answer +=           "<nickname>";
    answer +=             item->getAPINickName();
    answer +=           "</nickname>";
    answer +=           "<real_nickname>";
    answer +=             item->getAPIRealNickName();
    answer +=           "</real_nickname>";
    answer +=           "<icon>";
    answer +=             item->getAPIIcon();
    answer +=           "</icon>";
    answer +=           "<real_icon>";
    answer +=             item->getAPIRealIcon();
    answer +=           "</real_icon>";
    answer +=         "</item>";
    answer +=         "<description";
    answer +=           " msg_id=\"getItemSuccess.1\"";
    answer +=         ">";
    answer +=           "success";
    answer +=         "</description>";
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer only to current client
    session->queue(answer);

out:
    return err;
}

