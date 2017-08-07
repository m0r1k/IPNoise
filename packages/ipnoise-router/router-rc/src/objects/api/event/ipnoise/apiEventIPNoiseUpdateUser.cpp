/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netCommandObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventIPNoiseUpdateUser.hpp"

ApiEventIPNoiseUpdateUser::ApiEventIPNoiseUpdateUser()
    :   ApiEventIPNoise(API_EVENT_IPNOISE_UPDATE_USER)
{
}

ApiEventIPNoiseUpdateUser::~ApiEventIPNoiseUpdateUser()
{
}

int ApiEventIPNoiseUpdateUser::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int             err                 = -1;
    string          sessid              = "";
    SessionObject   *session            = NULL;
    ItemObject      *item               = NULL;
    DomElement      *user_el            = NULL;
    DomElement      *status_el          = NULL;
    DomElement      *nickname_el        = NULL;
    DomElement      *icon_el            = NULL;
    DomElement      *real_nickname_el   = NULL;
    DomElement      *real_icon_el       = NULL;

    // get session
    session = nc->getSession(skb);
    if (not session){
        goto out;
    }

    // get remote user
    item = session->getItem();

    user_el = nc->getElementByTagName("user");
    if (not user_el){
        goto out;
    }

    status_el = user_el->getElementByTagName("status");
    if (status_el){
        string val = status_el->getData();
        item->setAPIStatus(val);
    }

    nickname_el = user_el->getElementByTagName("nickname");
    if (nickname_el){
        string val = nickname_el->getData();
        item->setAPINickName(val);
    }

    icon_el = user_el->getElementByTagName("icon");
    if (icon_el){
        string val = icon_el->getData();
        item->setAPIIcon(val);
    }

    real_nickname_el = user_el->getElementByTagName("real_nickname");
    if (real_nickname_el){
        string val = real_nickname_el->getData();
        item->setAPIRealNickName(val);
    }

    real_icon_el = user_el->getElementByTagName("real_icon");
    if (real_icon_el){
        string val = real_icon_el->getData();
        item->setAPIRealIcon(val);
    }

    // all ok
    err = 0;

out:
    return err;
}

