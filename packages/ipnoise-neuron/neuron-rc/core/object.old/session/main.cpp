#include <algorithm>
#include "base64.hpp"
#include "fd_info.hpp"

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/bin/main.hpp"
#include "core/object/camera/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/openssl/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/template/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/users/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/websockFrame/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"
#include "db.hpp"
#include "db/mongo.hpp"

#include "core/object/session/main.hpp"

ObjectSession::ObjectSession(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectSession::~ObjectSession()
{
/*
    if (m_bev){
        bufferevent_disable(
            m_bev,
            EV_READ | EV_WRITE | EV_PERSIST
        );
        bufferevent_free(m_bev);
        m_bev = NULL;
    }
*/
}

void ObjectSession::do_init(
    const Object::CreateFlags   &a_flags)
{
//    PROP_HTTP_REQUEST(m_req);
//    if (!m_req){
//        PFATAL("cannot allocate memory\n");
//    }
}

string ObjectSession::getType()
{
    return ObjectSession::_getType();
}

void ObjectSession::do_init_props()
{
    ObjectInt32Sptr    prop_fd;

//    ObjectInt32Sptr    prop_websock_mask;
    ObjectStringSptr   prop_websock_accept;
//    ObjectStringSptr   prop_websock_pending_data;

    Object::do_init_props();

    PROP_INT32(prop_fd);
//    PROP_INT32(prop_websock_mask);
    PROP_STRING(prop_websock_accept);
//    PROP_STRING(prop_websock_pending_data);

    INIT_PROP(this, Fd, prop_fd);

//    INIT_PROP(this, WebsockMask,        prop_websock_mask);
    INIT_PROP(this, WebsockAccept,      prop_websock_accept);
//    INIT_PROP(this, WebsockPendingData, prop_websock_pending_data);
}

void ObjectSession::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("thread.io.read",  ObjectSession::actionIoRead);
    registerAction("thread.io.event", ObjectSession::actionIoEvent);
}

void ObjectSession::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("fd",   getFd);
//    SAVE_PROP("websock_mask",   getWebsockMask);
    SAVE_PROP("websock_accept", getWebsockAccept);

    Object::getAllProps(a_props);
}

int32_t ObjectSession::parseBSON(
    mongo::BSONObj  a_obj)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;

    // fd
    if (a_obj.hasField("fd")){
        ObjectInt32Sptr     prop_val;
        int32_t             val;

        val = a_obj.getIntField("fd");
        PROP_INT32(prop_val, val);

        INIT_PROP(this, Fd, prop_val);
    }

/*
    // websock mask
    if (a_obj.hasField("websock_mask")){
        ObjectInt32Sptr     prop_val;
        int32_t             val;

        val = a_obj.getIntField("websock_mask");
        PROP_INT32(prop_val, val);

        INIT_PROP(this, WebsockMask, prop_val);
    }
*/
    // websock accept
    if (a_obj.hasField("websock_accept")){
        ObjectStringSptr    prop_val;
        string              val;

        val = a_obj.getStringField("websock_accept");
        PROP_STRING(prop_val, val);

        INIT_PROP(this, WebsockAccept, prop_val);
    }
/*
    // websock pending data
    if (a_obj.hasField("websock_pending_data")){
        ObjectStringSptr    prop_val;
        string              val;

        val = a_obj.getStringField("websock_pending_data");
        PROP_STRING(prop_val, val);

        INIT_PROP(this, WebsockPendingData, prop_val);
    }
*/

    err = Object::parseBSON(a_obj);
    if (err){
        goto fail;
    }

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

/*
void ObjectSession::remove(
    const int32_t &a_force)
{
    ObjectVectorSptr    objects;
    ObjectVectorIt      objects_it;
    ObjectUniqueSptr    object_types;

    PROP_VECTOR(objects);
    PROP_UNIQUE(object_types);
    object_types->add("core.object.websock.frame");

    getNeighs(
        objects,
        "",             // name
        0,              // get removed
        1,              // db_lock
        object_types
    );

    for (objects_it = objects->begin();
        objects_it != objects->end();
        objects_it++)
    {
        ObjectWebsockFrameSptr frame;
        frame = dynamic_pointer_cast<ObjectWebsockFrame>(
            *objects_it
        );
        Object::remove(frame, a_force);
    }

    Object::remove(a_force);
}

int32_t ObjectSession::loop_after()
{
    int32_t             err = 0;
    ObjectContextSptr   context;
    string              session_id;

    context = Thread::getCurContext();
    if (context){
        session_id = context->getSessionId();
    }

    PDEBUG(10, "Thread http stopped\n"
        "  fd:          '%d'\n"
        "  session_id:  '%s'\n",
        m_fd,
        session_id.c_str()
    );
    return err;
}
*/

ObjectSptr  ObjectSession::getCurObject(
    ObjectMapSptr   a_req_props,
    ObjectMapSptr   a_req_params)
{
    ObjectSptr          cur_object;
    ObjectContextSptr   context;

    context    = Thread::getCurContext();
    cur_object = context->getCurUser();

    return cur_object;
}

/*
ObjectSessionSptr ObjectSession::createSession(
    ObjectHttpAnswerSptr    a_answer,
    ObjectUserSptr          a_user)
{
    ObjectSessionSptr   session;
    ObjectContextSptr   context;

    session = Object::createObject<ObjectSession>(
        "core.object.session"
    );

    // update current object
    session->setCurObject(a_user);

    // link objects (it will call save for both objects)
    session->addNeigh(a_user);

    // add set-cookie
    a_answer->setAnswerCookie(
        "sessid",
        session->getId()
    );

    return session;
}
*/
void ObjectSession::set302(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params,
    const string            &a_location,
    const string            &a_status_string)
{
    a_answer->setAnswerHeader("Location", a_location);
    setError(
        a_answer,
        a_req_props,
        a_req_params,
        302,
        a_status_string
    );
}

