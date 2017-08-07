class Api;

#ifndef API_HPP
#define API_HPP

#include <sys/types.h>
#include <unistd.h>

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QTcpSocket>
#include <QUrl>

// commands
#include "api/apiCommand.hpp"
#include "api/commands/init.hpp"
#include "api/commands/login.hpp"
#include "api/commands/getContactList.hpp"
#include "api/commands/conferenceJoin.hpp"
#include "api/commands/conferenceMsg.hpp"
#include "api/commands/conferenceInvite.hpp"
#include "api/commands/addContact.hpp"
#include "api/commands/addItem.hpp"
#include "api/commands/addGroup.hpp"
#include "api/commands/delGroup.hpp"
#include "api/commands/searchUsers.hpp"
#include "api/commands/updateItem.hpp"
#include "api/commands/updateContact.hpp"
#include "api/commands/logout.hpp"

// events
#include "api/apiEvent.hpp"
#include "api/events/init.hpp"
#include "api/events/loginSuccess.hpp"
#include "api/events/loginFailed.hpp"
#include "api/events/getContactListSuccess.hpp"
#include "api/events/conferenceCreated.hpp"
#include "api/events/conferenceJoin.hpp"
#include "api/events/conferenceMsg.hpp"
#include "api/events/conferenceInvited.hpp"
#include "api/events/updateContactList.hpp"
#include "api/events/updateItem.hpp"
#include "api/events/updateContact.hpp"
#include "api/events/searchUsersAnswer.hpp"
#include "api/events/updateConference.hpp"
#include "api/events/conferenceMsgDelivered.hpp"

#include "config.hpp"
#include "apiContactInfo.hpp"
#include "apiItemInfo.hpp"

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/strings.h>
#include <ipnoise-common/path.hpp>

typedef  QMap<QString, ApiCommand *>        Commands;
typedef  Commands::iterator                 CommandsIt;

typedef  QMap<int32_t, ApiContactInfo *>    ApiContactsInfo;
typedef  ApiContactsInfo::iterator          ApiContactsInfoIt;

typedef  QMap<QString, ApiItemInfo *>       ApiItemsInfo;
typedef  ApiItemsInfo::iterator             ApiItemsInfoIt;

#define SESSID_SIZE 10

#define REGISTER_COMMAND(name)                              \
    do {                                                    \
        CommandsIt  command_it;                             \
        ApiCommand  *command = NULL;                        \
        QString     command_name;                           \
                                                            \
        command = new ApiCommand##name(this);               \
        command_name = command->getName();                  \
                                                            \
        command_it = m_commands.find(command_name);         \
        if (m_commands.end() != command_it){                \
            PERROR("Attempt to double register"             \
                " command: '%s'",                           \
                command_name.toStdString().c_str()          \
            );                                              \
            delete command;                                 \
            break;                                          \
        };                                                  \
        m_commands[command_name] = command;                 \
    } while(0);

typedef  QMap<QString, ApiEvent *>      Events;
typedef  Events::iterator               EventsIt;

#define REGISTER_EVENT(name)                                \
    do {                                                    \
        EventsIt    event_it;                               \
        ApiEvent    *event = NULL;                          \
        QString     event_name;                             \
                                                            \
        event = new ApiEvent##name(this);                   \
        event_name = event->getName();                      \
                                                            \
        event_it = m_events.find(event_name);               \
        if (m_events.end() != event_it){                    \
            PERROR("Attempt to double register"             \
                " event: '%s'",                             \
                event_name.toStdString().c_str()            \
            );                                              \
            delete event;                                   \
            break;                                          \
        };                                                  \
        m_events[event_name] = event;                       \
    } while(0);

