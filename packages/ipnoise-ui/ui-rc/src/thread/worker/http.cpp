#include "network.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/users/main.hpp"
#include "network.hpp"
#include "http_request.hpp"
#include "http_answer.hpp"
#include "thread/http.hpp"
#include "template.hpp"

#include "thread/worker/http.hpp"

extern Network *g_network;

static mutex mutex_http;

ThreadWorkerHttp::ThreadWorkerHttp(
    Network             *a_net,
    int32_t             a_fd,
    const string        &a_name,
    QObject             *a_parent)
    :   QObject(a_parent)
{
//    int32_t res, flags = 0;

    m_fd    = a_fd;
    m_net   = a_net;
    m_name  = a_name;

/*
    flags = fcntl(m_fd, F_GETFL, 0);
    if (-1 == flags){
        flags = 0;
        PERROR("cannot get socket flags for FD: '%d'\n",
            m_fd
        );
    }

    flags &= ~O_NONBLOCK;

    res = fcntl(m_fd, F_SETFL, flags);
    if (-1 == res){
        PERROR("cannot set socket flags for FD: '%d'\n",
            m_fd
        );
    }
*/

//    qRegisterMetaType<QAbstractSocket::SocketState>(
//        "QAbstractSocket::SocketState"
//    );
}

ThreadWorkerHttp::~ThreadWorkerHttp()
{
}

string ThreadWorkerHttp::getName()
{
    return m_name;
}

Network * ThreadWorkerHttp::getNetwork()
{
    return m_net;
}

/*
void ThreadWorkerHttp::processFd()
{
    int32_t         res;
    HttpRequestSptr req;

    // delete old socket
    if (m_socket){
        delete m_socket;
        m_socket = NULL;
    }
    // setup new
    m_socket = new TcpSocket();
    res      = m_socket->setSocketDescriptor(m_fd);
    if (!res){
        emit error(m_socket->error());
        goto fail;
    }

    req = HttpRequestSptr(new HttpRequest());
    processRequests(req);

out:
    return;
fail:
    goto out;
}
*/

NeuronSptr  ThreadWorkerHttp::getCurNeuron(
    HttpRequestSptr  a_req)
{
    NeuronSptr          cur_neuron;
    ContextSptr         context;
    NeuronSessionSptr   session;
    PropSptr            object_id;
    PropMapSptr         req_params(new PropMap);

    // get req params
    a_req->getReqApi(req_params);

    // get context
    context = ThreadHttp::getContext();
    session = context->getSession();

    // maybe from params?
    object_id = req_params->get("object_id");
    if (object_id){
        cur_neuron = getNetwork()
            ->getNeuronById(
                PROP_NEURON_ID(object_id->toString())
            );
        if (!cur_neuron){
            goto out;
        }
    }

    // update session or get from session
    if (session){
        if (cur_neuron){
            session->setCurNeuron(cur_neuron);
            session->save();
        } else {
            cur_neuron = session->getCurNeuron();
        }
    }

    if (!cur_neuron){
        // setup default neuron if not found
        cur_neuron = getNetwork()
            ->getNeuronById(PROP_NEURON_ID(
                "core.shop"
            ));
    }

out:
    return cur_neuron;
}

NeuronSessionSptr ThreadWorkerHttp::createSession(
    HttpAnswerSptr  a_answer,
    NeuronUserSptr  a_user)
{
    NeuronSessionSptr   session;
    session = g_network
        ->createNeuron<NeuronSession>();

    // update current neuron
    session->setCurNeuron(a_user);

    // link objects (it will call save for both objects)
    LINK_AND_SAVE(session, a_user);

    // add set-cookie
    a_answer->setAnswerCookie(
        "sessid",
        session->getId()->toString()
    );

    return session;
}

void ThreadWorkerHttp::set302(
    HttpAnswerSptr  a_answer,
    const string    &a_location,
    const string    &a_status_string)
{
    a_answer->setAnswerHeader("Location", a_location);
    setError(a_answer, 302, a_status_string);
}

