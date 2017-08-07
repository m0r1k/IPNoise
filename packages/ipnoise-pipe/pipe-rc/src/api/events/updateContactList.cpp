#include "api.hpp"
#include "updateContactList.hpp"

ApiEventUpdateContactList::ApiEventUpdateContactList(
    Api *a_api)
    :   ApiEvent(a_api, "updateContactList")
{
}

ApiEventUpdateContactList::~ApiEventUpdateContactList()
{
}

void ApiEventUpdateContactList::process()
{
    QList<QDomElement>              contacts;
    QList<QDomElement>::iterator    contacts_it;
    QList<QDomElement>              items;
    QList<QDomElement>::iterator    items_it;
    QDomElement                     items_el;

    // contacts
    {
        items_el = m_api->getElementByTagName(
            m_element,
            "contacts"
        );
        if (items_el.isNull()){
            PERROR("'contacts' element was not found"
                " in 'updateContactList' event\n");
            goto out;
        }

        contacts.clear();
        m_api->getElementsByTagName(
            items_el,
            "contact",
            contacts
        );
        for (contacts_it = contacts.begin();
            contacts_it != contacts.end();
            contacts_it++)
        {
            QList<QDomElement>              contact_items;
            QList<QDomElement>::iterator    contact_items_it;
            QDomElement                     contact_items_el;

            ApiContactInfo  *info       = NULL;
            QDomElement     contact_el  = *contacts_it;
            QString         contact_id;
            QString         is_online;

            contact_id  = contact_el.attribute("contact_id");
            is_online   = contact_el.attribute("is_online");

            if (!contact_id.size()){
                PERROR("Impty huid in"
                    " ApiEventUpdateContactList::process\n");
                continue;
            }
            info = m_api->getCreateContactInfo(
                contact_id.toInt()
            );
            info->setOnline("true" == is_online);

            // nick name
            {
                QDomElement nickname;
                nickname = m_api->getElementByTagName(
                    contact_el,
                    "nickname"
                );
                if (not nickname.isNull()){
                    info->setNickName(nickname.text());
                }
            }

            // search contact items
            info->delContactItems();
            contact_items_el = m_api->getElementByTagName(
                contact_el,
                "contact_items"
            );
            if (not contact_items_el.isNull()){
                m_api->getElementsByTagName(
                    contact_items_el,
                    "contact_item",
                    contact_items
                );
            }
            for (contact_items_it = contact_items.begin();
                contact_items_it != contact_items.end();
                contact_items_it++)
            {
                QDomElement contact_item = *contact_items_it;
                QString huid = contact_item.attribute("huid");
                info->addContactItem(huid);
            }
        }
    }

    // items
    {
        items_el = m_api->getElementByTagName(
            m_element,
            "items"
        );
        if (items_el.isNull()){
            PERROR("'items' element was not found"
                " in 'updateContactList' event\n");
            goto out;
        }

        items.clear();
        m_api->getElementsByTagName(
            items_el,
            "item",
            items
        );
        for (items_it = items.begin();
            items_it != items.end();
            items_it++)
        {
            ApiItemInfo *info   = NULL;
            QDomElement item_el = *items_it;
            QString     huid;
            QString     is_ipnoise;
            QString     is_online;

            huid        = item_el.attribute("huid");
            is_ipnoise  = item_el.attribute("is_ipnoise");
            is_online   = item_el.attribute("is_online");

            if (!huid.size()){
                PERROR("Impty huid in"
                    " ApiEventUpdateContactList::process\n");
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
    }

out:
    return;
}