void ObjectSession::set404(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params,
    const string            &a_satus_string)
{
    setError(
        a_answer,
        a_req_props,
        a_req_params,
        404,
        a_satus_string
    );
}

void ObjectSession::set500(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params,
    const string            &a_satus_string)
{
    setError(
        a_answer,
        a_req_props,
        a_req_params,
        500,
        a_satus_string
    );
}

void ObjectSession::setError(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params,
    const int32_t           &a_status_code,
    const string            &a_satus_string)
{
    char buffer[512] = { 0x00 };

    a_answer->setAnswerStatusCode(a_status_code);
    a_answer->setAnswerStatusString(a_satus_string);

    // generate template name
    snprintf(buffer, sizeof(buffer),
        "%d",
        a_status_code
    );
    a_req_params->add("view", buffer);

    renderEmbedded(
        a_answer,
        a_req_props,
        a_req_params
    );
}

ObjectMapSptr ObjectSession::getParams(
    ObjectMapSptr a_req_props)
{
    ObjectSptr    tmp;
    ObjectMapSptr params;

    tmp    = a_req_props->get("params");
    params = dynamic_pointer_cast<ObjectMap>(tmp);

    return params;
}

ObjectMapSptr ObjectSession::getCreateParams(
    ObjectMapSptr a_req_props)
{
    ObjectMapSptr params;

    params = getParams(a_req_props);
    if (!params){
        PROP_MAP(params);
        a_req_props->add("params", params);
    }

    return params;
}

