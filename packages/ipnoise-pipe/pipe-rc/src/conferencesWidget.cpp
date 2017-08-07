#include "conferencesWidget.hpp"

ConferencesWidget::ConferencesWidget(
    Api     *a_api,
    QWidget *a_parent)
    :   QWidget(a_parent)
{
    m_api               = a_api;
    m_pending_invite    = NULL;

    setupUi(this);

    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventConferenceCreated *)),
        this,
        SLOT(apiEvent(ApiEventConferenceCreated *))
    );
    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventConferenceJoin *)),
        this,
        SLOT(apiEvent(ApiEventConferenceJoin *))
    );
    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventConferenceMsg *)),
        this,
        SLOT(apiEvent(ApiEventConferenceMsg *))
    );
    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventConferenceInvited *)),
        this,
        SLOT(apiEvent(ApiEventConferenceInvited *))
    );
    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventUpdateConference *)),
        this,
        SLOT(apiEvent(ApiEventUpdateConference *))
    );
    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventConferenceMsgDelivered *)),
        this,
        SLOT(apiEvent(ApiEventConferenceMsgDelivered *))
    );
}

ConferencesWidget::~ConferencesWidget()
{
}

void ConferencesWidget::setupUi(QWidget *a_parent)
{
    m_vbox = new QVBoxLayout(a_parent);
    m_tabs = new QTabWidget();

    m_tabs->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );

    m_vbox->addWidget(m_tabs);
    setLayout(m_vbox);
}

void ConferencesWidget::apiEvent(
    ApiEventConferenceCreated *a_event)
{
    ConferenceWidget    *conference = NULL;
    QString             conf_id     = a_event->getConfId();
    QString             conf_topic   = a_event->getConfName();

    conference = new ConferenceWidget(
        conf_id,
        m_api,
        this,
        m_tabs
    );
    m_tabs->addTab(conference, conf_topic);

    if (m_pending_invite
        && m_pending_invite->m_conf_id == conf_id)
    {
        // we have pending invite, process it
        ApiCommandArgs args;
        args["conf_id"] = m_pending_invite->m_conf_id;
        args["huid"]    = m_pending_invite->m_huid;
        m_api->processCommand("conferenceInvite", args);
        // remove helper
        removeHelper();
        // remove pending invite
        delete m_pending_invite;
        m_pending_invite = NULL;
    }
}

void ConferencesWidget::apiEvent(
    ApiEventConferenceMsg *a_event)
{
    ConferenceWidget    *conference = NULL;
    QString             conf_id     = a_event->getConfId();
    QString             msg_id      = a_event->getMsgId();
    QString             msg_value   = a_event->getMsgValue();
    QString             msg_type    = a_event->getMsgType();

    if (!conf_id.size()){
        PERROR("empty conf_id\n");
        goto fail;
    }

    if (!msg_id.size()){
        PERROR("empty msg_id\n");
        goto fail;
    }

    if (!msg_value.size()){
        PERROR("empty msg_value\n");
        goto fail;
    }

    if (!msg_type.size()){
        PERROR("empty msg_type\n");
        goto fail;
    }

    PDEBUG(5, "ConferencesWidget::conferenceMsg:\n"
        " conf_id:   '%s'\n"
        " msg_id:    '%s'\n"
        " msg_value: '%s'\n"
        " msg_type:  '%s'\n",
        conf_id.toStdString().c_str(),
        msg_id.toStdString().c_str(),
        msg_value.toStdString().c_str(),
        msg_type.toStdString().c_str()
    );

    conference = getById(conf_id);
    if (!conference){
        PERROR("Cannot get conference by id: '%s'\n",
            conf_id.toStdString().c_str());
    }

    conference->addMsg(msg_id, msg_value, msg_type);

out:
    return;
fail:
    goto out;
}

void ConferencesWidget::apiEvent(
    ApiEventConferenceMsgDelivered *a_event)
{
    ConferenceWidget    *conference = NULL;
    QString             conf_id     = a_event->getConfId();
    QString             msg_id      = a_event->getMsgId();

    if (!conf_id.size()){
        PERROR("empty conf_id\n");
        goto fail;
    }

    if (!msg_id.size()){
        PERROR("empty msg_id\n");
        goto fail;
    }

    PDEBUG(5, "ConferencesWidget::conferenceMsgDelivered:\n"
        " conf_id:   '%s'\n"
        " msg_id:    '%s'\n",
        conf_id.toStdString().c_str(),
        msg_id.toStdString().c_str()
    );

    conference = getById(conf_id);
    if (!conference){
        PERROR("Cannot get conference by id: '%s'\n",
            conf_id.toStdString().c_str());
    }

    conference->msgDelivered(msg_id);

out:
    return;
fail:
    goto out;
}


void ConferencesWidget::apiEvent(
    ApiEventConferenceJoin *a_event)
{
    ConferenceWidget        *conference = NULL;
    QString                 conf_id     = a_event->getConfId();
    QString                 huid        = a_event->getHuid();

    if (!conf_id.size()){
        PERROR("empty conf_id\n");
        goto fail;
    }

    if (!huid.size()){
        PERROR("empty huid\n");
        goto fail;
    }

    conference = getById(conf_id);
    if (!conference){
        PERROR("Cannot get conference by id: '%s'\n",
            conf_id.toStdString().c_str());
    }

    conference->getCreateHuid(huid);
    // item->setForeground(QBrush(Qt::black));

out:
    return;
fail:
    goto out;
}

