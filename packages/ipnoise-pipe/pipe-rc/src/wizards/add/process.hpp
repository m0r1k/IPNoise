#ifndef WIDGET_ADD_PAGE_PROCESS_HPP
#define WIDGET_ADD_PAGE_PROCESS_HPP

#include <QWidget>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QDialog>
#include <QMap>

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

