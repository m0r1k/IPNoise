#ifndef WIZARD_SEARCH_USER_HPP
#define WIZARD_SEARCH_USER_HPP

#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "linkWidget.hpp"
#include "api.hpp"
#include "wizard.hpp"

class WizardSearchUser
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardSearchUser(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardSearchUser();

        void            setupUi(QWidget *);
        LinkWidget *    getLink() const;
        QString         getHuid() const;
        void            accept();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
};

#endif

