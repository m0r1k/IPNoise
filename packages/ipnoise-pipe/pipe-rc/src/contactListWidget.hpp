class ContactListWidget;

#ifndef CONTACT_LIST_WIDGET_HPP
#define CONTACT_LIST_WIDGET_HPP

#include <QObject>
#include <QWidget>
#include <QList>
#include <QTreeWidget>
#include <QStringList>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QCloseEvent>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "contactList/group.hpp"
#include "contactList/item.hpp"
#include "api.hpp"
#include "conferencesWidget.hpp"
#include "wizards/add.hpp"
#include "wizards/addContact.hpp"
#include "wizards/addGroup.hpp"
#include "wizards/delGroup.hpp"
#include "wizards/searchUser.hpp"

class ContactListWidget
    :   public  QWidget
{
    Q_OBJECT

    public:
        ContactListWidget(
            Api         *a_api,
            QWidget     *a_parent
        );
        virtual ~ContactListWidget();

        void            setupUi(QWidget *a_parent);
        void            getCurPath(ContactListPath &);

        void getContactsById(
            int32_t                     &a_contact_id,
            QList<ContactListItem *>    &a_out
        );

        void getContactsById(
            int32_t                     &a_contact_id,
            QList<ContactListItem *>    &a_out,
            ContactListTreeItem         *a_item
        );

    protected:
        void createActions();
        void addGroup(
            QDomElement         &a_group_el,
            QTreeWidgetItem     *a_group = NULL
        );
        void itemDoubleClicked(ContactListItem *,     int);
        void itemDoubleClicked(ContactListGroup *,    int);

        virtual void contextMenuEvent(QContextMenuEvent *);

        void itemContactChanged(
            ApiContactInfo      *a_info,
            ContactListItem     *a_item
        );

    protected slots:
        void    itemContactChangedSlot(ApiContactInfo *);
        void    apiEvent(ApiEventUpdateContactList *);
        void    itemDoubleClicked(QTreeWidgetItem *, int);
        void    add();
        void    addContact();
        void    addGroup();
        void    delGroup();
        void    searchUser();

    private:
        Api                 *m_api;
        ConferencesWidget   *m_conferences;
        QVBoxLayout         *m_vbox;
        QTreeWidget         *m_treeWidget;
        QAction             *m_actAdd;
        QAction             *m_actAddContact;
        QAction             *m_actAddGroup;
        QAction             *m_actDelGroup;
        QAction             *m_actSearchUser;
};

#endif

