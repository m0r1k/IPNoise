#include "intro.hpp"

WizardAddPageIntro::WizardAddPageIntro(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardAddPageIntro::~WizardAddPageIntro()
{
}

void WizardAddPageIntro::setupUi(QWidget *a_parent)
{
    QString group_path;
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_add_intro.png"
    );

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(tr("Introduction"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox  = new QVBoxLayout(a_parent);
    m_label = new QLabel(QString(
        tr("Are you ready to add new item in")
        + ": <b>" + group_path + "</b>?<br/>"
        + tr("This wizard will help You to add"
        " new User, Group, Notes, Music, Video, Document"
        " or any File in your contact list")
    ));
    m_label->setWordWrap(true);

    m_vbox->addWidget(m_label);
    setLayout(m_vbox);
}

