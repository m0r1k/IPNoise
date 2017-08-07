#include "core/neuron/session/main.hpp"

NeuronSession::NeuronSession()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronSession::~NeuronSession()
{
};

void NeuronSession::do_init()
{
    m_event_fd = -1;

    initType(PROP_NEURON_TYPE("core.neuron.session"));
    initCurNeuronId(PROP_NEURON_ID());
    initEvents(PROP_VECTOR());
}

int32_t NeuronSession::do_autorun()
{
    int32_t err = 0;
    return err;
}

NeuronUserSptr NeuronSession::unlinkFromRegisteredUsers()
{
    NeuronUserSptr                      user;
    vector<NeuronUserSptr>              users_registered;
    vector<NeuronUserSptr>              users;
    vector<NeuronUserSptr>::iterator    user_it;

    getNeighs<NeuronUser>(users);
    for (user_it = users.begin();
        user_it != users.end();
        user_it++)
    {
        NeuronUserSptr      cur_user = *user_it;
        NeuronParamSptr     login;
        NeuronParamSptr     password;
        login    = cur_user->getParam("login");
        password = cur_user->getParam("password");
        if (    login
            &&  password)
        {
            cur_user->neighDelete(this);
        }
    }

    if (1 < users_registered.size()){
        PERROR("more than 1 user"
            " was found for session: '%s'\n",
            getId()->toString().c_str()
        );
    }

    return user;
}

NeuronUserSptr NeuronSession::getUser()
{
    NeuronUserSptr                      user;
    vector<NeuronUserSptr>              users_unregistered;
    vector<NeuronUserSptr>              users_registered;
    vector<NeuronUserSptr>              users;
    vector<NeuronUserSptr>::iterator    user_it;

    getNeighs<NeuronUser>(users);
    for (user_it = users.begin();
        user_it != users.end();
        user_it++)
    {
        NeuronUserSptr      cur_user = *user_it;
        NeuronParamSptr     login;
        NeuronParamSptr     password;
        login    = cur_user->getParam("login");
        password = cur_user->getParam("password");
        if (    login
            &&  password)
        {
            users_registered.push_back(cur_user);
        } else {
            users_unregistered.push_back(cur_user);
        }
    }

    // get first unregistered user
    if (users_unregistered.size()){
        user = *(users_unregistered.begin());
    }

    // get first registered user
    if (users_registered.size()){
        user = *(users_registered.begin());
    }

    if (1 < users_registered.size()){
        PERROR("more than 1 user"
            " was found for session: '%s'\n",
            getId()->toString().c_str()
        );
    }

    return user;
}

void NeuronSession::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("cur_neuron_id",  getCurNeuronId);
    SAVE_PROP("events",         getEvents);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronSession::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  cur_neuron_id;

    // Cur neuron
    if (a_obj.hasField("cur_neuron_id")){
        cur_neuron_id = a_obj.getStringField("cur_neuron_id");
        initCurNeuronId(PropNeuronIdSptr(
            new PropNeuronId(cur_neuron_id)
        ));
    }

    // Events
    if (a_obj.hasField("events")){
        PropVectorSptr      events(new PropVector);
        mongo::BSONElement  el;
        mongo::BSONObj      events_obj;

        el = a_obj.getField("events");
        if (mongo::BSONType::Array != el.type()){
            PERROR("cannot parse 'events' attribute,"
                " object dump: '%s'\n",
                a_obj.toString().c_str()
            );
            goto fail;
        }

        el.Val(events_obj);
        getDbThread()->bsonToProps(
            events_obj,
            events
        );
        initEvents(events);
    }

    err = SUPER_CLASS::parseBSON(a_obj);

out:
  return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// -------------------------- generic ---------------------------------------80

void NeuronSession::setCurNeuron(
    NeuronSptr  a_neuron)
{
    setCurNeuronId(a_neuron->getId());
}

NeuronSptr NeuronSession::getCurNeuron()
{
    NeuronSptr          neuron;
    PropNeuronIdSptr    id;

    id = getCurNeuronId();
    if (id){
        neuron = getDbThread()->getNeuronById(id);
    }

    return neuron;
}