/*
void ObjectSession::processRequest(
    ObjectHttpAnswerSptr    a_answer,
    const ObjectMapSptr     a_req_props,
    const ObjectMapSptr     a_req_params)
{
//    int32_t             res = 0;
    ObjectSptr          cur_object;
    ObjectSptr          object_id_object;
    string              object_id;
    ObjectSptr          action_name_object;
    string              action_name;
    ThreadSptr          thread;
    ObjectContextSptr   context;

    PTIMING(0.0f, "processRequest '%s'\n",
        getId().c_str()
    );

    thread  = Thread::getCurThread();
    context = thread->contextGet();

    PDEBUG(10, "thread: '%s' (0x%llx)"
        " processRequest:\n"
        " a_req_props: '%s'\n"
        " a_req_params: '%s'\n",
        thread->getName().c_str(),
        thread->getAddr(),
        a_req_props->serialize().c_str(),
        a_req_params->serialize().c_str()
    );

    cur_object = getCurObject(
        a_req_props,
        a_req_params
    );
    if (!cur_object){
        set404(
            a_answer,
            a_req_props,
            a_req_params
        );
        goto out;
    }

    // search method
    action_name_object = a_req_props->get("method");
    if (action_name_object){
        action_name = action_name_object->toString();
    }
    if (!action_name.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr("missing 'method' name");
        goto out;
    }

    // search object ID
    object_id_object = a_req_props->get("object_id");
    if (object_id_object){
        object_id = object_id_object->toString();
    }
    if (object_id.size()){
        cur_object = context->getObjectById(object_id);
    }
    if (    !cur_object
        &&  "openSession"       != action_name
        &&  "getCreateCamera"   != action_name)
    {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr("object not found");
        goto out;
    }

    // before request will be processed
    if (cur_object){
        cur_object->beforeRequest(
            a_answer,
            a_req_props,
            a_req_params
        );
    }

    PDEBUG(40, "process action_name: '%s'\n",
        action_name.c_str()
    );

    if ("openSession" == action_name){
        ObjectMapSptr       answer_params;
        ObjectSptr          object_sessid;
        string              sessid;
        ObjectSessionSptr   session;
        ObjectContextSptr   context;
        ObjectUserSptr      user;

        context = Thread::getCurContext();
        user    = context->getUser();

        PDEBUG(40, "request open session"
            " based on session with ID: '%s' (0x%x)"
            " and user with ID: '%s' (0x%x)\n",
            sessid.c_str(),
            (uint32_t)session.get(),
            user ? user->getId().c_str() : "",
            (uint32_t)user.get()
        );

        sessid = getId();
        PDEBUG(40, "was created new session: '%s'\n",
            sessid.c_str()
        );

        // store session ID for answer
        a_answer->setAnswerApiSessid(sessid);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "session was opened successfully"
        );
    } else if ("getCreateCamera" == action_name){
        ObjectContextSptr   context;
        ObjectUserSptr      user;
        ObjectCameraSptr    camera;
        ObjectMapSptr       answer_params;

        context = Thread::getCurContext();
        user    = context->getUser();
        camera  = user->getCreateCamera();

        // prepare answer params
        answer_params = a_answer->getCreateAnswerParams();

        {
            string                  html;
            ObjectHttpAnswerSptr    tmp_answer;
            PROP_HTTP_ANSWER(tmp_answer);

            camera->renderEmbedded(
                tmp_answer,
                a_req_props,
                a_req_params
            );

            html = base64_encode(tmp_answer->getAnswerBody());
            answer_params->add("html", html);
        }

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr("all ok");
    } else if ("upload" == action_name){
        string          url, object_id;
        BoundariesSptr  boundaries;
        BoundariesIt    boundaries_it;

        // get boundaries
        boundaries = a_answer->getReqBoundaries();
        for (boundaries_it = boundaries->begin();
            boundaries_it != boundaries->end();
            boundaries_it++)
        {
            ObjectHttpRequestSptr boundary = *boundaries_it;
            string name     = boundary->getReqDispositionName();
            string filename = boundary->getReqDispositionFileName();

            if ("file" != name){
                continue;
            }

            PDEBUG(20, "store file:\n"
                "   name:     '%s'\n"
                "   filename: '%s'\n",
                boundary->getReqDispositionName().c_str(),
                boundary->getReqDispositionFileName().c_str()
            );

            do {
                FILE    *out        = NULL;
                char    buffer[512] = { 0x00 };
                string  data;
                string  object_id;

                object_id = cur_object->getId();
                snprintf(buffer, sizeof(buffer),
                    "static/upload/%s",
                    object_id.c_str()
                );

                out = fopen(buffer, "w");
                if (!out){
                    break;
                }
                data = boundary->getReqBody();
                PDEBUG(40, "write: '%d' byte(s)\n", int32_t(data.size()));
                fwrite(data.c_str(), data.size(), 1, out);
                fclose(out);
            } while (0);
        }

        // refresh current page
        object_id = cur_object->getId();
        url = "/api?object_id="+object_id;
        set302(
            a_answer,
            a_req_props,
            a_req_params,
            url
        );
    } else if ("logout" == action_name){
        actionLogout(this, a_answer, a_req_props, a_req_params);
    } else if ("login" == action_name){
        actionLogin(this, a_answer, a_req_props, a_req_params);
    } else if ("register" == action_name){
        actionRegister(this, a_answer, a_req_props, a_req_params);
    } else if ("addToBookmarks" == action_name){
        ObjectMapSptr     answer_params;
        ObjectMapSptr     redirect_info;

        PROP_MAP(redirect_info);

        // add cur object to bookmarks
        cur_object->addToBookmarks();

        answer_params = a_answer->getCreateAnswerParams();

        // refresh page
        redirect_info->add("object_id", cur_object->getId());
        answer_params->add("redirect",  redirect_info);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "neigh was unlinked successfully"
        );
    } else if ("bookmarkDel" == action_name){
        string          neigh_id;
        ObjectSptr        object_neigh_id;
        ObjectSptr      neigh;
        ObjectMapSptr     answer_params;
        ObjectMapSptr     redirect_info;

        PROP_MAP(redirect_info);

        if (!a_req_params){
            goto params_not_found;
        }

        object_neigh_id = a_req_params->get("neigh_id");
        if (object_neigh_id){
            neigh_id = object_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = context->getObjectById(neigh_id);
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerApiStatus("failed");
            a_answer->setAnswerApiDescr("object not found");
            goto out;
        }

        // del neigh from bookmarks
        neigh->delFromBookmarks();

        answer_params = a_answer->getCreateAnswerParams();

        // refresh page
        redirect_info->add("object_id", cur_object->getId());
        answer_params->add("redirect",  redirect_info);
        answer_params->add("neigh_id",  object_neigh_id);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "bookmark was deleted successfully"
        );
    } else if ("bookmarkAdd" == action_name){
        string          neigh_id;
        ObjectSptr      object_neigh_id;
        ObjectSptr      neigh;
        ObjectMapSptr   answer_params;
        ObjectMapSptr   redirect_info;

        PROP_MAP(redirect_info);

        if (!a_req_params){
            goto params_not_found;
        }

        object_neigh_id = a_req_params->get("neigh_id");
        if (object_neigh_id){
            neigh_id = object_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = context->getObjectById(neigh_id);
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerApiStatus("failed");
            a_answer->setAnswerApiDescr("object not found");
            goto out;
        }

        // add neigh to bookmarks
        neigh->addToBookmarks();

        answer_params = a_answer->getCreateAnswerParams();

        // refresh page
        redirect_info->add("object_id", cur_object->getId());
        answer_params->add("redirect",  redirect_info);
        answer_params->add("neigh_id",  object_neigh_id);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "bookmark was successfully added"
        );
    } else if ("neighAdd" == action_name){
        string          neigh_id;
        ObjectSptr      object_neigh_id;
        ObjectSptr      neigh;
        ObjectMapSptr   answer_params;
        ObjectMapSptr   redirect_info;

        PROP_MAP(redirect_info);

        if (!a_req_params){
            goto params_not_found;
        }

        object_neigh_id = a_req_params->get("neigh_id");
        if (object_neigh_id){
            neigh_id = object_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = context->getObjectById(neigh_id);
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerApiStatus("failed");
            a_answer->setAnswerApiDescr("object not found");
            goto out;
        }

        // add neigh
        cur_object->addNeigh(neigh);
        cur_object->save();

        answer_params = a_answer->getCreateAnswerParams();

        // refresh page
        redirect_info->add("object_id", cur_object->getId());
        answer_params->add("redirect",  redirect_info);
        answer_params->add("neigh_id",  object_neigh_id);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "neigh was added successfully"
        );
    } else if ("neighDel" == action_name){
        string          neigh_id;
        ObjectSptr      object_neigh_id;
        string          redirect;
        ObjectMapSptr   answer_params;
        ObjectMapSptr   redirect_info;
//        ObjectIdSptr    prop_id;

        PROP_MAP(redirect_info);

        if (!a_req_params){
            goto params_not_found;
        }

        object_neigh_id = a_req_params->get("neigh_id");
        if (object_neigh_id){
            neigh_id = object_neigh_id->toString();
        }
        if (!neigh_id.size()){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerApiStatus("failed");
            a_answer->setAnswerApiDescr(
                "missing or empty param 'neigh_id'"
            );
            goto out;
        }

        cur_object->neighDelete(neigh_id);

        answer_params = a_answer->getCreateAnswerParams();

        // refresh page
        redirect_info->add("object_id", cur_object->getId());
        answer_params->add("redirect",  redirect_info);

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("success");
        a_answer->setAnswerApiDescr(
            "neigh was deleted successfully"
        );
    } else {
        cur_object->action(
            a_answer,
            a_req_props,
            a_req_params
        );
    }

out:
    return;

params_not_found:
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("failed");
    a_answer->setAnswerApiDescr(
        "missing 'params' or it is not HASHREF"
    );
    goto out;
}
*/