void ThreadWorkerHttp::set404(
    HttpAnswerSptr  a_answer,
    const string    &a_satus_string)
{
    setError(a_answer, 404, a_satus_string);
}

void ThreadWorkerHttp::set500(
    HttpAnswerSptr  a_answer,
    const string    &a_satus_string)
{
    setError(a_answer, 500, a_satus_string);
}

void ThreadWorkerHttp::setError(
    HttpAnswerSptr  a_answer,
    const int32_t   &a_status_code,
    const string    &a_satus_string)
{
    char            buffer[65535] = { 0x00 };
    string          html;
    TemplateSptr    tpl;

    a_answer->setAnswerStatusCode(a_status_code);
    a_answer->setAnswerStatusString(a_satus_string);

    // generate template name
    snprintf(buffer, sizeof(buffer),
        "%s/errors/%d.tpl",
        TEMPLATES_DIR,
        a_status_code
    );

    tpl = TemplateSptr(new Template());
    tpl->parseFile(buffer, html);

    a_answer->setAnswerBody(html);
}

PropMapSptr ThreadWorkerHttp::getParams(
    HttpAnswerSptr  a_answer,
    PropMapSptr     a_req_api)
{
    PropSptr    param   = a_req_api->get("params");
    PropMapSptr params  = dynamic_pointer_cast<PropMap>(param);

    return params;
}

void ThreadWorkerHttp::apiLogout(
    HttpAnswerSptr a_answer)
{
    string              answer, redirect;
    ContextSptr         context;
    NeuronUserSptr      user;
    NeuronSessionSptr   session;
    NeuronSptr          cur_neuron;

    string status = "failed";
    string descr  = "logout failed";

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    context = ThreadHttp::getContext();
    user    = context->getUser();
    session = context->getSession();

    // get current neuron
    cur_neuron = session->getCurNeuron();

    // unlink session from all registered users
    session->unlinkFromRegisteredUsers();
    session->save();

    // remove session from context
    context->setSession(NeuronSessionSptr());

    // setup status
    status  = "success";
    descr   = "you are successfully unlogged";

    // refrash page
    redirect += "\"redirect\":";
    redirect += " {";
    redirect +=     " \"object_id\": " + cur_neuron->getId()->serialize();
    redirect += " }";

    answer += "{";
    answer +=   "\"status\": \"" + status   + "\",";
    answer +=   " \"descr\": \""  + descr    + "\",";
    answer +=   " \"params\": {"  + redirect + "}";
    answer += "}";

    a_answer->setAnswerBody(answer);

    return;
}

