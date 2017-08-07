#include "contactList/group.hpp"
#include "delGroup/process.hpp"

#include "delGroup.hpp"

WizardDelGroup::WizardDelGroup(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   Wizard(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setPage(
        Wizard::Page_WizardDelGroupPageProcess,
        new WizardDelGroupPageProcess(m_api, m_contact_path)
    );

    setupUi(this);
}

WizardDelGroup::~WizardDelGroup()
{
}

void WizardDelGroup::setupUi(QWidget *)
{
    QString group_path;

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setWindowTitle(QString(
        tr("Deleting group") + ": '" + group_path + "'"
    ));
}

void WizardDelGroup::accept()
{
    ApiCommandArgs              args;
    WizardDelGroupPageProcess   *process        = NULL;
    QString                     parent_group_id = "-1";
    ContactListGroup            *group          = NULL;

    process = (WizardDelGroupPageProcess *)
        page(Wizard::Page_WizardDelGroupPageProcess);

    group = ContactListTreeItem::getContactGroup(
        m_contact_path
    );

    // prepare command
    args["strategy"]    = process->getDelStrategy();
    args["id"]          = group->getId();
    m_api->processCommand("delGroup", args);

    // close dialog
    close();
}

