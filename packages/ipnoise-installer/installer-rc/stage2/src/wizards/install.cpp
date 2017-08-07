#include "install/intro.hpp"
#include "install/selectInstallDir.hpp"
#include "install/install.hpp"
#include "install/finish.hpp"

#include "install.hpp"

WizardInstall::WizardInstall(
    Config              *a_config,
    QWidget             *a_parent)
    :   Wizard(a_parent)
{
    m_config = a_config;

    setWindowTitle(QString(
        tr("Installing Pipe")
    ));

    setPage(
        Wizard::Page_WizardInstallPageIntro,
        new WizardInstallPageIntro(m_config)
    );

    setPage(
        Wizard::Page_WizardInstallPageSelectInstallDir,
        new WizardInstallPageSelectInstallDir(m_config)
    );

    setPage(
        Wizard::Page_WizardInstallPageInstall,
        new WizardInstallPageInstall(m_config)
    );

    setPage(
        Wizard::Page_WizardInstallPageFinish,
        new WizardInstallPageFinish(m_config)
    );

    //setOption(QWizard::DisabledBackButtonOnLastPage, true);
    setOption(QWizard::NoBackButtonOnLastPage, true);
}

WizardInstall::~WizardInstall()
{
}

void WizardInstall::accept()
{
    // close app
    done(0);
}

void WizardInstall::reject()
{
    // close app
    done(-1);
}