//
// apiRegister
//
// context must have session
//
void ThreadWorkerHttp::apiRegister(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronUserSptr      tmp;
    ContextSptr         context = ThreadHttp::getContext();
    NeuronUserSptr      user    = context->getUser();
    NeuronSessionSptr   session = context->getSession();
    string              answer,     redirect;
    PropMapSptr         params;
    PropSptr            params_prop;
    NeuronParamSptr     param;
    NeuronSptr          cur_neuron;

    string      login,      password1,      password2;
    PropSptr    login_prop, password1_prop, password2_prop;

    string      status = "failed";
    string      descr  = "cannot register new user";

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    // get 'params'
    params_prop = a_req_props->get("params");
    if (    !params_prop
        ||  Prop::PROP_TYPE_MAP != params_prop->getType())
    {
        status  = "failed";
        descr   = "missing 'params' or it is not hash";
        goto out;
    }
    params = dynamic_pointer_cast<PropMap>(params_prop);

    // get 'login'
    login_prop = params->get("login");
    if (login_prop){
        login = login_prop->toString();
    }
    if (!login.size()){
        status  = "failed";
        descr   = "missing or empty 'login'";
        goto out;
    }

    // get 'password1'
    password1_prop = params->get("password1");
    if (password1_prop){
        password1 = password1_prop->toString();
    }
    if (!password1.size()){
        status  = "failed";
        descr   = "missing or empty 'password1'";
        goto out;
    }

    // get 'password2'
    password2_prop = params->get("password2");
    if (password2_prop){
        password2 = password2_prop->toString();
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
    tmp = NeuronUsers::auth_user(login);
    if (tmp){
        status  = "failed";
        descr   = "user already exist";
        goto out;
    }

    // setup status
    status  = "success";
    descr   = "user was successfully created";

    param = user->getCreateParam<NeuronParam>("login");
    param->setValue(PROP_STRING(login));
    LINK_AND_SAVE(user, param);

    param = user->getCreateParam<NeuronParam>("password");
    param->setValue(PROP_STRING(password1));
    LINK_AND_SAVE(user, param);

    // get current neuron
    cur_neuron = session->getCurNeuron();
    if (!cur_neuron){
        cur_neuron = user;
        session->setCurNeuron(cur_neuron);
    }

    // add logged user to current session
    LINK_AND_SAVE(session, user);

    redirect += "\"redirect\":";
    redirect += " {";
    redirect +=     " \"object_id\": " + cur_neuron->getId()->serialize();
    redirect += " }";

out:
    answer += "{";
    answer +=   "\"status\": \"" + status   + "\",";
    answer +=   " \"descr\": \""  + descr    + "\",";
    answer +=   " \"params\": {"  + redirect + "}";
    answer += "}";

    a_answer->setAnswerBody(answer);
}

//
// apiLogin
//
// context must have session
//
void ThreadWorkerHttp::apiLogin(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronUserSptr      user;
    ContextSptr         context = ThreadHttp::getContext();
    NeuronSessionSptr   session = context->getSession();
    string              answer,     redirect;
    PropSptr            login_prop, password_prop;
    string              login,      password;
    PropMapSptr         params;
    PropSptr            params_prop;
    NeuronSptr          cur_neuron;

    string          status = "failed";
    string          descr  = "login failed";

    // setup default values
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    // get 'params'
    params_prop = a_req_props->get(
        PROP_STRING("params")
    );
    if (    !params_prop
        ||  Prop::PROP_TYPE_MAP != params_prop->getType())
    {
        status  = "failed";
        descr   = "missing 'params' or it is not hash";
        goto out;
    }
    params = dynamic_pointer_cast<PropMap>(params_prop);

    // get 'login'
    login_prop = params->get("login");
    if (login_prop){
        login = login_prop->toString();
    }
    if (!login.size()){
        status  = "failed";
        descr   = "missing or empty 'login'";
        goto out;
    }

    // get 'password'
    password_prop = params->get("password");
    if (password_prop){
        password = password_prop->toString();
    }
    if (!password.size()){
        status  = "failed";
        descr   = "missing or empty 'password'";
        goto out;
    }

    // search user
    user = NeuronUsers::auth_user(login, password);
    if (!user){
        status  = "failed";
        descr   = "user not exist or invalid password";
        goto out;
    }

    // add logged user to current session
    LINK_AND_SAVE(session, user);

    // get current neuron
    cur_neuron = session->getCurNeuron();
    if (!cur_neuron){
        cur_neuron = user;
        session->setCurNeuron(cur_neuron);
    }

    // setup status
    status  = "success";
    descr   = "you are successfully logged";

    redirect += "\"redirect\":";
    redirect += " {";
    redirect +=     " \"object_id\": " + cur_neuron->getId()->serialize();
    redirect += " }";

out:
    answer += "{";
    answer +=   "\"status\": \"" + status   + "\",";
    answer +=   " \"descr\": \""  + descr    + "\",";
    answer +=   " \"params\": {"  + redirect + "}";
    answer += "}";

    a_answer->setAnswerBody(answer);
    return;
}

void ThreadWorkerHttp::processRequest(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props,
    PropMapSptr         a_req_api)
{
    int32_t         res = 0;
    NeuronSptr      cur_neuron;
    PropMapSptr     params;
    PropSptr        object_id_prop;
    string          object_id;
    PropSptr        api_method_prop;
    string          api_method;
    PropSptr        api_req_id_prop;
    string          api_req_id;

    cur_neuron = getCurNeuron(a_answer);
    if (!cur_neuron){
        set404(a_answer);
        goto out;
    }

    // before request will be processed
    cur_neuron->beforeRequest(
        a_answer,
        a_req_api
    );

    // search req_id
    api_req_id_prop = a_req_api->get("req_id");
    if (api_req_id_prop){
        api_req_id = api_req_id_prop->toString();
        a_answer->setAnswerApiReqId(api_req_id);
    }

    // search neuron ID
    object_id_prop = a_req_api->get("object_id");
    if (object_id_prop){
        object_id = object_id_prop->toString();
    }
    if (object_id.size()){
        cur_neuron = getNetwork()->getNeuronById(
            PROP_NEURON_ID(object_id)
        );
    }
    if (!cur_neuron){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                " \"descr\": \"object not found\""
            "}"
        );
        goto out;
    }

    // search method
    api_method_prop = a_req_api->get("method");
    if (api_method_prop){
        api_method = api_method_prop->toString();
    }
    if (!api_method.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                " \"descr\": \"missing 'method'\""
            "}"
        );
        goto out;
    }

    // search params
    params = ThreadWorkerHttp::getParams(a_answer, a_req_api);

    if ("updateObject" == api_method){
        if (!params){
            goto params_not_found;
        }
        res = cur_neuron->apiObjectUpdate(a_answer, params);
        if (!res){
            string  redirect;
            string  object_id;
            object_id = cur_neuron->getId()->serialize();

            redirect += "\"redirect\":";
            redirect += " {";
            redirect +=     " \"object_id\": " + object_id;
            redirect += " }";

            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"success\","
                    " \"descr\": \"object was updated successfully\","
                    " \"params\": {"
                        "\"object_id\":" + object_id + ","
                        + redirect +
                    "}"
                "}"
            );
        }
    } else if ("upload" == api_method){
        string          url, object_id;
        BoundariesSptr  boundaries;
        BoundariesIt    boundaries_it;

        // get boundaries
        boundaries = a_answer->getReqBoundaries();
        for (boundaries_it = boundaries->begin();
            boundaries_it != boundaries->end();
            boundaries_it++)
        {
            HttpRequestSptr boundary = *boundaries_it;
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

                object_id = cur_neuron->getId()->toString();
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
        object_id = cur_neuron->getId()->toString();
        url = "/api?object_id="+object_id;
        set302(a_answer, url);
    } else if ("logout" == api_method){
        apiLogout(a_answer);
    } else if ("login" == api_method){
        apiLogin(a_answer, a_req_api);
    } else if ("register" == api_method){
        apiRegister(a_answer, a_req_api);
    } else if ("addToBookmarks" == api_method){
        string redirect, object_id;

        // add cur neuron to bookmarks
        cur_neuron->addToBookmarks();

        // prepare answer
        object_id = cur_neuron->getId()->serialize();

        redirect += "\"redirect\":";
        redirect += " {";
        redirect +=     " \"object_id\": " + object_id;
        redirect += " }";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"success\","
                " \"descr\": \"neigh was unlinked successfully\","
                " \"params\": {"
                    + redirect +
                "}"
            "}"
        );

    } else if ("createObject" == api_method){
        if (!params){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"failed\","
                    " \"descr\": \"missing 'params' or it is not HASHREF\""
                "}"
            );
            goto out;
        }
        cur_neuron->apiObjectCreate(a_answer, params);
    } else if ("bookmarkDel" == api_method){
        PropSptr    prop_neigh_id;
        NeuronSptr  neigh;
        string      neigh_id;
        string      redirect;
        string      object_id;

        if (!params){
            goto params_not_found;
        }

        prop_neigh_id = params->get("neigh_id");
        if (prop_neigh_id){
            neigh_id = prop_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = getNetwork()->getDbThread()->getNeuronById(
                PROP_NEURON_ID(neigh_id)
            );
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"failed\","
                    " \"descr\": \"object not found\""
                "}"
            );
            goto out;
        }

        // del neigh from bookmarks
        neigh->delFromBookmarks();

        // prepare answer
        object_id = cur_neuron->getId()->serialize();

        redirect += "\"redirect\":";
        redirect += " {";
        redirect +=     " \"object_id\": " + object_id;
        redirect += " }";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"success\","
                " \"descr\": \"bookmark was successfully deleted\","
                " \"params\": {"
                   "\"neigh_id\":" + prop_neigh_id->serialize() + ","
                    + redirect +
                "}"
            "}"
        );
    } else if ("bookmarkAdd" == api_method){
        PropSptr    prop_neigh_id;
        NeuronSptr  neigh;
        string      neigh_id;
        string      redirect;
        string      object_id;

        if (!params){
            goto params_not_found;
        }

        prop_neigh_id = params->get("neigh_id");
        if (prop_neigh_id){
            neigh_id = prop_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = getNetwork()->getDbThread()->getNeuronById(
                PROP_NEURON_ID(neigh_id)
            );
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"failed\","
                    " \"descr\": \"object not found\""
                "}"
            );
            goto out;
        }

        // add neigh to bookmarks
        neigh->addToBookmarks();

        // prepare answer
        object_id = cur_neuron->getId()->serialize();

        redirect += "\"redirect\":";
        redirect += " {";
        redirect +=     " \"object_id\": " + object_id;
        redirect += " }";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"success\","
                " \"descr\": \"bookmark was successfully added\","
                " \"params\": {"
                   "\"neigh_id\":" + prop_neigh_id->serialize() + ","
                    + redirect +
                "}"
            "}"
        );
    } else if ("neighAdd" == api_method){
        PropSptr    prop_neigh_id;
        NeuronSptr  neigh;
        string      neigh_id;
        string      redirect;
        string      object_id;

        if (!params){
            goto params_not_found;
        }

        prop_neigh_id = params->get("neigh_id");
        if (prop_neigh_id){
            neigh_id = prop_neigh_id->toString();
        }
        if (neigh_id.size()){
            neigh = getNetwork()->getDbThread()->getNeuronById(
                PROP_NEURON_ID(neigh_id)
            );
        }
        if (!neigh){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"failed\","
                    " \"descr\": \"object not found\""
                "}"
            );
            goto out;
        }

        // add neigh
        cur_neuron->addNeigh(neigh);
        cur_neuron->save();

        // prepare answer
        object_id = cur_neuron->getId()->serialize();

        redirect += "\"redirect\":";
        redirect += " {";
        redirect +=     " \"object_id\": " + object_id;
        redirect += " }";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"success\","
                " \"descr\": \"neigh was successfully added\","
                " \"params\": {"
                   "\"neigh_id\":" + prop_neigh_id->serialize() + ","
                    + redirect +
                "}"
            "}"
        );
    } else if ("neighDel" == api_method){
        PropSptr    prop_neigh_id;
        string      neigh_id;
        string      redirect;
        string      object_id;

        if (!params){
            goto params_not_found;
        }

        prop_neigh_id = params->get("neigh_id");
        if (prop_neigh_id){
            neigh_id = prop_neigh_id->toString();
        }
        if (!neigh_id.size()){
            a_answer->setAnswerStatusCode(200);
            a_answer->setAnswerStatusString("OK");
            a_answer->setAnswerBody(
                "{"
                    "\"status\": \"failed\","
                    " \"descr\": \"missing or empty param 'neigh_id'\""
                "}"
            );
            goto out;
        }
        cur_neuron->neighDelete(
            PROP_NEURON_ID(neigh_id)
        );

        object_id = cur_neuron->getId()->serialize();

        redirect += "\"redirect\":";
        redirect += " {";
        redirect +=     " \"object_id\": " + object_id;
        redirect += " }";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"success\","
                " \"descr\": \"neigh was deleted successfully\","
                " \"params\": {"
                   "\"neigh_id\":" + prop_neigh_id->serialize() + ","
                    + redirect +
                "}"
            "}"
        );
    } else {
        cur_neuron->api(
            a_answer,
            a_req_api
        );
    }

