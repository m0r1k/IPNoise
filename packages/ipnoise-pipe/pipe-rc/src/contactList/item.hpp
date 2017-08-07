#ifndef CONTACT_LIST_ITEM
#define CONTACT_LIST_ITEM

#include <QTreeWidgetItem>
#include <QString>
#include <QStringList>

#include "contactListTreeItem.hpp"
#include "api.hpp"

class ContactListItem
    :   public ContactListTreeItem
{
    public:
        ContactListItem(
            int32_t             a_contact_id,
            Api                 *a_api,
            QTreeWidgetItem     *a_parent,
            const QStringList   &a_list
        );
        virtual ~ContactListItem();

        int32_t getContactId();
        QString getContactIdStr();

    private:
        int32_t m_contact_id;
};

#endif

