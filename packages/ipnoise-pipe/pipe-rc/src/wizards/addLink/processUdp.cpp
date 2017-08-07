#include "wizards/addLink.hpp"
#include "processUdp.hpp"

WizardAddLinkPageProcessUdp::WizardAddLinkPageProcessUdp(
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

WizardAddLinkPageProcessUdp::~WizardAddLinkPageProcessUdp()
{
}

void WizardAddLinkPageProcessUdp::setupUi(QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/default/images/wizard_addLink_process_udp.png"
    );

    setTitle(tr("Adding UDP v4. link"));
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

void WizardAddLinkPageProcessUdp::updateAddr()
{
    QString         addr;
    QString         ip      = m_ip_edit->text();
    QString         port    = m_port_edit->text();
    WizardAddLink   *w      = NULL;

    if (ip.size() && port.size()){
        addr = ip + ":" + port;
    }

    w = (WizardAddLink *)wizard();
    w->getLink()->setAddr(addr);
}

void WizardAddLinkPageProcessUdp::ipChanged(
    const QString &)
{
    updateAddr();
}

void WizardAddLinkPageProcessUdp::portChanged(
    const QString &)
{
    updateAddr();
}


int WizardAddLinkPageProcessUdp::nextId() const
{
    return -1;
}

