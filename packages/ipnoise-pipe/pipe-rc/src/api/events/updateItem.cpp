#include "api.hpp"
#include "updateItem.hpp"

ApiEventUpdateItem::ApiEventUpdateItem(
    Api *a_api)
    :   ApiEvent(a_api, "updateItem")
{
}

ApiEventUpdateItem::~ApiEventUpdateItem()
{
}

void ApiEventUpdateItem::process()
{
    QString         huid;
    QString         huid_old;
    QString         is_online;
    ApiItemInfo     *info = NULL;
    QDomElement     item_el;

    item_el = m_api->getElementByTagName(m_element, "item");
    if (item_el.isNull()){
        PERROR("item element was not found"
            " in updateItem event\n");
        goto out;
    }

    huid        = item_el.attribute("huid");
    is_online   = item_el.attribute("is_online");

    if (!huid.size()){
        PERROR("Impty huid in"
            " ApiEventUpdateItem::process\n");
        goto out;
    }

    info = m_api->getCreateItemInfo(huid);
    if (!info){
        PERROR("Cannot create item info\n");
        goto fail;
    }

    // fill
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

out:
    return;
fail:
    goto out;
}

