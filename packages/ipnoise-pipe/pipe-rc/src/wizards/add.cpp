#include "add/intro.hpp"
#include "add/select.hpp"
#include "add/process.hpp"
#include "addContact/intro.hpp"
#include "addContact/process.hpp"

#include "add.hpp"

WizardAdd::WizardAdd(
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

    setWindowTitle(QString(
        tr("Adding new item in") + ": '" + group_path + "'"
    ));

    setPage(
        Wizard::Page_WizardAddPageIntro,
        new WizardAddPageIntro(m_api, m_contact_path)
    );
    setPage(
        Wizard::Page_WizardAddPageSelect,
        new WizardAddPageSelect(m_api, m_contact_path)
    );
    setPage(
        Wizard::Page_WizardAddContactPageIntro,
        new WizardAddContactPageIntro(m_api, m_contact_path)
    );
    setPage(
        Wizard::Page_WizardAddContactPageProcess,
        new WizardAddContactPageProcess(m_api, m_contact_path)
    );
}

WizardAdd::~WizardAdd()
{
}

void WizardAdd::accept()
{
    close();
}

