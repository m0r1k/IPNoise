#ifndef WIZARD_ADD_GROUP_PAGE_PROCESS_HPP
#define WIZARD_ADD_GROUP_PAGE_PROCESS_HPP

#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

class WizardAddGroupPageProcess
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddGroupPageProcess(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddGroupPageProcess();

        void        setupUi(QWidget *a_parent);
        QString     getGroupName();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
        QFormLayout         *m_form;
        QLabel              *m_group_name_label;
        QLineEdit           *m_group_name_edit;
};

#endif

