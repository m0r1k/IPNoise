#include <vector>
#include <string>

#if 0
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#endif

#if defined(WIN32) || defined(WIN64)
    #include <windows.h>
    #include <mmsystem.h>
#endif

#include <ipnoise-common/rand.hpp>
#include <ipnoise-common/utils.hpp>
#include <ipnoise-common/log.h>

#include "api.hpp"

Api::Api(QObject *parent)
    :   QObject(parent)
{
    m_config    = NULL;
    m_socket    = NULL;

    // init session info
    setSrcSessId(getRand(SESSID_SIZE).c_str());
    setSessSeq(0);
    setSessAck("");

    // commands
    REGISTER_COMMAND(Init);
    REGISTER_COMMAND(Login);
    REGISTER_COMMAND(GetContactList);
    REGISTER_COMMAND(ConferenceJoin);
    REGISTER_COMMAND(ConferenceMsg);
    REGISTER_COMMAND(ConferenceInvite);
    REGISTER_COMMAND(AddContact);
    REGISTER_COMMAND(AddItem);
    REGISTER_COMMAND(AddGroup);
    REGISTER_COMMAND(DelGroup);
    REGISTER_COMMAND(SearchUsers);
    REGISTER_COMMAND(UpdateItem);
    REGISTER_COMMAND(UpdateContact);
    REGISTER_COMMAND(Logout);

    // events
    REGISTER_EVENT(Init);
    REGISTER_EVENT(LoginSuccess);
    REGISTER_EVENT(LoginFailed);
    REGISTER_EVENT(GetContactListSuccess);
    REGISTER_EVENT(ConferenceCreated);
    REGISTER_EVENT(ConferenceJoin);
    REGISTER_EVENT(ConferenceMsg);
    REGISTER_EVENT(ConferenceInvited);
    REGISTER_EVENT(UpdateContactList);
    REGISTER_EVENT(UpdateItem);
    REGISTER_EVENT(UpdateContact);
    REGISTER_EVENT(SearchUsersAnswer);
    REGISTER_EVENT(UpdateConference);
    REGISTER_EVENT(ConferenceMsgDelivered);

    connect(
        this,   SIGNAL(apiEvent(QDomElement &)),
        this,   SLOT(processEvent(QDomElement &))
    );
}

Api::~Api()
{
    // delete items info
    {
       ApiItemsInfoIt it;
       for (it = m_items_info.begin();
            it != m_items_info.end();
            it++)
        {
            ApiItemInfo *info = it.value();
            delete info;
        }
        m_items_info.clear();
    }
}

void Api::setConfig(Config *a_config)
{
    m_config = a_config;
}

// ---------------- sess seq ------------------------------

void Api::setSessSeq(const QString &a_val)
{
    m_sess_seq = a_val;
}

void Api::setSessSeq(const uint32_t &a_val)
{
    char buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%x",
        a_val
    );
    m_sess_seq = buffer;
}

uint32_t Api::getSessSeq()
{
    bool        ok  = false;
    uint32_t    ret = 0;

    ret = m_sess_seq.toUInt(&ok, 16);   // from hex
    return ret;
}

QString Api::getSessSeqStr()
{
    return m_sess_seq;
}

// ---------------- sess ack ------------------------------

void Api::setSessAck(const QString &a_val)
{
    m_sess_ack = a_val;
}

void Api::setSessAck(const uint32_t &a_val)
{
    char buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%x",
        a_val
    );
    m_sess_ack = buffer;
}

uint32_t Api::getSessAck()
{
    bool        ok  = false;
    uint32_t    ret = 0;

    ret = m_sess_ack.toUInt(&ok, 16);   // from hex
    return ret;
}

QString Api::getSessAckStr()
{
    PWARN("Api::getSessAckStr: '%s'\n",
        m_sess_ack.toStdString().c_str()
    );
    return m_sess_ack;
}

// ---------------- src sessid ----------------------------