out:
    return;

params_not_found:
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerBody(
        "{"
            "\"status\": \"failed\","
            " \"descr\": \"missing 'params' or it is not HASHREF\""
        "}"
    );
    goto out;
}

void ThreadWorkerHttp::processPost(
    HttpAnswerSptr  a_answer,
    PropMapSptr     a_req_api)
{
    NeuronSptr      cur_neuron;
//    PropVectorSptr  api_requests;
//    PropVectorIt    api_requests_it;

    PropVectorSptr  requests;
    PropVectorIt    requests_it;

    // detect and setup context
    setupContext(a_answer, a_req_api);

    cur_neuron = getCurNeuron(a_answer);
    if (!cur_neuron){
        set404(a_answer);
        goto out;
    }

    // get requests
/*
    api_requests = dynamic_pointer_cast<PropVector>(
        a_req_api->get("requests")
    );
    if (!api_requests){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                " \"descr\": \"empty requests\""
            "}"
        );
        goto out;
    }
*/

    if (a_req_api->has("requests")){
        requests = dynamic_pointer_cast<PropVector>(
            a_req_api->get("requests")
        );
    } else {
        // if requests not exist, create empty request
        // (it is params from url)
        PropMapSptr request;
        requests = PROP_VECTOR();
        request  = PROP_MAP();
        requests->push_back(request);
        (*(a_req_api.get()))["requests"] = requests;
    }

    for (requests_it = requests->begin();
        requests_it != requests->end();
        requests_it++)
    {
        PropMapSptr req = dynamic_pointer_cast<PropMap>(
            *requests_it
        );

        // add params from url (if not exist)
        a_answer->getReqApi(req);

        processRequest(
            a_answer,
            a_req_api,
            req
        );
    }

out:
    // process other methods
    a_answer->setAnswerHeader(
        "Content-Type",
        "application/json; charset=utf-8"
    );
    return;
}

