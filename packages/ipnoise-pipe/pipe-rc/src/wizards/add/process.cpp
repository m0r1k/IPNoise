#include "process.hpp"

WizardAddPageProcess::WizardAddPageProcess(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardAddPageProcess::~WizardAddPageProcess()
{
}

void WizardAddPageProcess::setupUi(QWidget *a_parent)
{
    QString group_path;
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_add_process.png"
    );

    setTitle(tr("Select install dir"));
    setSubTitle(tr("Please select install dir"));

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(tr("Add.."));
    setSubTitle(tr("Please specify item information"));
    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );
}

