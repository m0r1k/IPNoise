#include "addLink/intro.hpp"
#include "addLink/select.hpp"
#include "addLink/processTcp.hpp"
#include "addLink/processUdp.hpp"

#include "addLink.hpp"

WizardAddLink::WizardAddLink(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   Wizard(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;

    setPage(
        Wizard::Page_WizardAddLinkPageIntro,
        new WizardAddLinkPageIntro(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardAddLinkPageSelect,
        new WizardAddLinkPageSelect(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardAddLinkPageProcessTcp,
        new WizardAddLinkPageProcessTcp(
            m_api,
            m_contact_path,
            this
        )
    );
    setPage(
        Wizard::Page_WizardAddLinkPageProcessUdp,
        new WizardAddLinkPageProcessUdp(
            m_api,
            m_contact_path,
            this
        )
    );

    setWindowTitle(
        tr("Adding new link to") + " '"
            + ContactListTreeItem::getContactName(m_contact_path)
            + "'"
    );
}

WizardAddLink::~WizardAddLink()
{
}

LinkWidget * WizardAddLink::getLink() const
{
    WizardAddLinkPageSelect *select = NULL;

    select = (WizardAddLinkPageSelect *)
        page(Wizard::Page_WizardAddLinkPageSelect);

    return select->getLink();
}

void WizardAddLink::accept()
{
    // close window
    close();
}

