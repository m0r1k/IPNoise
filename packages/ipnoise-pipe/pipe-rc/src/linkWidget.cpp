#include "linkWidget.hpp"

LinkWidget::LinkWidget(
    const QString   &a_name,
    const QString   &a_display_name,
    const QIcon     &a_icon,
    QWidget         *a_parent)
    :   QDialog(a_parent)
{
    m_name          = a_name;
    m_display_name  = a_display_name;
    m_icon          = a_icon;
}

LinkWidget::~LinkWidget()
{
}

QString LinkWidget::getName()
{
    return m_name;
}

QString LinkWidget::getDisplayName()
{
    return m_display_name.size() ? m_display_name : m_name;
}

QIcon LinkWidget::getIcon()
{
    return m_icon;
}

void LinkWidget::setAddr(const QString &a_addr)
{
    m_addr = a_addr;
}

QString LinkWidget::getAddr()
{
    return m_addr;
}

