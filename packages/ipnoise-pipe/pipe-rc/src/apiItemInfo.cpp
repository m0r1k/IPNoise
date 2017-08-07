#include "apiItemInfo.hpp"

ApiItemInfo::ApiItemInfo(
    const QString &a_huid)
{
    m_huid          = a_huid;
    m_is_online     = false;
}

ApiItemInfo::~ApiItemInfo()
{
}

QString ApiItemInfo::getHuid()
{
    return m_huid;
}

QString ApiItemInfo::getNickName() const
{
    return m_nickname;
}

void ApiItemInfo::setNickName(const QString &a_nickname)
{
    if (m_nickname != a_nickname){
        m_nickname = a_nickname;
        emit changed(this);
    }
}

void ApiItemInfo::setOnline(
    bool a_is_online)
{
    if (m_is_online != a_is_online){
        m_is_online = a_is_online;
        emit changed(this);
    }
}

bool ApiItemInfo::isOnline() const
{
    return m_is_online;
}

ApiItemInfo & ApiItemInfo::operator=(
    const ApiItemInfo &a_right)
{
    m_nickname      = a_right.getNickName();
    m_is_online     = a_right.isOnline();

    return *this;
}

