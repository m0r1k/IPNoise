#ifndef WIZARD_INSTALL_HPP
#define WIZARD_INSTALL_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QString>
#include <QtCore/QList>

#include "wizard.hpp"
#include "config.hpp"

#include <ipnoise-common/log_common.h>

class WizardInstall
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardInstall(
            Config  *a_config,
            QWidget *a_parent = 0
        );
        virtual ~WizardInstall();

        void    accept();
        void    reject();

    private:
        Config  *m_config;
};

#endif

