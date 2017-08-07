/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "hmac-md5/gen-huid.h"
#include "libxml2/domDocument.h"

#include "apiCommandClientLogin.hpp"

ApiCommandClientLogin::ApiCommandClientLogin()
    :   ApiCommandClient(API_COMMAND_CLIENT_LOGIN)
{
}

ApiCommandClientLogin::~ApiCommandClientLogin()
{
}

int ApiCommandClientLogin::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    SessionObject           *session        = NULL;
    IPNoiseObject           *ipnoise        = NULL;
    ServerObject            *server         = NULL;
    UsersObject             *users          = NULL;
    UserObject              *user           = NULL;
    SessionsObject          *user_sessions  = NULL;
    SessionObject           *user_session   = NULL;

    string  src_sessid  = "";
    string  dst_sessid  = "";
    string  answer      = "";
    string  login       = "";
    string  password    = "";
    string  reg         = "";
    string  huid        = "";

    // search session in "unknown" sessions list
    session = nc->getSession(skb);
    if (not session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session IDs
    src_sessid = session->getSrcSessId();
    dst_sessid = session->getDstSessId();

    // get ipnoise
    ipnoise = (IPNoiseObject *)nc->getDocument()->getRootElement();

    // get server
    server = ipnoise->getServerObject();

    // get users
    users = server->getCreateUsersObject();

    // get login, password and register flag
    login       = nc->getAttribute("login");
    password    = nc->getAttribute("password");
    reg         = nc->getAttribute("register");

    // generate huid
    huid = genHuid(login, password);
    user = users->getUserByHuid(huid);

    if (!user && reg == "true"){
        // request register new user, create it
        user = users->createUser(login, password);
    }

    if (!user){
        // there are no such user
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer += ">";
        answer +=     "<events>";
        answer +=       "<event";
        answer +=         " type=\"loginFailed\"";
        answer +=         " ver=\"0.01\"";
        answer +=         " login=\"" + login + "\"";
        answer +=         " huid=\""  + huid  + "\"";
        answer +=       ">";
        answer +=         "<description";
        answer +=           " msg_id=\"loginFailed.1\"";
        answer +=         ">";
        answer +=           "access denied";
        answer +=         "</description>";
        answer +=       "</event>";
        answer +=     "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // move session from "unknown" to user
    {
        // get user sessions
        user_sessions = user->getCreateSessionsObject();

        // check maybe session already exist?
        user_session = user_sessions->getSession(
            dst_sessid
        );
        if (!user_session){
            user_session = user_sessions->createSession(
                "ipn_session_client",
                src_sessid
            );
        }

        // setup session
        user_session->setApi(session->getApi());
        user_session->setType(session->getType());
        user_session->setState("logged");
        user_session->setSrcSessId(session->getSrcSessId());
        user_session->setDstSessId(session->getDstSessId());
        user_session->setSeq(session->getSeq());
        user_session->setAck(session->getAck());

        // delete session from "unknown"
        DomDocument::deleteElement(session);

        // now session in "ipn_user" space
        session = user_session;
    }

    // all ok
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=     "<events>";
    answer +=       "<event";
    answer +=         " type=\"loginSuccess\"";
    answer +=         " ver=\"0.01\"";
    answer +=         " login=\"" + login + "\"";
    answer +=         " huid=\""  + huid  + "\"";
    answer +=       ">";
    answer +=         "<description";
    answer +=           " msg_id=\"loginSuccess.1\"";
    answer +=         ">";
    answer +=           "login success";
    answer +=         "</description>";
    answer +=       "</event>";
    answer +=       "<event";
    answer +=         " type=\"updateContactList\"";
    answer +=         " ver=\"0.01\"";
    answer +=       ">";
    user->getContactList(answer);
    answer +=       "</event>";
    answer +=     "</events>";
    answer += "</ipnoise>";

    // send answer only to current client
    session->queue(answer);

    user->getCreateSelfItem()->setOnline(true);

out:
    return err;
}

