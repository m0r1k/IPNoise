#include "intro.hpp"

WizardInstallPageIntro::WizardInstallPageIntro(
    Config  *a_config,
    QWidget *a_parent)
    :   QWizardPage(a_parent)
{
    m_config = a_config;

    setupUi(this);
}

WizardInstallPageIntro::~WizardInstallPageIntro()
{
}

void WizardInstallPageIntro::setupUi(QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/wizard_install_page_intro.png"
    );

    setTitle(tr("Introduction"));
    setPixmap(
        QWizard::WatermarkPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox  = new QVBoxLayout(a_parent);
    m_label = new QLabel(QString(
        tr("Welcome to Pipe!") + "<br/>"
        + tr("This wizard will help You install this application")
    ));
    m_label->setWordWrap(true);

    m_vbox->addWidget(m_label);
    setLayout(m_vbox);
}

