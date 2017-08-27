#ifndef WIDGET_INSTALL_PAGE_FINISH_HPP
#define WIDGET_INSTALL_PAGE_FINISH_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractButton>
#include <QtCore/QString>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>

#include "config.hpp"

class WizardInstallPageFinish
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardInstallPageFinish(
            Config      *a_config,
            QWidget     *a_parent = 0
        );
        virtual ~WizardInstallPageFinish();

        void    setupUi(QWidget *a_parent);

    private:
        Config          *m_config;
        QVBoxLayout     *m_vbox;
};

#endif