int32_t ObjectSession::willRedirect(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    int32_t     need_redirect = 0;
    PWARN("FIXME\n");
/*
    ObjectSptr    object_object_id;
    string      object_id;
    string      req_method    = a_answer->getReqMethod();

    do {
        if ("GET" == req_method){
            object_object_id = a_req_props->get("object_id");
            if (!object_object_id){
                need_redirect = 1;
                break;
            }

            object_id = object_object_id->toString();
            if (!object_id.size()){
                need_redirect = 1;
                break;
            }
        }
    } while (0);

    if (need_redirect){
        ObjectSptr cur_object;
        cur_object = getCurObject(
            a_req_props,
            a_req_params
        );
        if (cur_object){
            // object exist
            string url = "/api?object_id="
                +cur_object->getId();
            set302(
                a_answer,
                a_req_props,
                a_req_params,
                url
            );
        } else {
            // object not found :( do not redirect,
            // will be 404 not found
            need_redirect = 0;
        }
    }
*/

    return need_redirect;
}

void ObjectSession::processPost(
    ObjectSessionSptr       a_session,
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    ObjectSptr          cur_object;
    ObjectVectorSptr    requests;
    ObjectVectorIt      requests_it;
    string              waiting_req_id;
    int32_t             requests_count = 0;
    ObjectMapSptr       answer_headers;

    PTIMING(0.010f, "processPost: '%s'\n",
        a_session->getId().c_str()
    );

    answer_headers = a_answer->getCreateAnswerHeaders();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("all ok");

    string json = a_answer->getReqBody();
    if (json.size()){
        Utils::chomp(json);
    }

    // try parse BSON
    if (json.size()){
        try {
            PDEBUG(40, "parse json: '%s'\n", json.c_str());
            mongo::BSONObj obj = mongo::fromjson(json.c_str());
            DbMongo::bsonToProps(
                obj,
                a_req_props
            );
        } catch (...) {
            PERROR("Cannot parse: '%s'\n",
                json.c_str()
            );
        }
    }

    if (a_req_props->has("requests")){
        requests = dynamic_pointer_cast<ObjectVector>(
            a_req_props->get("requests")
        );
    } else {
        // if requests not exist, create empty request
        // (it is params from url)
        ObjectMapSptr request;
        PROP_VECTOR(requests);
        PROP_MAP(request);
        requests->push_back(request);
        a_req_props->add("requests", requests);
    }

    for (requests_it = requests->begin();
        requests_it != requests->end();
        requests_it++)
    {
        requests_count++;

        ObjectHttpAnswerSptr    answer;
        ObjectMapSptr           answer_api;
        ObjectMapSptr           req_props;
        ObjectMapSptr           req_params;
        ObjectSptr              req_id_object;
        string                  req_id;
        ObjectSptr              api_method_object;
        string                  api_method;
        string                  cur_waiting_req_id;
        ObjectMapSptr           cur_answer_headers;
        ObjectMapIt             cur_answer_headers_it;

        PROP_HTTP_ANSWER(answer);
        answer_api = answer->getCreateAnswerApi();

        // get objects
        req_props  = dynamic_pointer_cast<ObjectMap>(
            *requests_it
        );

        // get params
        req_params = ObjectSession::getCreateParams(
            req_props
        );

        // add params from url (if not exist)
        a_answer->getReqApi(req_params);

        // search req_id
        req_id_object = req_props->get("req_id");
        if (req_id_object){
            req_id = req_id_object->toString();
            answer->setAnswerApiReqId(req_id);
        }

        // "answer fork" here, now we create in map 'answer'
        // (what are upper) -> 'answer_api'
        // new, local answer map with request ID as key

        a_answer->getCreateAnswerApi()->add(
            req_id,
            answer_api
        );

        // detect and setup context
 //       setupContext(
 //           answer,
 //           req_props,
 //           req_params
 //       );

        cur_object = a_session->getCurObject(
            req_props,
            req_params
        );
        if (!cur_object){
            answer->setAnswerApiStatus("failed");
            answer->setAnswerApiDescr("object not found");
            continue;
        }

        // search method
        api_method_object = req_props->get("method");
        if (api_method_object){
            api_method = api_method_object->toString();
        }

/*
        // process request
        processRequest(
            answer,
            req_props,
            req_params
        );
*/

        // copy some answer headers
        cur_answer_headers = answer->getCreateAnswerHeaders();
        for (cur_answer_headers_it = cur_answer_headers->begin();
            cur_answer_headers_it != cur_answer_headers->end();
            cur_answer_headers_it++)
        {
            ObjectSptr cur_key = cur_answer_headers_it->first;
            ObjectSptr cur_val = cur_answer_headers_it->second;
            string   key     = cur_key->toString();

            transform(
                key.begin(),
                key.end(),
                key.begin(),
                ::tolower
            );

            if ("set-cookie" == key){
                answer_headers->add(cur_key, cur_val);
            }
        }

        cur_waiting_req_id = answer->getWaitingReqId();
        if (cur_waiting_req_id.size()){
            waiting_req_id = cur_waiting_req_id;
        }
    }

    if (1 == requests_count){
        a_answer->setWaitingReqId(waiting_req_id);
    }

    // process other methods
    a_answer->setAnswerHeader(
        "Content-Type",
        "application/json; charset=utf-8"
    );

    return;
}

