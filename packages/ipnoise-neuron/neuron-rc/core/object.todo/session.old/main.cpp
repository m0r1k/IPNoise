#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"

#include "core/object/session/main.hpp"

ObjectSession::ObjectSession(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectSession::~ObjectSession()
{
};

void ObjectSession::do_init(
    const Object::CreateFlags   &a_flags)
{
    //m_waiting_req_fd = -1;
}

string ObjectSession::getType()
{
    return ObjectSession::_getType();
}

void ObjectSession::do_init_props()
{
    Object::do_init_props();

    ObjectIdSptr        prop_cur_object_id;
    ObjectVectorSptr    prop_vector;

    PROP_ID(prop_cur_object_id);
    PROP_VECTOR(prop_vector);

    INIT_PROP(this, CurObjectId, prop_cur_object_id);
}

void ObjectSession::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectSession::do_autorun()
{
    int32_t err = 0;
    return err;
}

ObjectUserSptr ObjectSession::unlinkFromRegisteredUsers()
{
    ObjectUserSptr      user;
    ObjectVectorSptr    users_registered;
    ObjectVectorSptr    users;
    ObjectMapSptr       object_types;
    ObjectStringSptr    empty;
    ObjectVectorIt      user_it;
    int32_t             res;
    string              object_id = getId();

    PROP_VECTOR(users);
    PROP_VECTOR(users_registered);

    PROP_MAP(object_types);
    PROP_STRING(empty);
    object_types->add("core.object.user", empty);

    getNeighs(
        users,
        "",             // name
        0,              // removed
        object_types
    );

    for (user_it = users->begin();
        user_it != users->end();
        user_it++)
    {
        ObjectUserSptr      cur_user;
        ObjectParamSptr     login;
        ObjectParamSptr     password;

        cur_user = dynamic_pointer_cast<ObjectUser>(*user_it);
        login    = cur_user->getParam("login");
        password = cur_user->getParam("password");
        if (    login
            &&  password)
        {
            cur_user->neighDelete(object_id);
            users_registered->add(cur_user);
        }
    }

    res = users_registered->size();
    if (1 < res){
        PERROR("more than 1 user (%d)"
            " was found for session: '%s'\n",
            res,
            getId().c_str()
        );
    }

    return user;
}

ObjectUserSptr ObjectSession::getUser()
{
    ObjectUserSptr      user;
    ObjectVectorSptr    users;
    ObjectVectorSptr    users_registered;
    ObjectVectorSptr    users_unregistered;
    ObjectVectorIt      users_it;
    ObjectMapSptr       object_types;
    ObjectStringSptr    empty;
    int32_t             res;

    PROP_VECTOR(users);
    PROP_VECTOR(users_registered);
    PROP_VECTOR(users_unregistered);
    PROP_MAP(object_types);
    PROP_STRING(empty);

    object_types->add("core.object.user", empty);

    getNeighs(
        users,
        "",             // name
        0,              // get removed
        object_types
    );
    for (users_it = users->begin();
        users_it != users->end();
        users_it++)
    {
        ObjectUserSptr      cur_user;
        ObjectParamSptr     login;
        ObjectParamSptr     password;

        cur_user = dynamic_pointer_cast<ObjectUser>(*users_it);
        login    = cur_user->getParam("login");
        password = cur_user->getParam("password");
        if (    login
            &&  password)
        {
            users_registered->add(cur_user);
        } else {
            users_unregistered->add(cur_user);
        }
    }

    // get first unregistered user
    if (users_unregistered->size()){
        user = dynamic_pointer_cast<ObjectUser>(
            *(users_unregistered->begin())
        );
    }

    // get first registered user
    if (users_registered->size()){
        user = dynamic_pointer_cast<ObjectUser>(
            *(users_registered->begin())
        );
    }

    res = users_registered->size();
    if (1 < res){
        PERROR("more than 1 user (%d)"
            " was found for session: '%s'\n",
            res,
            getId().c_str()
        );
    }

    return user;
}

void ObjectSession::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("cur_object_id",  getCurObjectId);

    Object::getAllProps(a_props);
}

int32_t ObjectSession::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  cur_object_id;

    // Cur object
    if (a_obj.hasField("cur_object_id")){
        ObjectIdSptr prop_cur_object_id;

        cur_object_id = a_obj.getStringField("cur_object_id");
        PROP_ID(prop_cur_object_id, cur_object_id);

        INIT_PROP(this, CurObjectId, prop_cur_object_id);
    }

    err = Object::parseBSON(a_obj);

//out:
  return err;
//fail:
//    if (err >= 0){
//        err = -1;
//    }
//    goto out;
}

// -------------------------- generic ---------------------------------------80

void ObjectSession::setCurObject(
    ObjectSptr  a_object)
{
    setCurObjectId(a_object->getId());
}

ObjectSptr ObjectSession::getCurObject()
{
    ObjectSptr      object;

    PFATAL("FIXME\n");
/*
    ObjectIdSptr    id;

    id = getCurObjectId();
    if (id){
        object = ThreadDb::get()->getObjectById(id);
    }
*/
    return object;
}

int32_t ObjectSession::isOnline()
{
    int32_t ret = 0;
    PFATAL("FIXME\n");
/*
    string  session_id  = getId();
    ret = ThreadHttp::isSessionOnline(session_id);
*/
    return ret;
}

