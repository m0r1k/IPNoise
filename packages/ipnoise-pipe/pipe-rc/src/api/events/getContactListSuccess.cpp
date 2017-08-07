#include "api.hpp"
#include "getContactListSuccess.hpp"

ApiEventGetContactListSuccess::ApiEventGetContactListSuccess(
    Api *a_api)
    :   ApiEvent(a_api, "getContactListSuccess")
{
}

ApiEventGetContactListSuccess::~ApiEventGetContactListSuccess()
{
}

void ApiEventGetContactListSuccess::process()
{
    QList<QDomElement>              res;
    QList<QDomElement>::iterator    res_it;
    QDomElement                     items_el;

    items_el = m_api->getElementByTagName(
        m_element,
        "items"
    );
    if (items_el.isNull()){
        PERROR("'items' element was not found"
            " in 'getContactListSuccess' event\n");
        goto out;
    }

    m_api->getElementsByTagName(
        items_el,
        "item",
        res
    );

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ApiItemInfo *info       = NULL;
        QDomElement item_el     = *res_it;
        QString     huid;
        QString     is_ipnoise;
        QString     is_online;

        huid        = item_el.attribute("huid");
        is_ipnoise  = item_el.attribute("is_ipnoise");
        is_online   = item_el.attribute("is_online");

        if (!huid.size()){
            PERROR("Impty huid in"
                " ApiEventGetContactListSuccess::process\n");
            continue;
        }
        info = m_api->getCreateItemInfo(huid);
        info->setOnline("true" == is_online);

        // nick name
        {
            QDomElement nickname;
            nickname = m_api->getElementByTagName(
                item_el,
                "nickname"
            );
            if (not nickname.isNull()){
                info->setNickName(nickname.text());
            }
        }
    }

out:
    return;
}

