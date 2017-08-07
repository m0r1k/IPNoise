#include "process.hpp"

WizardAddGroupPageProcess::WizardAddGroupPageProcess(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardAddGroupPageProcess::~WizardAddGroupPageProcess()
{
}

QString WizardAddGroupPageProcess::getGroupName()
{
    return m_group_name_edit->text();
}

void WizardAddGroupPageProcess::setupUi(QWidget *a_parent)
{
    QString group_path;
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_addGroup_process.png"
    );

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(tr("Add group"));
    setSubTitle(tr("Please specify group name"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox  = new QVBoxLayout;
    m_form  = new QFormLayout;

    // group name
    m_group_name_label = new QLabel(tr("Group name"));
    m_form->setWidget(
        0,
        QFormLayout::LabelRole,
        m_group_name_label
    );
    m_group_name_edit = new QLineEdit;
    m_form->setWidget(
        0,
        QFormLayout::FieldRole,
        m_group_name_edit
    );

    m_vbox->addItem(m_form);
    setLayout(m_vbox);
}

