#ifndef WIZARD_SEARCH_USER_PAGE_SELECT_HPP
#define WIZARD_SEARCH_USER_PAGE_SELECT_HPP

#include <QtCore/QString>
#include <QtCore/QList>

#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpacerItem>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"

#include "linkWidget.hpp"
#include "wizard.hpp"

class WizardSearchUserPageSelect
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardSearchUserPageSelect(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardSearchUserPageSelect();

        void            setupUi(QWidget *a_parent);
        void            createLinksCombo();
        LinkWidget *    getLink() const;

        virtual int nextId() const;

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QLabel              *m_label;
        QVBoxLayout         *m_vbox;
        QComboBox           *m_combo_links;
        Links               m_links;
};

#endif

