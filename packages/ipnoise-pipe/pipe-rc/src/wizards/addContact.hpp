#ifndef WIZARD_ADD_CONTACT_HPP
#define WIZARD_ADD_CONTACT_HPP

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "api.hpp"
#include "wizard.hpp"

class WizardAddContact
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardAddContact(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddContact();

        void    accept();

    private:
        Api                *m_api;
        ContactListPath     m_contact_path;
};

#endif

