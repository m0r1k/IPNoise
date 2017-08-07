#ifndef WIZARD_HPP
#define WIZARD_HPP

#include <QObject>
#include <QWizard>

class Wizard
    :   public QWizard
{
    Q_OBJECT

    public:
        Wizard(QWidget *a_parent = NULL);
        virtual ~Wizard();

        enum WizardPage {
            // install
            Page_WizardInstallPageIntro             = 1,
            Page_WizardInstallPageSelectInstallDir,
            Page_WizardInstallPageInstall,
            Page_WizardInstallPageFinish
        };
};

#endif