int32_t ThreadWorkerHttp::willRedirect(
    HttpAnswerSptr  a_answer,
    PropMapSptr     a_req_api)
{
    PropSptr    prop_object_id;
    string      object_id;
    int32_t     need_redirect = 0;
    string      req_method    = a_answer->getReqMethod();

    do {
        if ("GET" == req_method){
            if (!a_req_api){
                need_redirect = 1;
                break;
            }

            prop_object_id = a_req_api->get("object_id");
            if (!prop_object_id){
                need_redirect = 1;
                break;
            }

            object_id = prop_object_id->toString();
            if (!object_id.size()){
                need_redirect = 1;
                break;
            }
        }
    } while (0);

    if (need_redirect){
        NeuronSptr cur_neuron;
        cur_neuron = getCurNeuron(a_answer);
        if (cur_neuron){
            // neuron exist
            string url = "/api?object_id="
                +cur_neuron->getId()->toString();
            set302(a_answer, url);
        } else {
            // neuron not found :( do not redirect,
            // will be 404 not found
            need_redirect = 0;
        }
    }

    return need_redirect;
}

void ThreadWorkerHttp::processGet(
    HttpAnswerSptr  a_answer,
    PropMapSptr     a_req_api)
{
    PropSptr    object_id;
    PropSptr    prop_embedded;
    int32_t     embedded    = 0;
    int32_t     res         = 0;
    NeuronSptr  cur_neuron;

    // get params from url
    a_answer->getReqApi(a_req_api);

    // detect and setup context
    setupContext(a_answer, a_req_api);

    // process redirect if need
    res = willRedirect(
        a_answer,
        a_req_api
    );
    if (res){
        goto out;
    }

    // get current neuron
    cur_neuron = getCurNeuron(a_answer);
    if (!cur_neuron){
        set404(a_answer);
        goto out;
    }

    // need embedded?
    if (a_req_api){
        prop_embedded = a_req_api->get("embedded");
        if (prop_embedded){
            embedded = prop_embedded->toInt();
        }
    }

    // before request will be processed
    cur_neuron->beforeRequest(
        a_answer,
        a_req_api
    );

    // do render
    if (embedded){
        cur_neuron->renderEmbedded(
            a_answer,
            a_req_api
        );
    } else {
        cur_neuron->render(
            a_answer,
            a_req_api
        );
    }

out:
    return;
}

