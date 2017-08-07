#ifndef WIZARD_INSTALL_PAGE_INTRO_HPP
#define WIZARD_INSTALL_PAGE_INTRO_HPP

#include <QWidget>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>

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

