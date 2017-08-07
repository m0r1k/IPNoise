#include "contactList/group.hpp"
#include "addGroup/process.hpp"

#include "addGroup.hpp"

WizardAddGroup::WizardAddGroup(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   Wizard(a_parent)
{
    QString group_path;

    m_api           = a_api;
    m_contact_path  = a_contact_path;

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setPage(
        Wizard::Page_WizardAddGroupPageProcess,
        new WizardAddGroupPageProcess(m_api, m_contact_path)
    );

    setWindowTitle(QString(
        tr("Adding new group in") + ": '" + group_path + "'"
    ));
}

WizardAddGroup::~WizardAddGroup()
{
}

void WizardAddGroup::accept()
{
    ApiCommandArgs              args;
    WizardAddGroupPageProcess   *process        = NULL;
    QString                     parent_group_id = "-1";
    ContactListGroup            *group          = NULL;

    process = (WizardAddGroupPageProcess *)
        page(Wizard::Page_WizardAddGroupPageProcess);

    group = ContactListTreeItem::getContactGroup(
        m_contact_path
    );

    // prepare command
    args["name"]    = process->getGroupName();
    args["parent"]  = group->getId();
    m_api->processCommand("addGroup", args);

    // close dialog
    close();
}

