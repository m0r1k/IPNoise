#ifndef ADD_WIZARD_ADD_LINK_PAGE_INTRO_HPP
#define ADD_WIZARD_ADD_LINK_PAGE_INTRO_HPP

#include <QWidget>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

class WizardAddLinkPageIntro
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddLinkPageIntro(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddLinkPageIntro();

        void setupUi(QWidget *a_parent);

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
};

#endif

