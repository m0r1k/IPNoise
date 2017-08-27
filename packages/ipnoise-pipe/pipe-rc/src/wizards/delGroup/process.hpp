#ifndef WIZARD_DEL_GROUP_PAGE_PROCESS_HPP
#define WIZARD_DEL_GROUP_PAGE_PROCESS_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QRadioButton>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

class WizardDelGroupPageProcess
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardDelGroupPageProcess(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardDelGroupPageProcess();

        void        setupUi(QWidget *a_parent);
        QString     getDelStrategy();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
        QRadioButton        *m_strategy_0;
        QRadioButton        *m_strategy_1;
};

#endif

