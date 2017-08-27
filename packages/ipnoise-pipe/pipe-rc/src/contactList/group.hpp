#ifndef CONTACT_LIST_GROUP
#define CONTACT_LIST_GROUP

#include <QtCore/QStringList>
#include <QtCore/QString>

#include <QtWidgets/QTreeWidgetItem>

#include "contactListTreeItem.hpp"
#include "api.hpp"

class ContactListGroup
    :   public ContactListTreeItem
{
    public:
        ContactListGroup(
            Api                 *a_api,
            const QString       &a_id,
            const QString       &a_name,
            bool                a_is_system,
            QTreeWidgetItem     *a_parent
        );
        virtual ~ContactListGroup();

        QString     getId();
        bool        isSystem();

    private:
        QString     m_id;
        bool        m_is_system;
};

#endif

