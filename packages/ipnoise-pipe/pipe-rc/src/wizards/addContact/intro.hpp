#ifndef WIZARD_ADD_CONTACT_PAGE_INTRO_HPP
#define WIZARD_ADD_CONTACT_PAGE_INTRO_HPP

#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

class WizardAddContactPageIntro
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddContactPageIntro(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddContactPageIntro();

        void setupUi(QWidget *a_parent);

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
};

#endif