void Api::setSrcSessId(const QString &a_sessid)
{
    m_src_sessid = a_sessid;
}

QString Api::getSrcSessId()
{
    return m_src_sessid;
}

void Api::setDstSessId(const QString &a_sessid)
{
    m_dst_sessid = a_sessid;
}

QString Api::getDstSessId()
{
    return m_dst_sessid;
}

ApiCommand * Api::getCommand(const QString &a_command_name)
{
    ApiCommand  *command = NULL;
    CommandsIt  command_it;

    if (!a_command_name.size()){
        goto out;
    }

    command_it = m_commands.find(a_command_name);
    if (m_commands.end() != command_it){
        command = command_it.value();
    }

out:
    return command;
}

ApiEvent * Api::getEvent(const QString &a_event_name)
{
    ApiEvent    *event = NULL;
    EventsIt    event_it;

    if (!a_event_name.size()){
        goto out;
    }

    event_it = m_events.find(a_event_name);
    if (m_events.end() != event_it){
        event = event_it.value();
    }

out:
    return event;
}

void Api::processCommand(
    const QString           &a_command_name,
    const ApiCommandArgs    &a_args)
{
    ApiCommand      *command    = NULL;
    ApiCommandArgs  args        = a_args;

    if (m_src_sessid.size()){
        args["src_sessid"] = m_src_sessid;
    }

    if (m_dst_sessid.size()){
        args["dst_sessid"] = m_dst_sessid;
    }

    command = getCommand(a_command_name);
    if (!command){
        PERROR("No such command: '%s'\n",
            a_command_name.toStdString().c_str());
        goto fail;
    }

    command->process(args);

out:
    return;
fail:
    goto out;
}

void Api::xmit(const QString &a_cmd)
{
    PWARN("[ API write ]\n"
        "    '%s'\n", a_cmd.toStdString().c_str());
    m_socket->write(a_cmd.toStdString().c_str());
}

void Api::queue(const QString &a_cmd)
{
    xmit(a_cmd);
    setSessSeq(getSessSeq() + 1);
    PINFO("seq: 0x'%x'\n", getSessSeq());
}

void Api::connectedCb()
{
    emit apiConnected();
}

void Api::errorCb(
    QAbstractSocket::SocketError)
{
    PWARN("QAbstractSocket::SocketError\n");
    emit apiDisconnected();
}

void Api::processEvent(QDomElement &a_event)
{
    ApiEvent *event     = NULL;
    QString  event_type = a_event.attribute("type");

    event = getEvent(event_type);
    if (!event){
        PERROR("Unsupported event: '%s'\n",
            event_type.toStdString().c_str());
        goto out;
    }

    event->process(a_event);

    if ("loginSuccess" == event_type){
        emit apiEvent((ApiEventLoginSuccess *)event);
    } else if ("loginFailed" == event_type){
        emit apiEvent((ApiEventLoginFailed *)event);
    } else if ("getContactListSuccess" == event_type){
        emit apiEvent((ApiEventGetContactListSuccess *)event);
    } else if ("conferenceCreated" == event_type){
        emit apiEvent((ApiEventConferenceCreated *)event);
    } else if ("conferenceJoin" == event_type){
        emit apiEvent((ApiEventConferenceJoin *)event);
    } else if ("conferenceMsg" == event_type){
        emit apiEvent((ApiEventConferenceMsg *)event);
    } else if ("conferenceInvited" == event_type){
        emit apiEvent((ApiEventConferenceInvited *)event);
    } else if ("updateContactList" == event_type){
        emit apiEvent((ApiEventUpdateContactList *)event);
    } else if ("updateItem" == event_type){
        emit apiEvent((ApiEventUpdateItem *)event);
    } else if ("searchUsersAnswer" == event_type){
        emit apiEvent((ApiEventSearchUsersAnswer *)event);
    } else if ("updateConference" == event_type){
        emit apiEvent((ApiEventUpdateConference *)event);
    } else if ("updateContact" == event_type){
        emit apiEvent((ApiEventUpdateContact *)event);
    } else if ("conferenceMsgDelivered" == event_type){
        emit apiEvent((ApiEventConferenceMsgDelivered *)event);
    } else {
        PERROR("cannot process event: '%s'\n",
            event_type.toStdString().c_str());
    }

out:
    return;
}

