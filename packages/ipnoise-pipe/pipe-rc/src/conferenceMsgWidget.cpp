#include "conferenceMsgWidget.hpp"

ConferenceMsgWidget::ConferenceMsgWidget(QListWidget *a_parent)
    :   QListWidgetItem(a_parent)
{
}

ConferenceMsgWidget::~ConferenceMsgWidget()
{
}

void ConferenceMsgWidget::setMsgId(
    const QString &a_msg_id)
{
    m_msg_id = a_msg_id;
}

QString ConferenceMsgWidget::getMsgId()
{
    return m_msg_id;
}

