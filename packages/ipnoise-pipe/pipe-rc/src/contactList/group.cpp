#include "group.hpp"

ContactListGroup::ContactListGroup(
    Api                 *a_api,
    const QString       &a_id,
    const QString       &a_name,
    bool                a_is_system,
    QTreeWidgetItem     *a_parent)
    :   ContactListTreeItem(
            "group",
            a_api,
            a_parent,
            QStringList(a_name)
        )
{
    m_id        = a_id;
    m_is_system = a_is_system;
}

ContactListGroup::~ContactListGroup()
{
}

QString ContactListGroup::getId()
{
    return m_id;
}

bool ContactListGroup::isSystem()
{
    return m_is_system;
}

