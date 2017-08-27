#ifndef WIZARD_INSTALL_PAGE_INTRO_HPP
#define WIZARD_INSTALL_PAGE_INTRO_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QString>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "config.hpp"

class WizardInstallPageIntro
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardInstallPageIntro(
            Config      *a_config,
            QWidget     *a_parent = 0
        );
        virtual ~WizardInstallPageIntro();

        void setupUi(QWidget *a_parent);

    private:
        Config              *m_config;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
};

#endif

