#include <QColor>

#include "conferencesWidget.hpp"
#include "conferenceWidget.hpp"

ConferenceWidget::ConferenceWidget(
    const QString       &a_id,
    Api                 *a_api,
    ConferencesWidget   *a_conferences,
    QWidget             *a_parent)
    :   QWidget(a_parent)
{
    setObjectName("conference");

    m_conferences   = a_conferences;
    m_id            = a_id;
    m_api           = a_api;

    setupUi(this);

    connect(
        m_edit,
        SIGNAL(submit()),
        this,
        SLOT(sendMsg())
    );
    connect(
        m_api,
        SIGNAL(itemInfoChanged(ApiItemInfo *)),
        this,
        SLOT(itemInfoChangedSlot(ApiItemInfo *))
    );
}

ConferenceWidget::~ConferenceWidget()
{
}

QString ConferenceWidget::getTopic()
{
    return m_conferences->getTabName(this);
}

void ConferenceWidget::setTopic(const QString &a_topic)
{
    m_conferences->setTabName(this, a_topic);
}

void ConferenceWidget::setId(const QString &a_id)
{
    m_id = a_id;
}

QString ConferenceWidget::getId()
{
    return m_id;
}

void ConferenceWidget::setupUi(QWidget *a_parent)
{
    m_vbox      = new QVBoxLayout(a_parent);
    m_hbox1     = new QHBoxLayout;
    m_history   = new QListWidget;
    m_vbox1     = new QVBoxLayout;
    m_members   = new QListWidget;
    m_invite    = new QPushButton("invite");
    m_status    = new QLabel("status line");
    m_edit      = new TextEditWidget;

    // members
    m_members->setMaximumWidth(100);
    m_vbox1->addWidget(m_members);
    m_vbox1->addWidget(m_invite);

    // history
    m_history->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
    m_hbox1->addWidget(m_history);
    m_hbox1->addLayout(m_vbox1);

    // message edit
    m_edit->setMaximumHeight(50);
    m_edit->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::MinimumExpanding
    );

    m_vbox->addLayout(m_hbox1);
    m_vbox->addWidget(m_status);
    m_vbox->addWidget(m_edit);

    setDisabled(true);
}

ConferenceItemWidget *ConferenceWidget::getItemByHuid(
    const QString &a_huid)
{
    int i;
    ConferenceItemWidget *item = NULL;

    for (i = 0; i < m_members->count(); i++){
        ConferenceItemWidget *cur_item = NULL;
        cur_item = (ConferenceItemWidget *)m_members->item(i);
        if (a_huid == cur_item->getHuid()){
            item = cur_item;
            break;
        }
    }

    return item;
}

bool ConferenceWidget::haveHuid(const QString &a_huid)
{
    ConferenceItemWidget *item = NULL;
    item = getItemByHuid(a_huid);
    return item;
}

void ConferenceWidget::itemInfoChanged(
    ApiItemInfo             *a_info,
    ConferenceItemWidget    *a_item)
{
    QString huid;

    if (not a_info && not a_item){
        goto out;
    }

    // search huid
    if (a_info){
        huid = a_info->getHuid();
    }
    if (a_item){
        huid = a_item->getHuid();
    }

    // search info
    if (not a_info){
        a_info = m_api->getItemInfo(huid);
    }

    if (not a_info){
        PERROR("Cannot get item info for huid: '%s'\n",
            huid.toStdString().c_str()
        );
        goto out;
    }

    // search item
    if (not a_item){
        a_item = getItemByHuid(huid);
    }

    // TODO XXX we have not such item? wtf?
    if (not a_item){
        PERROR("Item with huid: '%s' not found\n",
            huid.toStdString().c_str()
        );
        goto out;
    }

    a_item->setText(a_info->getNickName());

    // update item
    if (not a_info->isOnline()){
        a_item->setForeground(QBrush(Qt::darkGray));
    } else {
       a_item->setForeground(QBrush(Qt::black));
    }

out:
    return;
}

void ConferenceWidget::itemInfoChangedSlot(
    ApiItemInfo     *a_info)
{
    itemInfoChanged(a_info, NULL);
}

ConferenceItemWidget * ConferenceWidget::getCreateHuid(
    const QString &a_huid)
{
    ConferenceItemWidget *item = NULL;
    item = getItemByHuid(a_huid);
    if (item){
        goto out;
    }
    item = new ConferenceItemWidget(m_api, m_members, a_huid);
    itemInfoChanged(NULL, item);
    m_members->addItem(item);
    setDisabled(false);

out:
    return item;
}

void ConferenceWidget::sendMsg()
{
    ApiCommandArgs  args;
    QString         msg = m_edit->toPlainText();

    if (!msg.size()){
        goto out;
    }

    while(int msg_size = msg.size()){
        // remove new lines from end
        if (    '\r' == msg.at(msg_size - 1)
            ||  '\n' == msg.at(msg_size - 1))
        {
            msg.replace(msg_size - 1, 1, "");
        } else {
            break;
        }
    }

    args["conf_id"] = getId();
    args["msg"]     = msg;

    m_api->processCommand("conferenceMsg", args);
    m_edit->clear();

out:
    return;
}

void ConferenceWidget::addMsg(
    const QString &a_msg_id,
    const QString &a_msg_value,
    const QString &a_msg_type)
{
    ConferenceMsgWidget *msg            = NULL;
    QWidget             *focused_widget = NULL;

    msg = new ConferenceMsgWidget();
    msg->setMsgId(a_msg_id);
    msg->setText(a_msg_value);

    if ("outcoming" == a_msg_type){
        msg->setBackground(QColor(Qt::gray));
    }

    m_history->addItem(msg);
    m_history->scrollToItem(msg);

    // play sound
    focused_widget = QApplication::focusWidget();
    if (    "incoming" == a_msg_type
        &&  (not focused_widget
            ||  (void *)focused_widget != (void *)m_edit))
    {
        m_api->playSound("new_msg");
    }
}

void ConferenceWidget::msgDelivered(
    const QString &a_msg_id)
{
    int32_t i;

    for (i = (m_history->count() - 1); i >= 0; i--){
        ConferenceMsgWidget *msg = NULL;
        msg = (ConferenceMsgWidget *)m_history->item(i);
        if (msg->getMsgId() == a_msg_id){
            msg->setBackground(QColor(Qt::white));
            break;
        }
    }
}

