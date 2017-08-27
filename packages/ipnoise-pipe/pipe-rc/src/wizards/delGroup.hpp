#ifndef WIZARD_DEL_GROUP_HPP
#define WIZARD_DEL_GROUP_HPP

#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "api.hpp"
#include "wizard.hpp"

class WizardDelGroup
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardDelGroup(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardDelGroup();

        void    setupUi(QWidget *);
        void    accept();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
};

#endif

