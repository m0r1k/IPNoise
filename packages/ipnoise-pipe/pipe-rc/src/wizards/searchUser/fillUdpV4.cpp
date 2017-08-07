#include "wizards/searchUser.hpp"
#include "fillUdpV4.hpp"

WizardSearchUserPageFillUdpV4::WizardSearchUserPageFillUdpV4(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;
    m_link          = NULL;

    setupUi(this);
}

WizardSearchUserPageFillUdpV4::~WizardSearchUserPageFillUdpV4()
{
}

void WizardSearchUserPageFillUdpV4::setupUi(QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_searchUser_fillUdpV4.png"
    );

    setTitle(tr("Search via UDP v4. link"));
//        + " " + m_link->getDisplayName());

    setSubTitle(tr("Please specify link information"));
    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );

    // form
    m_form = new QFormLayout(a_parent);

    // ip
    m_ip_label = new QLabel("IP");
    m_form->setWidget(
        0,
        QFormLayout::LabelRole,
        m_ip_label
    );
    m_ip_edit = new QLineEdit;
    m_form->setWidget(
        0,
        QFormLayout::FieldRole,
        m_ip_edit
    );
    connect(
        m_ip_edit,  SIGNAL(textChanged(const QString &)),
        this,       SLOT(ipChanged(const QString &))
    );

    // port
    m_port_label = new QLabel("Port");
    m_form->setWidget(
        1,
        QFormLayout::LabelRole,
        m_port_label
    );
    m_port_edit = new QLineEdit;
    m_form->setWidget(
        1,
        QFormLayout::FieldRole,
        m_port_edit
    );
    connect(
        m_port_edit,    SIGNAL(textChanged(const QString &)),
        this,           SLOT(portChanged(const QString &))
    );
}

void WizardSearchUserPageFillUdpV4::updateAddr()
{
    QString         addr;
    QString         ip      = m_ip_edit->text();
    QString         port    = m_port_edit->text();
    WizardSearchUser   *w      = NULL;

    if (ip.size() && port.size()){
        addr = ip + ":" + port;
    }

    w = (WizardSearchUser *)wizard();
    w->getLink()->setAddr(addr);
}

void WizardSearchUserPageFillUdpV4::ipChanged(
    const QString &)
{
    updateAddr();
}

void WizardSearchUserPageFillUdpV4::portChanged(
    const QString &)
{
    updateAddr();
}


int WizardSearchUserPageFillUdpV4::nextId() const
{
    return Wizard::Page_WizardSearchUserPageProcess;
}