void ObjectSession::processGet(
    ObjectSessionSptr       a_session,
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    ObjectSptr          object_id;
    ObjectInt32Sptr     prop_embedded;
    int32_t             embedded    = 0;
    int32_t             res         = 0;
    ObjectSptr          cur_object;

    PTIMING(0.010f, "processGet: '%s'\n",
        a_session->getId().c_str()
    );

    // get params from url
    a_answer->getReqApi(a_req_props);

    // detect and setup context
 //   setupContext(
 //       a_answer,
 //       a_req_props,
 //       a_req_params
 //   );

    // process redirect if need
    res = a_session->willRedirect(
        a_answer,
        a_req_props,
        a_req_params
    );
    if (res){
        goto out;
    }

    // get current object
    cur_object = a_session->getCurObject(
        a_req_props,
        a_req_params
    );
    if (!cur_object){
        a_session->set404(
            a_answer,
            a_req_props,
            a_req_params
        );
        goto out;
    }

    // need embedded?
    prop_embedded = dynamic_pointer_cast<ObjectInt32>(
        a_req_params->get("embedded")
    );
    if (prop_embedded){
        embedded = prop_embedded->getVal();
    }

    // before request will be processed
    cur_object->beforeRequest(
        a_answer,
        a_req_props,
        a_req_params
    );

    // do render
    if (embedded){
        cur_object->renderEmbedded(
            a_answer,
            a_req_props,
            a_req_params
        );
    } else {
        cur_object->render(
            a_answer,
            a_req_props,
            a_req_params
        );
    }

out:
    return;
}

ObjectUserSptr ObjectSession::getUser(
    ObjectSessionSptr   a_session)
{
    ObjectUserSptr  user;

    if (!a_session){
        PFATAL("missing argument: 'a_session'\n");
    }

    user = Object::getNeigh<ObjectUser>(
        a_session,
        ""                  // name
        "core.object.user"  // type
    );

    return user;
}

/*
void ObjectSession::setupContext(
    ObjectHttpAnswerSptr    a_answer,
    ObjectMapSptr           a_req_props,
    ObjectMapSptr           a_req_params)
{
    ObjectUsersSptr     users;
    //ObjectShopSptr      shop;
    ObjectUserSptr      user;
    ObjectContextSptr   context;
//    ObjectSptr          sessid_object;
    string              sessid;
//    ObjectSessionSptr   session;
    int32_t             save_user  = 0;

    sessid  = getId();
    context = Thread::getCurContext();

    users = context->getCreateObjectById<ObjectUsers>(
        "static.users",
        "core.object.users"
    );

    //shop = context->getObjectById<ObjectShop>(
    //    "static.shop"
    //);

 //   // get session ID
 //   do {
 //       // from params (for openSession request)
 //       sessid_object = a_req_params->get("sessid");
 //       if (sessid_object){
 //           sessid = sessid_object->toString();
 //       }
 //       if (sessid.size()){
 //           break;
 //       }
 //
 //       // from objects (for generic requests)
 //       sessid_object = a_req_props->get("sessid");
 //       if (sessid_object){
 //           sessid = sessid_object->toString();
 //       }
 //       if (sessid.size()){
 //           break;
 //       }
 //
 //       // from cookie
 //       sessid_object = a_answer->getReqCookie("sessid");
 //       if (sessid_object){
 //           sessid = sessid_object->toString();
 //       }
 //       if (sessid.size()){
 //           break;
 //       }
 //   } while(0);
 //
 //   if (!sessid.empty()){
 //       session = ObjectSession::getSession(sessid);
 //   }
 //
 //   if (session){
 //       user = session->getUser();
 //   }
 //

    PFATAL("FIXME\n");
    //user = getUser();

    PDEBUG(50, "\nsession ID: '%s' user: 0x%x\n"
        "  a_req_props:     '%s'\n"
        "  a_req_params:    '%s'\n"
        "\n",
        sessid.c_str(),
        (uint32_t)user.get(),
        a_req_props->toString()->c_str(),
        a_req_params->toString()->c_str()
    );

    if (!user){
        // create new user and session
        user = context->createObject<ObjectUser>(
            "core.object.user"
        );
        save_user = 1;
    }

    // setup cur user
    context->setUser(user);

    if (save_user){
        // call method addNeigh only after setup context
        // because it must know user from context
        Object::addNeigh(users, user);
        addNeigh(user.get());
    }

 //   // create session
 //   if (!session){
 //       session = ObjectSession::createSession(
 //           a_answer,
 //           user
 //       );
 //       sessid = session->getId();
 //
 //       // setup default object
 //       // session->setCurObject(shop);
 //
 //       // add user to session
 //       user->addNeigh(session);
 //   }
 //
 //   // store session ID for answer
 //   a_answer->setAnswerApiSessid(sessid);

    // update context
    //context->setSession(session);
    //context->setFd(m_fd);
}

int32_t ObjectSession::sendAnswer(
    const int32_t &a_stop_thread)
{
    int32_t     ret = -1;
    PFATAL("FIXME\n");
    string      content;
    string      waiting_req_id;
    ThreadSptr  thread = Thread::getCurThread();

    // create content
    content = m_answer->toHttp();

    waiting_req_id = m_answer->getWaitingReqId();

    PDEBUG(100, "thread: '%s' (0x%llx)"
        " send answer\n"
        "  m_fd:            '%d'\n"
        "  waiting_req_id:  '%s'\n"
        "  content:         '%s'\n",
        thread->getName().c_str(),
        thread->getAddr(),
        m_fd,
        waiting_req_id.c_str(),
        content.c_str()
    );

    // send answer
    do {
        ret = ::write(
            m_fd,
            content.c_str(),
            content.size()
        );
    } while (0 > ret && EINTR == errno);

    m_answer = ObjectHttpAnswerSptr();

//out:
    if (a_stop_thread){
        stop();
    }
    return ret;
}

*/