void Api::sendAck(
    const QString   &a_id)
{
    QString packet;
    QString id = getSessAckStr();

    if (a_id.size()){
        id = a_id;
    }

    packet += "<ipnoise";
    packet += " src_sessid=\""  + getSrcSessId() + "\"";
    packet += " dst_sessid=\""  + getDstSessId() + "\"";
    packet += " ack=\""         + id + "\"";
    packet += "/>";

    xmit(packet);
}

void Api::sendReset(
    const QString   &a_dst_sessid,
    const QString   &a_src_sessid)
{
    QString packet;
    packet += "<ipnoise";
    packet += " flags=\"reset\"";
    packet += " src_sessid=\"" + a_src_sessid + "\"";
    packet += " dst_sessid=\"" + a_dst_sessid + "\"";
    packet += "/>";
    xmit(packet);
}

void Api::processPacket(QByteArray &a_packet)
{
    int             i, res      = 0;
    QDomDocument    *dom        = new QDomDocument();
    QString         errorMsg    = "";
    int             errorLine   = 0;
    int             errorColumn = 0;

    QString     packet_dst_sessid;
    QString     packet_src_sessid;
    QString     packet_ack_str;
    uint32_t    packet_ack = 0;
    QString     packet_seq_str;
    uint32_t    packet_seq = 0;
    bool        ok = false;

    PWARN("[ API read ]\n"
        "    '%s'\n", a_packet.data());

    res = dom->setContent(
        a_packet,
        &errorMsg,
        &errorLine,
        &errorColumn
    );

    if (!res){
        PERROR("Cannot parse:    '%s'\n"
            "    errorMsg:    '%s'\n"
            "    errorLine:   '%d'\n"
            "    errorColumn: '%d'\n",
            a_packet.data(),
            errorMsg.toStdString().c_str(),
            errorLine,
            errorColumn
        );
        goto fail;
    }

    // check root element tag name
    {
        QList<QDomElement>  res_els;
        QDomElement         events_el;
        QDomElement         root_el = dom->documentElement();
        QString             root_tag_name = root_el.tagName();

        if ("ipnoise" != root_tag_name){
            PERROR("Unsupported root element's tag name: '%s'\n",
                root_tag_name.toStdString().c_str()
            );
            goto fail;
        }

        packet_dst_sessid = root_el.attribute("dst_sessid");
        packet_src_sessid = root_el.attribute("src_sessid");
        packet_ack_str    = root_el.attribute("ack");
        packet_ack        = packet_ack_str.toUInt(&ok, 16);
        packet_seq_str    = root_el.attribute("seq");
        packet_seq        = packet_seq_str.toUInt(&ok, 16);

        // maybe reset?
        if ("reset" == root_el.attribute("flags")){
            tcp_connect();
            goto out;
        }

        // check what session exist
        if (getSrcSessId() != packet_dst_sessid){
            // not our session, send reset
            goto reset;
        }

        // may be ack?
        if (    packet_ack_str.size()
            &&  not packet_seq_str.size())
        {
            // TODO send only 'packet_ack' package here
            goto out;
        }

        // check seq?
        if (not packet_seq_str.size()){
            goto reset;
        }

        // maybe init?
        if ("init" == root_el.attribute("flags")){
            // request next packet
            setSessAck(packet_seq + 1);
            sendAck();
            ApiEvent *event = getEvent("init");
            // call 'process' for setup sessid
            event->process(root_el);
            // emit event for others
            emit apiEvent((ApiEventInit *)event);
            goto out;
        }

        if (packet_seq < getSessAck()){
            // we already have received this packet
            PWARN("was received"
                " already received packet, ID: '%u'\n",
                packet_seq
            );
            // request next packet
            sendAck();
            goto out;
        } else if (packet_seq > getSessAck()){
            // packet from future
            PWARN("was received"
                " packet from future, ID: '%u'\n",
                packet_seq
            );
            // request next packet
            sendAck();
            goto out;
        }

        // request next packet
        setSessAck(packet_seq + 1);
        sendAck();

        // get 'events' element
        events_el = getElementByTagName(root_el, "events");
        if (events_el.isNull()){
            goto fail;
        }

        // get events
        res_els.clear();
        PWARN("search events\n");
        getElementsByTagName(events_el, "event", res_els);

        // process events
        for (i = 0; i < res_els.count(); i++){
            QDomElement event = res_els.at(i);
            emit apiEvent(event);
        }
    }

out:
    if (dom){
        delete dom;
        dom = NULL;
    }
    return;

fail:
    goto out;

reset:
    sendReset(
        packet_src_sessid,      // dst sess ID
        packet_dst_sessid       // src sess ID
    );
    goto out;
}

