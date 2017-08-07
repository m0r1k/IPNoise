#include "conferenceWidget.hpp"
#include "conferencesWidget.hpp"
#include "conferenceHelperWidget.hpp"

SelectConferenceButton::SelectConferenceButton(
    ConferenceWidget    *a_conference,
    const QString       &a_text,
    QWidget             *a_parent)
    :   QPushButton(a_text, a_parent)
{
    m_conference = a_conference;

    connect(
        this, SIGNAL(clicked(bool)),
        this, SLOT(clicked(bool))
    );
}

SelectConferenceButton::~SelectConferenceButton()
{
}

void SelectConferenceButton::clicked(bool)
{
    emit clicked(m_conference);
}

// --------------------------------------------

NewConferenceButton::NewConferenceButton(
    const QString       &a_huid,
    const QString       &a_text,
    QWidget             *a_parent)
    :   QPushButton(a_text, a_parent)
{
    m_huid = a_huid;

    connect(
        this, SIGNAL(clicked(bool)),
        this, SLOT(clicked(bool))
    );
}

NewConferenceButton::~NewConferenceButton()
{
}

void NewConferenceButton::clicked(bool)
{
    emit clicked(m_huid);
}

// --------------------------------------------

ConferenceHelperWidget::ConferenceHelperWidget(
    int32_t             a_contact_id,
    Api                 *a_api,
    ConferencesWidget   *a_conferences,
    QWidget             *a_parent)
    :   QWidget(a_parent)
{
    setObjectName("conferenceHelper");

    m_conferences   = a_conferences;
    m_api           = a_api;
    m_contact_id    = a_contact_id;

    setupUi(this);
}

ConferenceHelperWidget::~ConferenceHelperWidget()
{
}

void ConferenceHelperWidget::setupUi(QWidget *a_parent)
{
    int i;
    ApiContactInfo  *contact_info = NULL;
    ContactItems    contact_items;
    ContactItemsIt  contact_items_it;

    contact_info = m_api->getContactInfo(m_contact_id);
    contact_info->getContactItems(contact_items);

    m_vbox = new QVBoxLayout(a_parent);

    // top spacer
    m_top_spacer = new QSpacerItem(
        20, 40,
        QSizePolicy::Minimum,
        QSizePolicy::Expanding
    );

    m_vbox->addItem(m_top_spacer);

    // already opened conferences
    if (0){
        QString m_huid;
        QList<ConferenceWidget *> opened_conferences;
        m_conferences->getByHuid(m_huid, opened_conferences);
        for (i = 0; i < opened_conferences.count(); i++){
            ConferenceWidget        *conference = NULL;
            SelectConferenceButton  *button     = NULL;

            conference = opened_conferences.at(i);
            button = new SelectConferenceButton(
                conference,
                "select conference '"
                + conference->getTopic() + "'",
                a_parent
            );
            connect(
                button,
                SIGNAL(clicked(ConferenceWidget *)),
                this,
                SLOT(selectConference(ConferenceWidget *))
            );
            m_vbox->addWidget(button);
        }
    }

    // start new conference
    for (contact_items_it = contact_items.begin();
        contact_items_it != contact_items.end();
        contact_items_it++)
    {
        NewConferenceButton *button     = NULL;
        ApiItemInfo         *item_info  = NULL;
        QString             huid = contact_items_it.key();

        item_info = m_api->getItemInfo(huid);

        button = new NewConferenceButton(
            huid,
            "start new conference with '"
                + item_info->getNickName() + "'",
            a_parent
        );
        connect(
            button,
            SIGNAL(clicked(QString)),
            this,
            SLOT(startNewConference(QString))
        );
        m_vbox->addWidget(button);
    }

    // bottom spacer
    m_bottom_spacer = new QSpacerItem(
        20, 40,
        QSizePolicy::Minimum,
        QSizePolicy::Expanding
    );
    m_vbox->addItem(m_bottom_spacer);
}

void ConferenceHelperWidget::removeUs()
{
    m_conferences->removeHelper();
}

void ConferenceHelperWidget::startNewConference(
    const QString   &a_huid)
{
    ApiCommandArgs  args;
    ApiContactInfo  *info = m_api->getContactInfo(
        m_contact_id
    );
    QString conf_id = m_api->generateConfId();

    args["flags"]       = "create,excl";
    args["conf_flags"]  = "private";
    args["conf_id"]     = conf_id;
    args["conf_topic"]  = info->getNickName();

    // create pending invite
    m_conferences->setPendingInvite(conf_id, a_huid);

    // request create conference
    m_api->processCommand("conferenceJoin", args);

    // temporary disable us
    setDisabled(true);
}

void ConferenceHelperWidget::selectConference(
    ConferenceWidget    *a_conference)
{
    m_conferences->selectTab(a_conference);
    removeUs();
}

