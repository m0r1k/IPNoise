#include "select.hpp"

WizardAddPageSelect::WizardAddPageSelect(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardAddPageSelect::~WizardAddPageSelect()
{
}

void WizardAddPageSelect::setupUi(QWidget *a_parent)
{
    QString group_path;
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_add_select.png"
    );

    setTitle(tr("Select install dir"));
    setSubTitle(tr("Please select install dir"));

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(tr("What are would You like to add?"));
    setSubTitle(tr("Please specify item select for add"));
    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox        = new QVBoxLayout(a_parent);
    m_types       = new QGroupBox();
    m_types_vbox  = new QVBoxLayout;

    // new user
    m_type_new_user = new QRadioButton(tr(
        "New &user"
    ));
    m_type_new_user->setChecked(true);
    m_types_vbox->addWidget(m_type_new_user);

    // new group
    m_type_new_group = new QRadioButton(tr(
        "New &group"
    ));
    m_types_vbox->addWidget(m_type_new_group);

    // new notes
    m_type_new_notes = new QRadioButton(tr(
        "New &notes"
    ));
    m_types_vbox->addWidget(m_type_new_notes);

    // new file
    m_type_new_file = new QRadioButton(tr(
        "New &file"
    ));
    m_types_vbox->addWidget(m_type_new_file);

    m_types_vbox->setStretch(1, 1);
    m_types->setLayout(m_types_vbox);
    m_vbox->addWidget(m_types);

    setLayout(m_vbox);
}

int WizardAddPageSelect::nextId() const
{
    int ret = -1;
    if (m_type_new_user->isChecked()){
        ret = Wizard::Page_WizardAddContactPageIntro;
    }
    return ret;
}

