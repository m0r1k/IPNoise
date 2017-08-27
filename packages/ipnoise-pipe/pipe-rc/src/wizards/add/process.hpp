#ifndef WIDGET_ADD_PAGE_PROCESS_HPP
#define WIDGET_ADD_PAGE_PROCESS_HPP

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QDialog>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

#include "wizards/addLink.hpp"

class WizardAddPageProcess
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddPageProcess(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddPageProcess();

        void    setupUi(QWidget *a_parent);

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QVBoxLayout         *m_vbox;
        QTabWidget          *m_tabs;
        QPushButton         *m_add_button;
        QComboBox           *m_add_type;
};

#endif

