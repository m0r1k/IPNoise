#include "api.hpp"
#include "updateContact.hpp"

ApiEventUpdateContact::ApiEventUpdateContact(
    Api *a_api)
    :   ApiEvent(a_api, "updateContact")
{
}

ApiEventUpdateContact::~ApiEventUpdateContact()
{
}

void ApiEventUpdateContact::process()
{
    int32_t         contact_id;
    QString         contact_id_str;
    QString         is_online;
    ApiContactInfo  *info = NULL;
    QDomElement     contact_el;

    contact_el = m_api->getElementByTagName(m_element, "contact");
    if (contact_el.isNull()){
        PERROR("contact element was not found"
            " in updateContact event\n");
        goto out;
    }

    contact_id_str = contact_el.attribute("contact_id");
    if (!contact_id_str.size()){
        PERROR("Impty contact ID in"
            " ApiEventUpdateContact::process\n");
        goto out;
    }

    contact_id  = contact_id_str.toInt();
    is_online   = contact_el.attribute("is_online");

    info = m_api->getCreateContactInfo(contact_id);
    if (!info){
        PERROR("Cannot create contact info\n");
        goto fail;
    }

    // fill
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

out:
    return;
fail:
    goto out;
}