void ThreadWorkerHttp::setupContext(
    HttpAnswerSptr  a_answer,
    PropMapSptr     a_req_api)
{
    NeuronUsersSptr     users;
    NeuronShopSptr      shop;
    NeuronUserSptr      user;
    ContextSptr         context;
    PropSptr            sessid_prop;
    string              sessid;
    NeuronSessionSptr   session;
    DbThread            *db_thread = NULL;

    db_thread = getNetwork()->getDbThread();

    users = db_thread->getNeuronById<NeuronUsers>(
        PROP_NEURON_ID("core.users")
    );
    shop = db_thread->getNeuronById<NeuronShop>(
        PROP_NEURON_ID("core.shop")
    );

    // get session ID
    do {
        // from params
        sessid_prop = a_req_api->get("sessid");
        if (sessid_prop){
            sessid = sessid_prop->toString();
        }

        if (sessid.size()){
            break;
        }

        // from cookie
        sessid_prop = a_answer->getReqCookie("sessid");
        if (sessid_prop){
            sessid = sessid_prop->toString();
        }
        if (sessid.size()){
            break;
        }
    } while(0);

    if (sessid.size()){
        session = NeuronUsers::get_session(
            PROP_NEURON_ID(sessid)
        );
    }

    if (session){
        user = session->getUser();
    }

    if (!user){
        // create new user and session
        user = getNetwork()->createNeuron<NeuronUser>();
    }

    // create context
    context = ContextSptr(new Context(user));
    ThreadHttp::setContext(context);

    // call method addNeigh only after setup context
    // because it must know user from context
    LINK_AND_SAVE(users, user);

    // create session
    if (!session){
        session = ThreadWorkerHttp::createSession(
            a_answer,
            user
        );
        sessid = session->getId()->toString();
        // setup default neuron
        session->setCurNeuron(shop);
    }

    // add user to session
    LINK_AND_SAVE(user, session);

    // store session ID for answer
    a_answer->setAnswerApiSessid(sessid);

    // update context
    context->setSession(session);
    context->setFd(m_fd);
}

