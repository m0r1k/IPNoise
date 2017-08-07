#include "contactList/item.hpp"
#include "contactList/group.hpp"

#include "searchUser/intro.hpp"
#include "searchUser/select.hpp"
#include "searchUser/fillTcpV4.hpp"
#include "searchUser/fillUdpV4.hpp"
#include "searchUser/process.hpp"

#include "searchUser.hpp"

WizardSearchUser::WizardSearchUser(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   Wizard(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setPage(
        Wizard::Page_WizardSearchUserPageIntro,
        new WizardSearchUserPageIntro(m_api, m_contact_path)
    );
    setPage(
        Wizard::Page_WizardSearchUserPageSelect,
        new WizardSearchUserPageSelect(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardSearchUserPageFillTcpV4,
        new WizardSearchUserPageFillTcpV4(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardSearchUserPageFillUdpV4,
        new WizardSearchUserPageFillUdpV4(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardSearchUserPageProcess,
        new WizardSearchUserPageProcess(
            m_api,
            m_contact_path,
            this
        )
    );

    setupUi(this);
}

WizardSearchUser::~WizardSearchUser()
{
}

LinkWidget * WizardSearchUser::getLink() const
{
    WizardSearchUserPageSelect *select = NULL;

    select = (WizardSearchUserPageSelect *)
        page(Wizard::Page_WizardSearchUserPageSelect);

    return select->getLink();
}

QString WizardSearchUser::getHuid() const
{
    WizardSearchUserPageProcess *process = NULL;

    process = (WizardSearchUserPageProcess *)
        page(Wizard::Page_WizardSearchUserPageProcess);

    return process->getHuid();
}

void WizardSearchUser::setupUi(QWidget *)
{
    QString contact_name;

    contact_name = ContactListTreeItem::getContactName(
        m_contact_path
    );

    setWindowTitle(QString(
        tr("Search user") + ": '" + contact_name + "'"
    ));
}

void WizardSearchUser::accept()
{
    ApiCommandArgs  args;
    QString         new_huid;
    ContactListItem *item = NULL;

    // close dialog
    close();

    new_huid = getHuid();
    if (not new_huid.size()){
        goto out;
    }

    item = ContactListTreeItem::getContactItem(
        m_contact_path
    );
    if (not item){
        goto out;
    }

    args["id"]          = item->getContactIdStr();
    args["item_add"]    = new_huid;
    m_api->processCommand("updateContact", args);

out:
    return;
}

