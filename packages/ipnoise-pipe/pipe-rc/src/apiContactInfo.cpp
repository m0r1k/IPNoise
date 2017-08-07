#include "apiContactInfo.hpp"

ApiContactInfo::ApiContactInfo(
    int32_t a_contact_id)
{
    m_contact_id    = a_contact_id;
    m_is_online     = false;
}

ApiContactInfo::~ApiContactInfo()
{
}

int32_t ApiContactInfo::getContactId()
{
    return m_contact_id;
}

QString ApiContactInfo::getContactIdStr()
{
    char    buffer[128] = { 0x00 };
    int32_t contact_id  = getContactId();

    snprintf(buffer, sizeof(buffer),
        "%d",
        contact_id
    );

    return buffer;
}

QString ApiContactInfo::getNickName() const
{
    return m_nickname;
}

void ApiContactInfo::setNickName(const QString &a_nickname)
{
    if (m_nickname != a_nickname){
        m_nickname = a_nickname;
        emit changed(this);
    }
}

void ApiContactInfo::setOnline(
    bool a_is_online)
{
    if (m_is_online != a_is_online){
        m_is_online = a_is_online;
        emit changed(this);
    }
}

bool ApiContactInfo::isOnline() const
{
    return m_is_online;
}

void ApiContactInfo::getContactItems(
    ContactItems    &a_out)
{
    a_out = m_contact_items;
}

void ApiContactInfo::delContactItems()
{
    m_contact_items.clear();
}

void ApiContactInfo::addContactItem(
    const QString   &a_huid)
{
    if (not a_huid.size()){
        goto out;
    }

    m_contact_items[a_huid] = 1;

out:
    return;
}

ApiContactInfo & ApiContactInfo::operator=(
    const ApiContactInfo &a_right)
{
    m_nickname      = a_right.getNickName();
    m_is_online     = a_right.isOnline();

    return *this;
}