void ThreadWorkerHttp::processRequests(
    const HttpRequestSptr a_req)
{
    HttpAnswerSptr      answer(new HttpAnswer(*(a_req.get())));
    string              req_method  = answer->getReqMethod();
    string              req_body    = answer->getReqBody();
    PropMapSptr         req_api     = PROP_MAP();
    string              content;
    ContextSptr         context;
    NeuronSessionSptr   session;
    int32_t             res;

    answer->setAnswerProto(answer->getReqProto());
    answer->setAnswerStatusCode(500);
    answer->setAnswerStatusString("Interal error");
    answer->setAnswerHeader("Connection", "close");
    answer->setAnswerHeader("Allow", "GET, POST");

    // try parse BSON
    try {
        string json = answer->getReqBody();
        if (json.size()){
            Utils::chomp(json);
        }
        if (json.size()){
            PDEBUG(40, "parse json: '%s'\n", json.c_str());
            mongo::BSONObj obj = mongo::fromjson(json.c_str());
            DbThread::bsonToProps(
                obj,
                req_api
            );
        }
    } catch (...) {
        PERROR("Cannot parse: '%s'\n",
            answer->getReqBody().c_str()
        );
        set500(answer, "Cannot parse json");
        goto out;
    }

    // process
    if ("POST" == req_method){
        processPost(answer, req_api);
    } else if ("GET" == req_method){
        processGet(answer, req_api);
    } else {
        answer->setAnswerStatusCode(405);
        answer->setAnswerStatusString("Method Not Allowed");
    }

    // get context
    context = ThreadHttp::getContext();
    session = context->getSession();

out:

    // create content
    content = answer->toHttp();

    PDEBUG(40, "content: '%s'\n", content.c_str());

    PWARN("m_fd: '%d', needDisconnect(): '%d'\n",
        m_fd,
        answer->needDisconnect()
    );

    // remove context
    //resetContext();

    if (answer->needDisconnect()){
        // send answer
        do {
            res = ::write(
                m_fd,
                content.c_str(),
                content.size()
            );
        } while (0 > res && EINTR == errno);

        m_socket_notifier = QSocketNotifierSptr();
        ::close(m_fd);
        m_fd = -1;
    } else {
        // unsubscribe from events, now it session's care
        CHECKED_DISCONNECT(
            m_socket_notifier.get(),    SIGNAL(activated(int)),
            this,                       SLOT(readSlot(int))
        );
        m_socket_notifier->setEnabled(0);
        m_socket_notifier = QSocketNotifierSptr();

        session->setEventFd(m_fd);
        m_fd = -1;
    }
}