void ObjectSession::setFd(
    const int32_t &a_fd)
{
    ObjectInt32Sptr prop_fd;
    PROP_INT32(prop_fd, a_fd);
    setFd(prop_fd);
}

/*
ObjectHttpAnswerSptr ObjectSession::getAnswer()
{
    return m_answer;
}
*/

void ObjectSession::processHttp(
    ObjectSessionSptr       a_session,
    ObjectHttpRequestSptr   a_req)
{
    int32_t res;

    ObjectHttpAnswerSptr    answer;
    string                  req_method;
    string                  req_body;
    ObjectMapSptr           req_props;
    ObjectMapSptr           req_params;
    ThreadSptr              thread;
    ObjectContextSptr       context;
//    string                  session_id;
    string                  waiting_req_id;
    string                  content;
    string                  object_id;
    int32_t                 fd = -1;

    PROP_HTTP_ANSWER(answer, a_req)

    fd          = a_session->getFd()->getVal();
    object_id   = a_session->getId();
    thread      = Thread::getCurThread();
    context     = thread->contextGet();

    PROP_MAP(req_props);
    PROP_MAP(req_params);

    req_method  = answer->getReqMethod();
    req_body    = answer->getReqBody();

    answer->setAnswerProto(answer->getReqProto());
    answer->setAnswerStatusCode(500);
    answer->setAnswerStatusString("Interal error");
    answer->setAnswerHeader("Connection", "close");
    answer->setAnswerHeader("Allow", "GET, POST");

    PWARN("req:\n'%s'\n",
        a_req->toString()->c_str()
    );

    // process
    if ("POST" == req_method){
        if (!answer->isReqWebsock()){
            ObjectSession::processPost(
                a_session,
                answer,
                req_props,
                req_params
            );
        }
    } else if ("GET" == req_method){
        if (!answer->isReqWebsock()){
            ObjectSession::processGet(
                a_session,
                answer,
                req_props,
                req_params
            );
        }
    } else {
        answer->setAnswerStatusCode(405);
        answer->setAnswerStatusString("Method Not Allowed");
    }

    // create content
    content = answer->toHttp();

    // send answer
    do {
        res = ::write(
            fd,
            content.c_str(),
            content.size()
        );
    } while (0 > res && EINTR == errno);

    if (answer->isReqWebsock()){
        ObjectStringSptr        prop_accept;
        string                  accept;
        ObjectWebsockFrameSptr  frame;

        accept = answer->getReqWebsockAccept();
        PROP_STRING(prop_accept, accept);

        a_session->setWebsockAccept(prop_accept);

        // get pending frame
        frame = Object::getCreateNeigh<ObjectWebsockFrame>(
            a_session,
            "core.object.websock.frame",    // type
            "pending",                      // name
            0,                              // get removed
            1                               // db lock
        );
        if (!frame){
            PFATAL("cannot create neigh\n");
        }

        // subscr frame for io
        thread->subscrIo(frame->getId(), fd);

        // unsubscr session from
        Object::save(a_session);
    } else {
        thread->close(fd);
        a_session->setFd(-1);
        Object::remove(a_session, 1);
    }

//    // get context
//    context     = Thread::getCurContext();
//    session_id  = context->getSessionId();

//    waiting_req_id = m_answer->getWaitingReqId();

//    Thread::resetCurContext();

  //  if (waiting_req_id.size()){
  //      // now it session's care
  //      FdInfoSptr  fd_info;
  //      // create new waiting
  //      fd_info = FdInfoSptr(new FdInfo(
  //          m_fd,
  //          _getName(),
  //          session_id,
  //          waiting_req_id
  //      ));
  //
  //      ObjectSession::eventsFdsAddTo(
  //          session_id,
  //          fd_info
  //      );
  //  } else {
  //      sendAnswer();
  //  }
  //
  //  ObjectSession::dump_fds_info_to_stderr();
}

int32_t ObjectSession::parseDataWebsock(
    ObjectSessionSptr   a_session,
    const string        &a_data)
{
    int32_t                 err = -1;
    ObjectStringSptr        websock_pending_data;
    ObjectWebsockFrameSptr  frame;
    ObjectActionSptr        action;
    ObjectBinSptr           prop_bin;

    PTIMING(0.030f, "session: '%s'\n",
        a_session->getId().c_str()
    );

    PROP_BIN(prop_bin, a_data);

    // get pending frame without db lock
    frame = Object::getCreateNeigh<ObjectWebsockFrame>(
        a_session,
        "core.object.websock.frame",    // type
        "pending",                      // name
        0,                              // get removed
        1                               // db lock
    );
    if (!frame){
        PFATAL("cannot create neigh\n");
    }

//    ObjectWebsockFrame::parse(frame, prop_bin);
//    Object::save(frame);


    // create action
    OBJECT_ACTION(action, "thread.io.read");
    action->addParam("data", prop_bin);
    // object not locked, so don't create backlink
    Object::addNeigh(action, frame, 0);
    action->scheduleCurThread();
    Object::save(action);

    // all ok
    err = 0;

    return err;
}