void NeuronSession::addEvent(
    PropMapSptr a_event)
{
    PropVectorSptr  events = getEvents();
    events->add(a_event);
    save();

    // attempt to flush events
    flushEvents();
}

void NeuronSession::eventSockExceptionSlot(
    int a_fd)
{
    PWARN("MORIK exception on fd: '%d'\n", a_fd);
}

void NeuronSession::setEventFd(
    int32_t a_fd)
{
    PWARN("MORIK session: '%s' store event fd: '%d'\n",
        getId()->toString().c_str(),
        a_fd
    );
    m_event_fd = a_fd;
}

int32_t NeuronSession::getEventFd()
{
    int32_t ret = -1;
    ret = m_event_fd;
    return ret;
}

void NeuronSession::flushEvents()
{
    PropVectorSptr  events;
    PropVectorIt    it;
    int32_t         res;
    string          events_str;
    string          ret;

    if (0 > m_event_fd){
        PWARN("session: '%s',"
            " m_event_fd: '%d'\n",
            getId()->toString().c_str(),
            m_event_fd
        );
        goto out;
    }

    m_event_socket_notifier = QSocketNotifierSptr(
        new QSocketNotifier(
            m_event_fd,
            QSocketNotifier::Exception
        )
    );

    events = getEvents();
    if (!events->size()){
        PWARN("session: '%s',"
            " empty events size\n",
            getId()->toString().c_str()
        );
        goto out;
    }

    ret += "HTTP/1.1 200 OK\n";
    ret += "Connection: close\n";
    ret += "Content-Type: application/json; charset=UTF-8\n";
    ret += "\n";
    ret += "{\n";
    ret += " \"descr\":  \"events was fetched successfully\",\n";
    ret += " \"status\": \"success\",\n";
    ret += " \"params\": {\n";
    ret += "     \"events\": "+events->serialize()+"\n";
    ret += "  }\n";
    ret += "}\n";

    res = ::write(m_event_fd, ret.c_str(), ret.size());
    PWARN("MORIK"
        " session: '%s'"
        " flush events to fd: '%d',"
        " res: '%d'\n",
        getId()->toString().c_str(),
        m_event_fd,
        res
    );
    m_event_socket_notifier->setEnabled(0);
    m_event_socket_notifier = QSocketNotifierSptr();
    ::close(m_event_fd);
    m_event_fd = -1;

    if (res >= 0){
        clearEvents();
    }

    PDEBUG(30, "was wrote"
        " to fd: '%d'"
        " text: '%s',"
        " '%d' bytes\n",
        m_event_fd,
        ret.c_str(),
        res
    );

out:
    return;
}

void NeuronSession::clearEvents()
{
    setEvents(PROP_VECTOR());
    save();
}

PropVectorSptr NeuronSession::getClearEvents()
{
    PropVectorSptr ret;
    ret = getEvents();
    clearEvents();
    return ret;
}

//void NeuronSession::closeEventSocket()
//{
//    int32_t fd = getEventFd();
//
//    PWARN("NeuronSession::closeEventSocket, fd: '%d'\n", fd);
//
//    if (m_event_socket){
//        m_event_socket->close();
//        m_event_socket = TcpSocketSptr();
//    }
//}

/*
void NeuronSession::setEventSocket(
    TcpSocketSptr a_socket)
{
    m_event_socket = a_socket;

    PWARN("NeuronSession::setEventSocket"
        " fd: '%d'\n",
        a_socket->socketDescriptor()
    );
}
*/

// ---------------- module ----------------

int32_t NeuronSession::init()
{
    int32_t err = 0;
    return err;
}

void NeuronSession::destroy()
{
}

NeuronSptr NeuronSession::object_create()
{
    NeuronSptr neuron(new NeuronSession);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.session",
    .init           = NeuronSession::init,
    .destroy        = NeuronSession::destroy,
    .object_create  = NeuronSession::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

