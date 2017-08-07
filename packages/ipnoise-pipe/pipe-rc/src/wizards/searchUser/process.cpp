#include "wizards/searchUser.hpp"
#include "linkWidget.hpp"
#include "process.hpp"

WizardSearchUserPageProcess::WizardSearchUserPageProcess(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;
    m_timer         = NULL;

    setupUi(this);

    connect(
        m_api,  SIGNAL(apiEvent(ApiEventSearchUsersAnswer *)),
        this,   SLOT(apiEvent(ApiEventSearchUsersAnswer *))
    );
}

WizardSearchUserPageProcess::~WizardSearchUserPageProcess()
{
}

void WizardSearchUserPageProcess::setupUi(QWidget *a_parent)
{
    m_vbox              = new QVBoxLayout(a_parent);
    m_gbox              = new QGroupBox;
    m_radios_box        = new QVBoxLayout;
    m_progress          = new QProgressBar;
    m_vertical_spacer   = NULL;

    // init results group box
    m_gbox->setLayout(m_radios_box);
    m_gbox->setHidden(true);
    m_vbox->addWidget(m_gbox);

    // add vertical spacer
    m_vertical_spacer = new QSpacerItem(
        20, 40,
        QSizePolicy::Minimum,
        QSizePolicy::Expanding
    );
    m_vbox->addItem(m_vertical_spacer);

    // init progress
    m_progress->setValue(0);
    m_progress->setMaximum(MAX_SEARCH_TIME_SECS);
    m_vbox->addWidget(m_progress);

    setLayout(m_vbox);
}

void WizardSearchUserPageProcess::timerSlot()
{
    int32_t search_time = m_progress->value();
    if (search_time >= MAX_SEARCH_TIME_SECS){
        // timeout
        delete m_timer;
        m_timer = NULL;
        goto out;
    }

    // update state
    m_progress->setValue(search_time + 1);

    if (m_timer){
        m_timer->start(1000);
    }

out:
    QCoreApplication::processEvents();
    return;
}

void WizardSearchUserPageProcess::initializePage()
{
    LinkWidget      *link = NULL;

    link = ((WizardSearchUser *)wizard())->getLink();

    // clear current values if exist
    clearResults();

    // update item, add potential link
    if (0){
        // We have not HUID at this moment
        ApiCommandArgs  args;
        args["link0_name"] = link->getName();
        args["link0_addr"] = link->getAddr();
        m_api->processCommand("updateItem", args);
    }

    // send 'searchUsers' command
    {
        ApiCommandArgs  args;
        args["link_name"] = link->getName();
        args["link_addr"] = link->getAddr();
        m_api->processCommand("searchUsers", args);
    }

    // init progress
    m_progress->setValue(0);

    // setup timer
    m_timer = new QTimer;
    connect(
        m_timer,    SIGNAL(timeout()),
        this,       SLOT(timerSlot())
    );
    m_timer->start(0);
}

int WizardSearchUserPageProcess::nextId() const
{
    return -1;
}

QString WizardSearchUserPageProcess::getHuid()
{
    QString huid;
    QList<QRadioButton *>::iterator m_radios_it;
    for (m_radios_it = m_radios.begin();
        m_radios_it != m_radios.end();
        m_radios_it++)
    {
        QRadioButton *radio = *m_radios_it;
        if (radio->isChecked()){
            huid = radio->objectName();
            break;
        }
    }

    return huid;
}

void WizardSearchUserPageProcess::clearResults()
{
    QList<QRadioButton *>::iterator m_radios_it;

    m_gbox->setHidden(true);

    if (not m_radios_box){
        goto out;
    }

    for (m_radios_it = m_radios.begin();
        m_radios_it != m_radios.end();
        m_radios_it++)
    {
        QRadioButton *radio = *m_radios_it;
        delete radio;
    }

    m_radios.clear();
    delete m_radios_box;
    m_radios_box = NULL;

out:
    return;
}

void WizardSearchUserPageProcess::apiEvent(
    ApiEventSearchUsersAnswer   *a_event)
{
    QDomElement                     event_el;
    QDomElement                     users_el;
    QList<QDomElement>              users;
    QList<QDomElement>::iterator    users_it;

    // mark as done
    m_progress->setValue(MAX_SEARCH_TIME_SECS);

    // clear current values
    clearResults();

    m_radios_box = new QVBoxLayout;
    m_radios_box->setStretch(1, 1);
    m_gbox->setLayout(m_radios_box);

    event_el = a_event->getElement();
    users_el = m_api->getElementByTagName(
        event_el,
        "users"
    );
    if (users_el.isNull()){
        goto out;
    }

    m_api->getElementsByTagName(
        users_el,
        "user",
        users
    );

    for (users_it = users.begin();
        users_it != users.end();
        users_it++)
    {
        QDomElement     user     = *users_it;
        QString         huid     = user.attribute("huid");
        QString         nickname = user.attribute("nickname");
        QRadioButton    *radio   = new QRadioButton(
            nickname + " (" + huid + ")"
        );
        radio->setObjectName(huid);
        m_radios_box->addWidget(radio);
        m_radios.push_back(radio);
    }

    if (users.size()){
        m_gbox->setHidden(false);
    }

out:
    return;
}

