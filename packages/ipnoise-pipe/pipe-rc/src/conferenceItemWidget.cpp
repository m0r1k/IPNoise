#include "conferenceItemWidget.hpp"

ConferenceItemWidget::ConferenceItemWidget(
    Api             *a_api,
    QListWidget     *a_parent,
    const QString   &a_huid)
    :   QListWidgetItem(a_parent)
{
    m_api   = a_api;
    m_huid  = a_huid;

    ApiItemInfo *info = m_api->getItemInfo(a_huid);
    if (info){
        setText(info->getNickName());
    } else {
        PERROR("Cannot get user info for: '%s'\n",
            a_huid.toStdString().c_str()
        );
    }
}

ConferenceItemWidget::~ConferenceItemWidget()
{
}

QString ConferenceItemWidget::getHuid()
{
    return m_huid;
}

