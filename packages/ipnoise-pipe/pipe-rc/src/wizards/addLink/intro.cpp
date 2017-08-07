#include "intro.hpp"

WizardAddLinkPageIntro::WizardAddLinkPageIntro(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardAddLinkPageIntro::~WizardAddLinkPageIntro()
{
}

void WizardAddLinkPageIntro::setupUi(QWidget *a_parent)
{
    QString pixmap_str;
    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_addLink_intro.png"
    );

    setTitle(tr("Introduction"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox  = new QVBoxLayout(a_parent);
    m_label = new QLabel(QString(
        tr("Are you ready add new contact for user")
        + ": <b>"
        + ContactListTreeItem::getContactName(m_contact_path)
        + "</b>?<br/>"
        + tr("This wizard will help You to add"
            " new contact such as Phone, E-mail, ICQ, etc..")
    ));
    m_label->setWordWrap(true);

    m_vbox->addWidget(m_label);
    setLayout(m_vbox);
}

