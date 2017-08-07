#include "contactList/group.hpp"
#include "addContact/intro.hpp"
#include "addContact/process.hpp"

#include "addContact.hpp"

WizardAddContact::WizardAddContact(
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
        Wizard::Page_WizardAddContactPageIntro,
        new WizardAddContactPageIntro(m_api, m_contact_path)
    );
    setPage(
        Wizard::Page_WizardAddContactPageProcess,
        new WizardAddContactPageProcess(m_api, m_contact_path)
    );

    setWindowTitle(QString(
        tr("Adding new contact in") + ": '" + group_path + "'"
    ));
}

WizardAddContact::~WizardAddContact()
{
}

void WizardAddContact::accept()
{
    ApiCommandArgs args;

    QMap<QString, QString>              contact_info;
    QMap<QString, QString>              contact_links;
    QMap<QString, QString>::iterator    it;
    WizardAddContactPageProcess         *process = NULL;
    ContactListGroup                    *group = NULL;

    process = (WizardAddContactPageProcess *)
        page(Wizard::Page_WizardAddContactPageProcess);

    process->getContactInfo(contact_info);
    process->getContactLinks(contact_links);

    // request add to group
    group = ContactListTreeItem::getContactGroup(
        m_contact_path
    );
    args["group_" + group->getId()] = "add";

    // request add contact's info
    PWARN("contact_info:\n");
    for (it = contact_info.begin();
        it != contact_info.end();
        it++)
    {
        PWARN("    %s: %s\n",
            it.key().toStdString().c_str(),
            it.value().toStdString().c_str()
        );
        args["info_" + it.key()] = it.value();
    }

    // process command
    m_api->processCommand("addContact", args);

    // close dialog
    close();
}

