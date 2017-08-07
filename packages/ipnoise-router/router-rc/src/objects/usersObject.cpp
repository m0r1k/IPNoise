/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/serverObject.h"

#include "hmac-md5/gen-huid.h"
#include <ipnoise-common/system.h>
#include <ipnoise-common/log.h>
#include "objects/ipnoiseObject.h"

#include "objects/usersObject.h"

extern IPNoiseObject *ipnoise;

UsersObject * UsersObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new UsersObject(a_node, a_doc, a_tagname);
}

UsersObject::UsersObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

UsersObject::~UsersObject()
{
};

void UsersObject::registrySignals()
{
    getDocument()->registrySignal(
        "new_user_was_created",
        "ipn_user"
    );
}

string UsersObject::getAnyUserHuid()
{
    string huid;
    UserObject * user = NULL;

    user = getAnyUser();
    if (user){
        huid = user->getHuid();
    }

    return huid;
}

UserObject * UsersObject::getAnyUser()
{
    UserObject * user = NULL;
    vector <DomElement *> res;

    getElementsByTagName("ipn_user", res);
    if (res.size()){
        int id = int((double(rand())/RAND_MAX)*(res.size() - 1));
        user = (UserObject *)res[id];
    }
    return user;
}

void UsersObject::getUsers(vector<UserObject *> &a_users_out)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    getElementsByTagName("ipn_user", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        a_users_out.push_back((UserObject *)(*res_it));
    }
}

UserObject * UsersObject::getUserByHuid(
    const string    &a_huid)
{
    char buffer[1024] = { 0x00 };
    UserObject * user = NULL;

    snprintf(buffer, sizeof(buffer),
        "//ipn_user[@huid=\"%s\"]",
        a_huid.c_str()
    );
    user = (UserObject *)getElementByXpath(buffer);
    return user;
}

UserObject * UsersObject::getUserByLoginPassword(
    const string    &a_login,
    const string    &a_password)
{
    UserObject  *user   = NULL;
    string      huid    = "";

    huid = genHuid(a_login, a_password);
    user = getUserByHuid(huid);

    return user;
}

UserObject * UsersObject::createUserByHuid(
    const string    &a_huid,
    const string    &a_real_nickname)
{
    UserObject *user = NULL;

    user = UsersObject::getUserByHuid(a_huid);
    if (user){
        // already exist
        goto out;
    }

    // create user
    user = getDocument()->createElement<UserObject>("ipn_user");
    user->setHuid(a_huid);

    // store user
    appendChild(user);

    // setup user self name (after user added in DOM)
    user->getCreateSelfContact()->setAPIRealNickName(
        a_real_nickname
    );

out:
    return user;
}

UserObject * UsersObject::createUser(
    const string    &a_login,
    const string    &a_password)
{
    UserObject  *user       = NULL;
    string      huid        = "";

    user = UsersObject::getUserByLoginPassword(
        a_login,
        a_password
    );
    if (user){
        // already exist
        goto out;
    }

    // calculate huid
    huid = genHuid(a_login, a_password);

    // create user
    user = getDocument()->createElement<UserObject>("ipn_user");
    user->setHuid(huid);

    // store user
    appendChild(user);

    // setup user self name (after user added in DOM)
    user->getCreateSelfContact()->setAPIRealNickName(a_login);

out:
    return user;
}

