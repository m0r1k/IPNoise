#ifndef WIZARD_ADD_PAGE_SELECT_HPP
#define WIZARD_ADD_PAGE_SELECT_HPP

#include <QtCore/QString>
#include <QtCore/QList>

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

#include "wizard.hpp"

class WizardAddPageSelect
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddPageSelect(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddPageSelect();

        void        setupUi(QWidget *a_parent);
        virtual int nextId() const;

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
        QVBoxLayout         *m_types_vbox;
        QGroupBox           *m_types;
        QRadioButton        *m_type_new_user;
        QRadioButton        *m_type_new_group;
        QRadioButton        *m_type_new_notes;
        QRadioButton        *m_type_new_file;
};

#endif

