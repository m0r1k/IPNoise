#include "item.hpp"

ContactListItem::ContactListItem(
    int32_t             a_contact_id,
    Api                 *a_api,
    QTreeWidgetItem     *a_parent,
    const QStringList   &a_list)
    :   ContactListTreeItem(
            "item",
            a_api,
            a_parent,
            a_list
        )
{
    m_contact_id = a_contact_id;
}

ContactListItem::~ContactListItem()
{
}

int32_t ContactListItem::getContactId()
{
    return m_contact_id;
}

QString ContactListItem::getContactIdStr()
{
    char buffer[128] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%d",
        m_contact_id
    );
    return buffer;
}

