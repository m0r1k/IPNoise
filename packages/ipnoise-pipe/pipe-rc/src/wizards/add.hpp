#ifndef WIZARD_ADD_HPP
#define WIZARD_ADD_HPP

#include <QtCore/QString>
#include <QtCore/QList>

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include <ipnoise-common/log_common.h>
#include "wizard.hpp"
#include "api.hpp"
#include "contactListWidget.hpp"

class WizardAdd
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardAdd(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAdd();

        void    accept();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
};

#endif

