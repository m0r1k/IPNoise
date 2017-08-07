class ContactListTreeItem;

#ifndef CONTACT_LIST_TREE_ITEM
#define CONTACT_LIST_TREE_ITEM

#include <QObject>
#include <QString>
#include <QTreeWidgetItem>
#include <QStringList>
#include "api.hpp"

typedef QList<ContactListTreeItem *>    ContactListPath;
typedef ContactListPath::iterator       ContactListPathIt;
typedef ContactListPath::const_iterator ContactListPathConstIt;

class ContactListItem;
class ContactListGroup;

class ContactListTreeItem
    :   public QTreeWidgetItem
{
    public:
        ContactListTreeItem(
            const QString       &a_type,
            Api                 *a_api,
            QTreeWidgetItem     *a_parent,
            const QStringList   &a_list
        );
        virtual ~ContactListTreeItem();

        QString             getType();
        virtual QString     getName();

        static QString getContactGroupsPath(
            ContactListPath &
        );
        static ContactListGroup * getContactGroup(
            ContactListPath &
        );
        static ContactListItem * getContactItem(
            ContactListPath &
        );
        static ContactListGroup * getContactParentGroup(
            ContactListPath &
        );
        static QString getContactName(
            ContactListPath &
        );

    private:
        Api         *m_api;
        QString     m_type;
};

#endif

