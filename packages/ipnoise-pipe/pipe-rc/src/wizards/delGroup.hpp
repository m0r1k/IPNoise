#ifndef WIZARD_DEL_GROUP_HPP
#define WIZARD_DEL_GROUP_HPP

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>

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

