#include "intro.hpp"

WizardSearchUserPageIntro::WizardSearchUserPageIntro(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setupUi(this);
}

WizardSearchUserPageIntro::~WizardSearchUserPageIntro()
{
}

void WizardSearchUserPageIntro::setupUi(QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_searchUser_intro.png"
    );

    setTitle(tr("Introduction"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox  = new QVBoxLayout(a_parent);
    m_label = new QLabel(QString(
        tr("Are you ready start search link for contact")
        + ": <b>"
        + ContactListTreeItem::getContactName(m_contact_path)
        + "</b>?<br/>"
        + tr("This wizard will help You to search link")
    ));
    m_label->setWordWrap(true);

    m_vbox->addWidget(m_label);
    setLayout(m_vbox);
}