void ConferencesWidget::apiEvent(
    ApiEventConferenceInvited *a_event)
{
    ConferenceWidget    *conference = NULL;
    QString             conf_id     = a_event->getConfId();
    QString             huid        = a_event->getHuid();

    if (!conf_id.size()){
        PERROR("empty conf_id\n");
        goto fail;
    }

    if (!huid.size()){
        PERROR("empty huid\n");
        goto fail;
    }

    conference = getById(conf_id);
    if (!conference){
        PERROR("Cannot get conference by id: '%s'\n",
            conf_id.toStdString().c_str());
    }

    conference->getCreateHuid(huid);

out:
    return;
fail:
    goto out;
}

void ConferencesWidget::apiEvent(
    ApiEventUpdateConference    *a_event)
{
    ConferenceWidget    *conference = NULL;
    QString             conf_id     = a_event->getConfId();
    QString             conf_topic  = a_event->getConfTopic();

    if (!conf_id.size()){
        PERROR("empty conf_id\n");
        goto fail;
    }

    conference = getById(conf_id);
    if (not conference){
        PERROR("Cannot get conference by id: '%s'\n",
            conf_id.toStdString().c_str());
        goto out;
    }

    conference->setTopic(conf_topic);

out:
    return;
fail:
    goto out;

}

ConferenceWidget * ConferencesWidget::getById(
    const QString   &a_id)
{
    int i;
    ConferenceWidget *conference = NULL;

    // search private conference this such huid
    for (i = 0; i < m_tabs->count(); i++){
        ConferenceWidget *cur_conference = NULL;
        cur_conference = (ConferenceWidget *)m_tabs->widget(i);
        if (a_id == cur_conference->getId()){
            conference = cur_conference;
            break;
        }
    }

    return conference;
}

void ConferencesWidget::getByHuid(
    const QString               &a_huid,
    QList<ConferenceWidget *>   &a_out)
{
    int i;

    // search any conferences this such huid
    for (i = 0; i < m_tabs->count(); i++){
        ConferenceWidget    *cur_conference = NULL;
        QWidget             *cur_widget     = NULL;
        cur_widget = m_tabs->widget(i);
        if ("conference" != cur_widget->objectName()){
            continue;
        }
        cur_conference = (ConferenceWidget *)m_tabs->widget(i);
        if (cur_conference->haveHuid(a_huid)){
            a_out.push_back(cur_conference);
        }
    }
}

ConferenceHelperWidget * ConferencesWidget::getHelper()
{
    int i;
    ConferenceHelperWidget *helper = NULL;
    for (i = 0; i < m_tabs->count(); i++){
        QWidget *cur_widget = NULL;
        cur_widget = m_tabs->widget(i);
        if ("conferenceHelper" == cur_widget->objectName()){
            helper = (ConferenceHelperWidget *)cur_widget;
            break;
        }
    }
    return helper;
}

ConferenceHelperWidget * ConferencesWidget::getCreateHelper(
    int32_t a_contact_id)
{
    ConferenceHelperWidget *helper = NULL;
    helper = getHelper();
    if (not helper){
        helper = new ConferenceHelperWidget(
            a_contact_id,
            m_api,
            this,
            m_tabs
        );
        m_tabs->addTab(helper, "helper");
    }
    return helper;
}

void ConferencesWidget::removeHelper()
{
    int                     index   = -1;
    ConferenceHelperWidget  *helper = NULL;

    helper = getHelper();
    if (not helper){
        goto out;
    }

    index = getTabIndex(helper);
    if (index < 0){
        goto out;
    }

    removeTab(index);

out:
    return;
}

int ConferencesWidget::getTabIndex(QWidget *a_widget)
{
    int i, index = -1;

    for (i = 0; i < m_tabs->count(); i++){
        QWidget *cur_widget = NULL;
        cur_widget = m_tabs->widget(i);
        if ((void *)cur_widget == (void *)a_widget){
            index = i;
            break;
        }
    }

    return index;
}

QString ConferencesWidget::getTabName(QWidget *a_widget)
{
    int     index = -1;
    QString name;

    index = getTabIndex(a_widget);
    if (index < 0){
        goto out;
    }

    name = m_tabs->tabText(index);

out:
    return name;
}

void ConferencesWidget::setTabName(
    QWidget         *a_widget,
    const QString   &a_tab_name)
{
    int     index = -1;
    QString name;

    index = getTabIndex(a_widget);
    if (index < 0){
        goto out;
    }

    m_tabs->setTabText(index, a_tab_name);

out:
    return;
}


void ConferencesWidget::removeTab(int a_index)
{
    QWidget *widget = getTabWidget(a_index);

    if (not widget){
        goto fail;
    }

    m_tabs->removeTab(a_index);
    delete widget;

out:
    return;
fail:
    goto out;
}

QWidget * ConferencesWidget::getTabWidget(int a_index)
{
    QWidget *widget = NULL;

    if (a_index < 0){
        goto fail;
    }

    widget = m_tabs->widget(a_index);

out:
    return widget;
fail:
    goto out;
}

void ConferencesWidget::selectTab(QWidget *a_widget)
{
    int index = -1;

    index = getTabIndex(a_widget);
    if (index >= 0){
        m_tabs->setCurrentIndex(index);
    }
}

void ConferencesWidget::itemClicked(
    int32_t a_contact_id)
{
    ConferenceHelperWidget *helper = NULL;

    removeHelper();
    helper = getCreateHelper(a_contact_id);
    selectTab(helper);
}

void ConferencesWidget::setPendingInvite(
    const QString   &a_conf_id,
    const QString   &a_huid)
{
    if (m_pending_invite){
        PFATAL("pending invite already exist\n");
    }
    m_pending_invite = new PendingInvite;
    m_pending_invite->m_conf_id = a_conf_id;
    m_pending_invite->m_huid    = a_huid;
}