void ThreadWorkerHttp::stateChangedSlot(
    QAbstractSocket::SocketState a_state)
{
}

void ThreadWorkerHttp::readSlot(int)
{
    char        buffer[65535] = { 0x00 };
    int32_t     res = 0;
    QByteArray  req_buff;

    do {
        res = ::read(
            m_fd,
            buffer,
            sizeof(buffer)
        );
    } while (res < 0 && -EINTR == errno);

    if (0 < res){
        req_buff.append(buffer, res);
        res = m_req->parseRequest(req_buff);
        if (res){

//            mutex_http.lock();
            processRequests(m_req);
//            mutex_http.unlock();

//            m_socket_notifier->setEnabled(0);
//            m_socket_notifier = QSocketNotifierSptr();
//            ::close(m_fd);
//            m_fd = -1;

//            m_socket = TcpSocketSptr();
//            QThread::currentThread()->exit();
        }
    } else if (0 > res){
    } else {
        // unsubscribe from events, now it session's care
        CHECKED_DISCONNECT(
            m_socket_notifier.get(),    SIGNAL(activated(int)),
            this,                       SLOT(readSlot(int))
        );
        m_socket_notifier->setEnabled(0);
        m_socket_notifier = QSocketNotifierSptr();


//        m_socket_notifier->setEnabled(0);
//        m_socket_notifier = QSocketNotifierSptr();
//        ::close(m_fd);
//        m_fd = -1;

//        m_socket->close();
//        QThread::currentThread()->exit();
    }

//    req_buff = m_socket->read(1024*1024*1024);
//    req_buff = m_socket->readAll();
    return;
}

void ThreadWorkerHttp::exceptionSlot(
    int a_fd)
{
    PWARN("MORIK exception, a_fd: '%d'\n",
        a_fd
    );
}

void ThreadWorkerHttp::disconnectedSlot()
{
    PWARN("MORIK ThreadWorkerHttp::disconnectedSlot\n");
    //  m_socket = TcpSocketSptr();
    //  QThread::currentThread()->exit();
}

void ThreadWorkerHttp::processSlot()
{
    m_req             = HttpRequestSptr(new HttpRequest());
    m_socket_notifier = QSocketNotifierSptr(
        new QSocketNotifier(
            m_fd,
            QSocketNotifier::Read
        )
    );

    CHECKED_CONNECT(
        m_socket_notifier.get(),    SIGNAL(activated(int)),
        this,                       SLOT(readSlot(int))
    );

//    CHECKED_CONNECT(
//        m_socket.get(),  SIGNAL(disconnected()),
//        this,            SLOT(disconnectedSlot())
//    );

/*
    // delete old socket
    if (m_socket){
        delete m_socket;
        m_socket = NULL;
    }
    // setup new
    m_socket = new TcpSocket();
    m_socket->moveToThread(
        QThread::currentThread()
    );
    res = m_socket->setSocketDescriptor(m_fd);
    if (!res){
        emit error(m_socket->error());
        goto fail;
    }
*/




/*

    int32_t         res;
//    HttpRequestSptr req;

    // delete old socket
    if (m_socket){
        delete m_socket;
        m_socket = NULL;
    }
    // setup new
    m_socket = new TcpSocket();
    res      = m_socket->setSocketDescriptor(m_fd);
    if (!res){
        emit error(m_socket->error());
        goto fail;
    }

    req = HttpRequestSptr(new HttpRequest(m_socket));

    PWARN("MORIK was read:\n%s\n",
        req->dump().c_str()
    );

    processReq(req);

out:
    return;
fail:
    goto out;
*/
}