void Api::readCb()
{
    std::vector<std::string>              packets;
    std::vector<std::string>::iterator    packets_it;

    // store new data
    m_socket_buffer += m_socket->readAll();

    // get packets and update data
    {
        std::string tmp = m_socket_buffer.data();
        cutIPNoisePackets(tmp, packets);
        m_socket_buffer.setRawData(
            tmp.c_str(),
            tmp.size()
        );
    }

    for (packets_it = packets.begin();
        packets_it != packets.end();
        packets_it++)
    {
        QByteArray packet = packets_it->c_str();
        processPacket(packet);
    }
}

int Api::tcp_connect()
{
    int err = -1;

    if (m_socket){
        // already in progress
        m_socket->abort();
    }

    // reset seq, ack
    setSessAck("");
    setSessSeq(0);
    setDstSessId("");
    setSrcSessId(getRand(SESSID_SIZE).c_str());

    m_socket = new QTcpSocket(this);

    connect(
        m_socket,
        SIGNAL(connected()),
        this,
        SLOT(connectedCb())
    );

    connect(
        m_socket,
        SIGNAL(readyRead()),
        this,
        SLOT(readCb())
    );

    connect(
        m_socket,
        SIGNAL(error(QAbstractSocket::SocketError)),
        this,
        SLOT(errorCb(QAbstractSocket::SocketError))
    );

    m_socket->waitForConnected(1e3);

    do {
        QString     host;
        int         port = 0;
        QString     address;
        if (m_config->contains("address")){
            address = (*m_config)["address"];
        }
        Splitter    host_port(
            address.toStdString().c_str(), ":"
        );
        if (2 != host_port.size()){
            PERROR("Cannot parse: '%s'\n",
                address.toStdString().c_str());
            goto fail;
        }
        host = host_port[0].c_str();
        if (not host.size()){
            PERROR("Cannot connect to: '%s'\n",
                address.toStdString().c_str());
            goto fail;
        }
        port = atoi(host_port[1].c_str());
        if (port <= 0){
            PERROR("Cannot connect to: '%s'\n",
                address.toStdString().c_str());
            goto fail;
        }
        PDEBUG(5, "connecting to: '%s:%d'\n",
            host.toStdString().c_str(),
            port
        );
        m_socket->connectToHost(host, port);
    } while (0);

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

ApiContactInfo * Api::getContactInfo(
    int32_t a_contact_id)
{
    ApiContactInfo     *info = NULL;
    ApiContactsInfoIt  it;

    if (a_contact_id <= 0){
        PERROR("Invalid contact_id: '%d'"
            " at Api::getContactInfo\n",
            a_contact_id
        );
        goto fail;
    }

    it = m_contacts_info.find(a_contact_id);
    if (m_contacts_info.end() != it){
        info = it.value();
    }

out:
    return info;
fail:
    goto out;
}

ApiItemInfo * Api::getItemInfo(const QString &a_huid)
{
    ApiItemInfo     *info = NULL;
    ApiItemsInfoIt  it;

    if (!a_huid.size()){
        PERROR("Empty huid at Api::getItemInfo\n");
        goto fail;
    }

    it = m_items_info.find(a_huid);
    if (m_items_info.end() != it){
        info = it.value();
    }

out:
    return info;
fail:
    goto out;
}

void Api::deleteContactInfo(
    ApiContactInfo *a_info)
{
    ApiContactsInfoIt   contacts_info_it;
    int32_t             contact_id;

    if (not a_info){
        goto out;
    }

    contact_id = a_info->getContactId();
    contacts_info_it = m_contacts_info.find(contact_id);
    if (m_contacts_info.end() != contacts_info_it){
        m_contacts_info.erase(contacts_info_it);
        delete a_info;
    }

out:
    return;
}

void Api::deleteItemInfo(
    ApiItemInfo *a_info)
{
    ApiItemsInfoIt  items_info_it;
    QString         huid;

    if (not a_info){
        goto out;
    }

    huid = a_info->getHuid();
    items_info_it = m_items_info.find(huid);
    if (m_items_info.end() != items_info_it){
        m_items_info.erase(items_info_it);
        delete a_info;
    }

out:
    return;
}

ApiContactInfo * Api::getCreateContactInfo(
    int a_contact_id)
{
    ApiContactInfo *info = NULL;

    if (a_contact_id <= 0){
        PERROR("Invalid contact_id: '%d'"
            " at Api::getItemInfo\n",
            a_contact_id
        );
        goto fail;
    }

    info = getContactInfo(a_contact_id);
    if (!info){
        info = new ApiContactInfo(a_contact_id);
        m_contacts_info[a_contact_id] = info;
        connect(
            info, SIGNAL(changed(ApiContactInfo *)),
            this, SLOT(apiContactInfoChangedSlot(ApiContactInfo *))
        );
    }

out:
    return info;
fail:
    goto out;
}

ApiItemInfo * Api::getCreateItemInfo(
    const QString &a_huid)
{
    ApiItemInfo *info = NULL;

    if (!a_huid.size()){
        PERROR("Empty huid at Api::getItemInfo\n");
        goto fail;
    }

    info = getItemInfo(a_huid);
    if (!info){
        info = new ApiItemInfo(a_huid);
        m_items_info[a_huid] = info;
        connect(
            info, SIGNAL(changed(ApiItemInfo *)),
            this, SLOT(apiItemInfoChangedSlot(ApiItemInfo *))
        );
    }

out:
    return info;
fail:
    goto out;
}


void Api::apiItemInfoChangedSlot(
    ApiItemInfo *a_info)
{
    emit itemInfoChanged(a_info);
}

void Api::apiContactInfoChangedSlot(
    ApiContactInfo *a_info)
{
    emit itemContactChanged(a_info);
}

void Api::getElementsByTagName(
    QDomElement         &a_element,
    const QString       &a_tagname,
    QList<QDomElement>  &a_out)
{
    int i;

    QDomNodeList children = a_element.childNodes();
    for (i = 0; i <children.count(); i++){
        QDomNode node = children.at(i);
        if (a_tagname == node.nodeName()){
            a_out.push_back(node.toElement());
        }
    }
}

QDomElement Api::getElementByTagName(
    QDomElement         &a_element,
    const QString       &a_tagname)
{
    QDomElement         ret;
    QList<QDomElement>  res;

    getElementsByTagName(a_element, a_tagname, res);
    if (1 == res.count()){
        ret = res.at(0);
    } else if (1 < res.count()){
        PERROR("More that one '%s' element found\n",
            a_tagname.toStdString().c_str()
        );
        ret = res.at(0);
    }

    return ret;
}

QString Api::generateConfId()
{
    int     i;
    QString conf_id;
    char    buffer[128];
    pid_t   cur_pid;

    // clear buffer
    memset(buffer, 0x00, sizeof(buffer));

    // add prefix
    conf_id += "pipe_";

    // add pid
    cur_pid = getpid();
    snprintf(buffer, sizeof(buffer), "%d_", cur_pid);
    conf_id += buffer;

    // add rand
    for (i = 0; i < 16; i+= 2){
        int byte = int((double(rand())/RAND_MAX)*255);
        snprintf(buffer + i, sizeof(buffer) - i,
            "%2.2x", byte);
    }
    conf_id += buffer;

    return conf_id;
}

// --------------------------------------------------------

// return resource path if resource exist
QString Api::resourceImagePath(
    const QString &a_resource_id)
{
    QString path;
    path = resourcePath(
        RESOURCE_TYPE_IMAGE,
        a_resource_id
    );
    return path;
}

// return resource path if resource exist
QString Api::resourceSoundPath(
    const QString &a_resource_id)
{
    QString path;
    path = resourcePath(
        RESOURCE_TYPE_SOUND,
        a_resource_id
    );
    return path;
}

// return resource path if resource exist
QString Api::resourcePath(
    const ResourceType  &a_resource_type,
    const QString       &a_resource_id)
{
    QString ret;
    QString resource_type;
    QString theme = "default";
    Path    path;

    QList<QString>              formats;
    QList<QString>::iterator    formats_it;

    switch (a_resource_type){
        case (RESOURCE_TYPE_SOUND):
            resource_type = "sounds";
            formats.push_back("wav");
            break;
        case (RESOURCE_TYPE_IMAGE):
            resource_type = "images";
            formats.push_back("png");
            formats.push_back("jpg");
            break;

        default:
            PERROR("unsupported resource type: '%d'"
                " for resource ID: '%s'\n",
                a_resource_type,
                a_resource_id.toStdString().c_str()
            );
            goto fail;
    };

    for (formats_it = formats.begin();
        formats_it != formats.end();
        formats_it++)
    {
        QString tmp = "resources/"
            + theme + "/"
            + resource_type + "/"
            + a_resource_id + "." + *formats_it;

        path.setPathUnix(tmp.toStdString());
        if (path.isFile()){
            // we have found file
            break;
        }
    }

    if (not path.isFile()){
        // not found :(
        goto out;
    }

#if defined(WIN32) || defined(WIN64)
    {
        wstring wstr;
        wstr = path.absolutePath16();
        ret.setUtf16(
            (const ushort *)wstr.c_str(),
            wstr.size()
        );
    }
#else
    ret = path.absolutePath().c_str();
#endif

out:
    return ret;

fail:
    goto out;
}

void Api::playSound(
    const QString &a_resource_id)
{
    QString path;

    // search resource
    path = resourceSoundPath(a_resource_id);
    if (not path.size()){
        goto out;
    }

#if defined(WIN32) || defined(WIN64)
    {
        QString full_path = "file://" + path;

        PWARN("playing full_path: '%s'\n",
            full_path.toStdString().c_str()
        );

        PlaySound(
            (LPCSTR)full_path.toStdString().c_str(),
            NULL,
            SND_FILENAME | SND_ASYNC
        );
    }
#if 0
    {
        QString full_path = "file://" + path;

        PWARN("playing full_path: '%s'\n",
            full_path.toStdString().c_str()
        );

        // QString::fromUtf8("file:///tmp/привед_медвед/pipe/resources/default/sounds/new_msg.wav");
        QUrl url = QUrl(path);
        Phonon::MediaSource src(url);
        Phonon::MediaObject obj(this);
        obj.setCurrentSource(src);

        Phonon::AudioOutput audio(
            Phonon::MusicCategory,
            this
        );
        Phonon::createPath(&obj, &audio);

        obj.play();
    }
#endif

#else
    {
        QString cmd = "play " + path + " 2>&1 >/dev/null";
        PWARN("executing: '%s'\n",
            cmd.toStdString().c_str()
        );
        system(cmd.toStdString().c_str());
    }
#endif

out:
    return;
}