class Api
    :   public QObject
{
    Q_OBJECT

    signals:
        void    apiConnected();
        void    apiDisconnected();
        void    itemContactChanged(ApiContactInfo *);
        void    itemInfoChanged(ApiItemInfo *);
        void    apiEvent(QDomElement &);
        void    apiEvent(ApiEventInit *);
        void    apiEvent(ApiEventLoginSuccess *);
        void    apiEvent(ApiEventLoginFailed *);
        void    apiEvent(ApiEventGetContactListSuccess *);
        void    apiEvent(ApiEventConferenceCreated *);
        void    apiEvent(ApiEventConferenceJoin *);
        void    apiEvent(ApiEventConferenceMsg *);
        void    apiEvent(ApiEventConferenceInvited *);
        void    apiEvent(ApiEventUpdateContactList *);
        void    apiEvent(ApiEventUpdateItem *);
        void    apiEvent(ApiEventSearchUsersAnswer *);
        void    apiEvent(ApiEventUpdateConference *);
        void    apiEvent(ApiEventUpdateContact *);
        void    apiEvent(ApiEventConferenceMsgDelivered *);

    public:
        Api(QObject *parent);
        virtual ~Api();

        enum ResourceType {
            RESOURCE_TYPE_ERROR = 0,
            RESOURCE_TYPE_IMAGE,
            RESOURCE_TYPE_SOUND
        };

        void            setConfig(Config *);
        int             tcp_connect();
        ApiCommand  *   getCommand(
            const QString &a_command_name
        );
        ApiEvent    *   getEvent(
            const QString &a_event_name
        );
        void            processCommand(
            const QString           &a_command_name,
            const ApiCommandArgs    &a_args
        );
        void            xmit(const QString &);
        void            queue(const QString &);
        void            sendAck(const QString &a_id = "");

        // sessions
        void            setSrcSessId(const QString &);
        QString         getSrcSessId();
        void            setDstSessId(const QString &);
        QString         getDstSessId();

        void            setSessSeq(const QString &);
        void            setSessSeq(const uint32_t &);
        uint32_t        getSessSeq();
        QString         getSessSeqStr();

        void            setSessAck(const QString &);
        void            setSessAck(const uint32_t &);
        uint32_t        getSessAck();
        QString         getSessAckStr();

        // contact info
        ApiContactInfo *    getContactInfo(int);
        void                deleteContactInfo(
            ApiContactInfo *a_info
        );
        ApiContactInfo *    getCreateContactInfo(int);

        // item info
        ApiItemInfo *   getItemInfo(const QString &);
        void            deleteItemInfo(
            ApiItemInfo *a_info
        );
        ApiItemInfo *   getCreateItemInfo(
            const QString   &a_huid
        );

        void getElementsByTagName(
            QDomElement             &a_element,
            const QString           &a_tagname,
            QList<QDomElement>      &a_out
        );
        QDomElement getElementByTagName(
            QDomElement         &a_element,
            const QString       &a_tagname
        );

        QString generateConfId();

        // resources
        QString resourceImagePath(
            const QString &a_resource_id
        );
        QString resourceSoundPath(
            const QString &a_resource_id
        );
        QString resourcePath(
            const ResourceType  &a_resource_type,
            const QString       &a_resource_id
        );

        // sound
        void playSound(
            const QString &a_resource_id
        );

    protected:
        void    sendReset(
            const QString   &a_dst_sessid,
            const QString   &a_src_sessid
        );
        void    processPacket(QByteArray &a_packet);

    private slots:
        void    connectedCb();
        void    errorCb(QAbstractSocket::SocketError err);
        void    readCb();
        void    processEvent(QDomElement &);
        void    apiItemInfoChangedSlot(ApiItemInfo *);
        void    apiContactInfoChangedSlot(ApiContactInfo *);

    private:
        Config              *m_config;
        Commands            m_commands;
        Events              m_events;
        QTcpSocket          *m_socket;
        QByteArray          m_socket_buffer;
        QString             m_src_sessid;
        QString             m_dst_sessid;
        QString             m_sess_seq;
        QString             m_sess_ack;
        ApiItemsInfo        m_items_info;
        ApiContactsInfo     m_contacts_info;
};

#endif

