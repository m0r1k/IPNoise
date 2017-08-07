#ifndef WIZARD_HPP
#define WIZARD_HPP

#include <QObject>
#include <QWizard>

class Wizard
    :   public QWizard
{
    Q_OBJECT

    public:
        Wizard(QWidget *a_parent);
        virtual ~Wizard();

        enum WizardPage {
            // add group
            Page_WizardAddGroupPageProcess = 1,
            // add..
            Page_WizardAddPageIntro,
            Page_WizardAddPageSelect,
            // add user
            Page_WizardAddContactPageIntro,
            Page_WizardAddContactPageProcess,
            // add link
            Page_WizardAddLinkPageIntro,
            Page_WizardAddLinkPageSelect,
            Page_WizardAddLinkPageProcessTcp,
            Page_WizardAddLinkPageProcessUdp,
            // del group
            Page_WizardDelGroupPageProcess,
            // search user
            Page_WizardSearchUserPageIntro,
            Page_WizardSearchUserPageSelect,
            Page_WizardSearchUserPageFillUdpV4,
            Page_WizardSearchUserPageFillTcpV4,
            Page_WizardSearchUserPageProcess
        };
};

#endif

