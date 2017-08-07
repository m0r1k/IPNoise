#include "contactList/group.hpp"
#include "process.hpp"

WizardDelGroupPageProcess::WizardDelGroupPageProcess(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardDelGroupPageProcess::~WizardDelGroupPageProcess()
{
}

QString WizardDelGroupPageProcess::getDelStrategy()
{
    QString ret = "0";
    if (m_strategy_1->isChecked()){
        ret = "1";
    }
    return ret;
}

void WizardDelGroupPageProcess::setupUi(QWidget *a_parent)
{
    QString group_path;
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_delGroup_process.png"
    );

    setTitle(tr("Select install dir"));
    setSubTitle(tr("Please select install dir"));

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(
        tr("Deleting group")
        + ": <b>'"
        + ContactListTreeItem::getContactGroupsPath(
            m_contact_path)
        + "'</b>"
    );
    setSubTitle(tr("Please specify delete strategy"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox = new QVBoxLayout;
    // strategy 0
    m_strategy_0 = new QRadioButton(tr(
        "Remove this group and child groups,"
        " keep contacts (available via 'All items' group)"
    ));
    m_strategy_0->setChecked(true);
    m_vbox->addWidget(m_strategy_0);

    // strategy 1
    m_strategy_1 = new QRadioButton(tr(
        "Remove this group, child groups and contacts"
    ));
    m_vbox->addWidget(m_strategy_1);

    setLayout(m_vbox);
}

