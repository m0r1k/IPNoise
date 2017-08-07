#include "finish.hpp"

WizardInstallPageFinish::WizardInstallPageFinish(
    Config      *a_config,
    QWidget     *a_parent)
    :   QWizardPage(a_parent)
{
    m_config = a_config;

    setupUi(this);
}

WizardInstallPageFinish::~WizardInstallPageFinish()
{
}

void WizardInstallPageFinish::setupUi(QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/resources/wizard_install_page_finish.png"
    );

    setTitle(tr("Pipe installation finished"));
    setSubTitle(tr("Installation finished"));

    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );

    m_vbox = new QVBoxLayout(a_parent);

//    QAbstractButton *button = NULL;
//    button = wizard()->button(QWizard::BackButton);
//    if (button){ button->setEnabled(false); }
//    button = wizard()->button(QWizard::CancelButton);
//    if (button){ button->setEnabled(false); }


    setLayout(m_vbox);
}

