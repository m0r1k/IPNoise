#ifndef WIZARD_ADD_LINK_HPP
#define WIZARD_ADD_LINK_HPP

#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "api.hpp"
#include "wizard.hpp"
#include "linkWidget.hpp"

class WizardAddLink
    :   public Wizard
{
    Q_OBJECT

    public:
        WizardAddLink(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddLink();

        void        accept();
//        void        setAddr(const QString &);
//        QString     getAddr();

        LinkWidget * getLink() const;

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QString             m_addr;
};

#endif