int32_t ObjectSession::parseDataHttp(
    ObjectSessionSptr   a_session,
    const string        &a_data)
{
    int32_t                 res, ret = -1;
    ObjectHttpRequestSptr   req;

    PTIMING(0.030f, "session: '%s'\n",
        a_session->getId().c_str()
    );

    PROP_HTTP_REQUEST(req);

    res = req->parseRequest(a_data);
    if (res){
        ret = res;
        goto out;
    }

    ObjectSession::processHttp(
        a_session,
        req
    );

    // all ok
    ret = 0;

out:
    return ret;
}

int32_t ObjectSession::parseData(
    ObjectSessionSptr   a_session,
    const string        &a_data)
{
    int32_t err = -1;

    PTIMING(0.030f, "session: '%s'\n",
        a_session->getId().c_str()
    );

    if (a_session->isWebsock()){
        err = ObjectSession::parseDataWebsock(
            a_session,
            a_data
        );
    } else {
        err = ObjectSession::parseDataHttp(
            a_session,
            a_data
        );
    }

    return err;
}

// ---------------- api ----------------

void ObjectSession::actionLogout(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectContextSptr   context;
    ObjectUserSptr      user;
    ObjectSessionSptr   session;
    ObjectSptr          cur_object;
    ObjectMapSptr       answer_params;
    ObjectMapSptr       redirect_info;

    context = Thread::getCurContext();

    PFATAL("FIXME\n");

    PROP_MAP(redirect_info);

    string status = "failed";
    string descr  = "logout failed";

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    context         = Thread::getCurContext();
    user            = context->getUser();
    session         = context->getSession();
    answer_params   = a_answer->getCreateAnswerParams();

    // get current object
    cur_object = session->getCurObject();

    // unlink session from all registered users
    session->unlinkFromRegisteredUsers();
    session->save();

    // remove session from context
    context->setSession(ObjectSessionSptr());

    // setup status
    status  = "success";
    descr   = "you are successfully unlogged";

    // refresh page
    redirect_info->add("object_id", cur_object->getId());
    answer_params->add("redirect",  redirect_info);

    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);
*/
    return;
}

//
// actionRegister
//
// context must have session
//
void ObjectSession::actionRegister(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectUserSptr      tmp;
    ObjectContextSptr   context     = Thread::getCurContext();
    ObjectUserSptr      user        = context->getUser();
    ObjectSessionSptr   session     = context->getSession();
    ObjectMapSptr       redirect_info;
    ObjectMapSptr       answer_params;

    ObjectParamSptr     param;
    ObjectSptr          cur_object;

    string              login;
    string              password1;
    string              password2;
    ObjectSptr          login_object;
    ObjectSptr          password1_object;
    ObjectSptr          password2_object;
    ObjectStringSptr    prop_login;
    ObjectStringSptr    prop_password;

    string      status = "failed";
    string      descr  = "cannot register new user";

    PROP_MAP(redirect_info);

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    answer_params = a_answer->getCreateAnswerParams();

    // get 'login'
    login_object = a_req_params->get("login");
    if (login_object){
        login = login_object->toString();
    }
    if (!login.size()){
        status  = "failed";
        descr   = "missing or empty 'login'";
        goto out;
    }

    // get 'password1'
    password1_object = a_req_params->get("password1");
    if (password1_object){
        password1 = password1_object->toString();
    }
    if (!password1.size()){
        status  = "failed";
        descr   = "missing or empty 'password1'";
        goto out;
    }

    // get 'password2'
    password2_object = a_req_params->get("password2");
    if (password2_object){
        password2 = password2_object->toString();
    }
    if (!password2.size()){
        status  = "failed";
        descr   = "missing or empty 'password2'";
        goto out;
    }

    // check passwords
    if (password1 != password2){
        status  = "failed";
        descr   = "passwords not equal";
        goto out;
    }

    // search user
    tmp = ObjectUsers::auth_user(login);
    if (tmp){
        status  = "failed";
        descr   = "user already exist";
        goto out;
    }

    param = user->getCreateParam<ObjectParam>("login");
    PROP_STRING(prop_login, login)
    param->setValue(prop_login));
    LINK_AND_SAVE(user, param);

    param = user->getCreateParam<ObjectParam>("password");
    PROP_STRING(prop_password, password1);
    param->setValue(prop_password);
    LINK_AND_SAVE(user, param);

    // get current object
    cur_object = session->getCurObject();
    if (!cur_object){
        cur_object = user;
        session->setCurObject(cur_object);
    }

    // add logged user to current session
    LINK_AND_SAVE(session, user);

    // setup status
    status  = "success";
    descr   = "user was successfully created";

    // refresh page
    redirect_info->add("object_id", cur_object->getId());
    answer_params->add("redirect",  redirect_info);

out:
    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);

    return;
*/
}