/*
void ObjectSession::setWaitingReqFd(
    int32_t a_req_fd)
{
    PWARN("MORIK session: '%s',"
        " set waiting req fd: '%d'\n",
        getId()->toString()->c_str(),
        a_req_fd
    );

    if (0 <= m_waiting_req_fd){
        ::close(m_waiting_req_fd);
        m_waiting_req_fd = -1;
    }

    m_waiting_req_fd = a_req_fd;
    if (0 > m_waiting_req_fd){
        goto out;
    }

    //m_waiting_req_notifier = QSocketNotifierSptr(
    //    new QSocketNotifier(
    //        m_waiting_req_fd,
    //        QSocketNotifier::Exception
    //    )
    //);

    //CHECKED_CONNECT(
    //    m_waiting_req_notifier.get(), SIGNAL(activated(int)),
    //    this,                         SLOT(waitingReqSlot(int))
    //);

out:
    return;
}

void ObjectSession::waitingReqSlot(
    int a_fd)
{
    PWARN("ObjectSession::waitingReqSlot fd: '%d'\n", a_fd);
}

void ObjectSession::setWaitingReqId(
    const string &a_req_id)
{
    PWARN("MORIK session: '%s' store request ID: '%s'\n",
        getId().c_str(),
        a_req_id.c_str()
    );
    m_waiting_req_id = a_req_id;
}

int32_t ObjectSession::getWaitingReqFd() const
{
    int32_t ret = -1;
    ret = m_waiting_req_fd;
    return ret;
}

string ObjectSession::getWaitingReqId() const
{
    return m_waiting_req_id;
}
*/

void ObjectSession::processEvents()
{
    PFATAL("FIXME\n");
/*
    ObjectVectorSptr        events;
    ObjectVectorIt          it;
    int32_t                 res;
    string                  events_str;
    string                  ret;

    ObjectHttpAnswerSptr    top_answer;
    ObjectMapSptr           top_answer_api;
    ObjectHttpAnswerSptr    answer(new HttpAnswer);
    ObjectMapSptr           answer_api;
    ObjectMapSptr           answer_params;
    string                  sessid = getId();

    ThreadHttpSptr          thread;
    int32_t                 fd = -1;

    FdInfoSptr              fd_info;
    string                  req_id;

    fd_info = ThreadHttp::getFdInfoBySessId(sessid);
    if (    !fd_info
        ||  !fd_info->isOnline())
    {
        goto out;
    }

    // remove from events fds
    ThreadHttp::eventsFdsRemoveFrom(sessid);

    fd      = fd_info->getFd();
    req_id  = fd_info->getReqId();
    thread  = dynamic_pointer_cast<ThreadHttp>(
        fd_info->getThread()
    );

    // get events
    events = getEvents();
    if (!events->size()){
        goto out;
    }

    if (!thread){
        PWARN("something wrong, thread not found\n");
        if (0 <= fd){
            ::close(fd);
        }
        goto out;
    }

    // get top answer
    top_answer = thread->getAnswer();
    if (!top_answer){
        goto out;
    }

    top_answer_api = top_answer->getCreateAnswerApi();
    top_answer->setAnswerHeader(
        "Content-Type",
        "application/json; charset=utf-8"
    );

    // create sub answer
    answer_api      = answer->getCreateAnswerApi();
    answer_params   = answer->getCreateAnswerParams();
    answer_params->add("events", events);
    answer->setAnswerApiReqId(req_id);
    answer->setAnswerApiStatus("success");
    answer->setAnswerApiDescr(
        "events was fetched successfully"
    );
    // add answer to top answer
    top_answer_api->add(req_id, answer_api);

    // send answer
    res = thread->sendAnswer();

    PDEBUG(50, "session: '%s'"
        " flush events to fd: '%d'"
        " wrote: '%d' byte(s)\n",
        sessid.c_str(),
        fd,
        res
    );

    if (0 <= res){
        clearEvents();
    }

    // stop waiting thread
    thread->stop();

    PDEBUG(30, "was wrote"
        " to fd: '%d'"
        " text: '%s',"
        " '%d' bytes\n",
        fd,
        ret.c_str(),
        res
    );

out:
    return;
*/
}

//void ObjectSession::closeEventSocket()
//{
//    int32_t fd = getEventFd();
//
//    PWARN("ObjectSession::closeEventSocket, fd: '%d'\n", fd);
//
//    if (m_event_socket){
//        m_event_socket->close();
//        m_event_socket = TcpSocketSptr();
//    }
//}

/*
void ObjectSession::setEventSocket(
    TcpSocketSptr a_socket)
{
    m_event_socket = a_socket;

    PWARN("ObjectSession::setEventSocket"
        " fd: '%d'\n",
        a_socket->socketDescriptor()
    );
}
*/

// ---------------- static ----------------

ObjectSessionSptr ObjectSession::getSession(
    const string    &a_sessid)
{
    ObjectSessionSptr   session;
    ObjectContextSptr   context = Thread::getCurContext();

    context->getObjectById<ObjectSession>(a_sessid);

    return session;
}

// ---------------- module ----------------

string ObjectSession::_getType()
{
    return "core.object.session";
}

string ObjectSession::_getTypePerl()
{
    return "core::object::session::main";
}

int32_t ObjectSession::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectSession::_destroy()
{
}

ObjectSptr ObjectSession::_object_create()
{
    ObjectSptr object;
    OBJECT_SESSION(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectSession::_getType,
    .type_perl      = ObjectSession::_getTypePerl,
    .init           = ObjectSession::_init,
    .destroy        = ObjectSession::_destroy,
    .object_create  = ObjectSession::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