//
// actionLogin
//
// context must have session
//
void ObjectSession::actionLogin(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    ObjectContextSptr context       = Thread::getCurContext();
    ObjectSessionSptr session       = context->getSession();
    ObjectMapSptr     redirect_info;
    ObjectUserSptr    user;
    ObjectMapSptr     answer_params;

    ObjectSptr        login_object, password_object;
    string            login,        password;
    ObjectSptr        cur_object;

    PROP_MAP(redirect_info);

    string  status = "failed";
    string  descr  = "login failed";

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    // get 'login'
    login_object = a_req_params->get("login");
    if (login_object){
        login = login_object->toString();
    }
    if (!login.size()){
        status  = "failed";
        descr   = "missing or empty 'login'";
        goto out;
    }

    // get 'password'
    password_object = a_req_params->get("password");
    if (password_object){
        password = password_object->toString();
    }
    if (!password.size()){
        status  = "failed";
        descr   = "missing or empty 'password'";
        goto out;
    }

    // search user
    user = ObjectUsers::auth_user(login, password);
    if (!user){
        status  = "failed";
        descr   = "user not exist or invalid password";
        goto out;
    }

    // add logged user to current session
    LINK_AND_SAVE(session, user);

    // get current object
    cur_object = session->getCurObject();
    if (!cur_object){
        cur_object = user;
        session->setCurObject(cur_object);
    }

    // setup status
    status  = "success";
    descr   = "you are successfully logged";

    // refresh page
    redirect_info->add("object_id", cur_object->getId());
    answer_params->add("redirect",  redirect_info);

out:
    a_answer->setAnswerApiStatus(status);
    a_answer->setAnswerApiDescr(descr);

    return;
*/
}

bool ObjectSession::isWebsock()
{
    string accept = getWebsockAccept()->toString();
    return accept.empty() ? false : true;
}

// ---------------- static ----------------

void ObjectSession::actionIoRead(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ObjectInt32Sptr     prop_fd;
    ObjectBinSptr       prop_data;
    ObjectSessionSptr   session;
    ThreadSptr          cur_thread = Thread::getCurThread();
    ObjectContextSptr   context    = cur_thread->contextGet();
    int32_t             fd         = -1;
    string              data;
    string              object_id;

    PTIMING(0.030f, "session: '%s',"
        " process io.read action: '%s'\n",
        a_action->getId().c_str(),
        a_action->getId().c_str()
    )

    session   = dynamic_pointer_cast<ObjectSession>(a_object);
    object_id = session->getId();

    // get fd
    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getParam("fd")
    );
    fd = prop_fd->getVal();
    if (0 > fd){
        PFATAL("%s, 0 < fd\n"
            "  fd:          '%d'\n"
            "  a_action:    '%s'\n",
            object_id.c_str(),
            fd,
            a_action->toString()->c_str()
        );
    }

    // store fd
    session->setFd(fd);

    // get data
    prop_data = dynamic_pointer_cast<ObjectBin>(
        a_action->getParam("data")
    );
    data = prop_data->toString();

    // parse
    ObjectSession::parseData(session, data);

    // request initial thread close fd
  //  {
  //      ObjectActionSptr  action;
  //
  //      OBJECT_ACTION(action, "thread.io.close");
  //
  //      action->addParam("fd", fd);
  //
  //      // send action
  //      session->addAction(action);
  //      session->schedule();
  //      session->save();
  //  }

}

void ObjectSession::actionIoWrite(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    PWARN("new io write: '%s'\n",
        a_req_props->toString()->c_str()
    );
*/
}

void ObjectSession::actionIoEvent(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ObjectInt32Sptr     prop_fd;
    ThreadSptr          cur_thread = Thread::getCurThread();
    ObjectVectorSptr    flags_vector;
    ObjectUniqueSptr    flags;
    int32_t             fd = -1;

    flags_vector = dynamic_pointer_cast<ObjectVector>(
        a_action->getParam("flags")
    );
    PROP_UNIQUE(flags, flags_vector);

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getParam("fd")
    );
    fd = prop_fd->getVal();

    PDEBUG(10, "new io event, fd: '%d', flags: '%s'\n",
        fd,
        flags->toString()->c_str()
    );

    Object::remove(a_object, 1);
}

/*
FdInfoSptr ObjectSession::getFdInfoBySessId(
    const string &a_sessid)
{
    FdInfoSptr  ret;
    FdsInfoIt   it;

    g_fds_info_mutex.lock();
    it = g_fds_info.find(a_sessid);
    if (g_fds_info.end() != it){
        ret = it->second;
    }
    g_fds_info_mutex.unlock();

    return ret;
}

int32_t ObjectSession::isSessionOnline(
    const string &a_sessid)
{
    int32_t     ret  = 0;
    FdInfoSptr  info = getFdInfoBySessId(a_sessid);
    if (info){
        ret = info->isOnline();
    }
    PDEBUG(50, "info: 0x%llx, sessid: '%s', ret: '%d'\n",
        info ? (uint64_t)info.get() : 0,
        a_sessid.c_str(),
        ret
    );
    return ret;
}

void ObjectSession::dump_fds_info(
    string &a_out)
{
    FdsInfoIt   it;

    a_out = "dump of fds info:\n";

    g_fds_info_mutex.lock();
    if (!g_fds_info.size()){
        a_out += "empty list\n";
    }
    for (it = g_fds_info.begin();
        it != g_fds_info.end();
        it++)
    {
        FdInfoSptr  cur_fd_info = it->second;
        cur_fd_info->dump(a_out);
    }
    g_fds_info_mutex.unlock();
}

void ObjectSession::dump_fds_info_to_stderr()
{
    string dump;
    ObjectSession::dump_fds_info(dump);
    PWARN("%s", dump.c_str());
}

void ObjectSession::eventsFdsRemoveFrom(
    const string &a_sessid)
{
    g_fds_info_mutex.lock();
    g_fds_info.erase(a_sessid);
    g_fds_info_mutex.unlock();
}

void ObjectSession::eventsFdsAddTo(
    const string    &a_sessid,
    FdInfoSptr      a_info)
{
    g_fds_info_mutex.lock();
    g_fds_info[a_sessid] = a_info;
    g_fds_info_mutex.unlock();
}
*/

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
    _OBJECT_SESSION(object, 0);
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

