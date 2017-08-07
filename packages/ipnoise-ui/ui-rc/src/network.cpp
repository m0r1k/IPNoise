#include <stdio.h>
#include <dlfcn.h>

#include "thread/http.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/user/main.hpp"
#include "core/neuron/core/main.hpp"
#include "module.hpp"

#include "network.hpp"

Network::Network(int32_t &a_argc, char **a_argv)
#ifdef UI
    :   QApplication(a_argc, a_argv)
#else
    :   QCoreApplication(a_argc, a_argv)
#endif
{
    m_http_server   = NULL;
    m_state         = NETWORK_STATE_NOT_INITED;
    m_kladr_db      = NULL;
    m_timer         = NULL;

#ifdef UI
    m_ui_viewer     = NULL;
#endif
#ifdef HUMAN
    m_human_viewer  = NULL;
#endif
}

Network::~Network()
{
}

int32_t Network::doInit()
{
    int32_t         res, err = -1;
    NeuronGodSptr   god;
    ContextSptr     context;

    setState(Network::NETWORK_STATE_INIT);

    err = init_kladr_db();
    if (err){
        PWARN("cannot init kladr db\n");
    }

    res = startThreads();
    if (res){
        PERROR("cannot init threads\n");
        goto fail;
    }

    {
        PINFO("wait then all threads will be inited\n");
        unique_lock<mutex> lck(m_init_lock);
        while (Network::NETWORK_STATE_INIT_THREADS == m_state){
            // wait when all threads will be inited
            m_init_cv.wait(lck);
        }
        PINFO("all threads was inited\n");
    }

    // init modules
    res = initModules();
    if (res){
        PERROR("cannot init modules\n");
        goto fail;
    }

    PINFO("prepare start http server\n");

    // setup context
    god     = getCreateGodNeuron();
    context = ContextSptr(new Context(god));
    ThreadHttp::setContext(context);
    // call addNeigh method only after setup context,
    // because it will use user from context
    god->do_autorun();

    // get/create 'core' neuron
    getCreateCoreNeuron();

    // init http server
    res = initHttpServer();
    if (res){
        PERROR("Cannot init http server\n");
        goto fail;
    }

#ifdef UI
    {
        m_ui_viewer = new UiViewer(this);
        m_ui_viewer->setBackgroundColor(QColor(255, 0, 0));
        m_ui_viewer->show();
    }
#endif
#ifdef HUMAN
    {
        m_human_viewer = new HumanViewer(this);
    }
#endif

    // ok, now we are running
    setState(Network::NETWORK_STATE_RUNNING);

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// ---- CoreNeuron ----

NeuronCoreSptr Network::getCoreNeuron()
{
    NeuronCoreSptr  neuron;

    neuron = getNeuronById<NeuronCore>(
        PROP_NEURON_ID("core")
    );

    return neuron;
}

NeuronCoreSptr Network::getCreateCoreNeuron()
{
    NeuronCoreSptr neuron;

    neuron = getCoreNeuron();
    if (!neuron){
        // create new
        neuron = createNeuron<NeuronCore>();
        neuron->save();
    }

    return neuron;
}

NeuronGodSptr Network::getGodNeuron()
{
    NeuronGodSptr  neuron;

    neuron = getNeuronById<NeuronGod>(
        PROP_NEURON_ID("core.users.god"),
        PropNeuronTypeSptr(),
        0,  // a_get_removed
        0   // a_do_autorun
    );

    return neuron;
}

bool Network::isNeuronExist(
    PropNeuronIdSptr    a_id)
{
    bool ret = false;
    ret = getDbThread()->isNeuronExist(
        a_id
    );
    return ret;
}

NeuronGodSptr Network::getCreateGodNeuron()
{
    NeuronGodSptr neuron;

    neuron = getGodNeuron();
    if (!neuron){
        // create new (without autorun);
        neuron = createNeuron<NeuronGod>("", 0);
        neuron->save();
    }

    return neuron;
}

void Network::shutdown()
{
    if (Network::NETWORK_STATE_SHUTDOWN <= m_state){
        PERROR("attempt to double shutdown network, m_state: '%d'\n",
            m_state
        );
        goto fail;
    }

    setState(Network::NETWORK_STATE_SHUTDOWN);

    stopThreads();
    close_kladr_db();

    exit(1);

out:
    return;
fail:
    goto out;
}

void Network::threadStateChangedSlot(
    Thread              *a_thread,
    Thread::ThreadState a_state)
{
    string      name = a_thread->getName();
    ThreadsIt   m_threads_it;

    PDEBUG(30, "thread: '%s' state changed, '%d'\n",
        name.c_str(),
        a_state
    );

    if (Network::NETWORK_STATE_INIT_THREADS == getState()){
        int32_t all_threads_inited = 1;
        for (m_threads_it = m_threads.begin();
            m_threads_it != m_threads.end();
            m_threads_it++)
        {
            Thread *cur_thread = m_threads_it->second;
            Thread::ThreadState state = cur_thread->getState();

            if (   Thread::THREAD_STATE_NOT_INITED == state
                || Thread::THREAD_STATE_INIT       == state
                || Thread::THREAD_STATE_INITED     == state)
            {
                all_threads_inited = 0;
                break;
            }
        }

        if (all_threads_inited){
            setState(Network::NETWORK_STATE_INIT_AFTER_THREADS);
            m_init_cv.notify_all();
        }
    }

    return;
}

#ifdef UI
UiViewer * Network::getUiViewer()
{
    return m_ui_viewer;
}
#endif

#ifdef HUMAN
HumanViewer * Network::getHumanViewer()
{
    return m_human_viewer;
}
#endif

DbThread * Network::getDbThread()
{
    DbThread *db_thread = NULL;
    db_thread = (DbThread *)m_threads["db"];
    return db_thread;
}

/*
NeuronSptr  Network::getNeuronById(
    const PropNeuronIdSptr a_id)
{
    return getDbThread()->getNeuronById(a_id);
}
*/

int32_t Network::registerThread(Thread *a_thread)
{
    int32_t    err     = -1;
    string     name    = a_thread->getName();
    ThreadsIt  m_threads_it;

    m_threads_it = m_threads.find(name);
    if (m_threads.end() != m_threads_it){
        PERROR("thread: '%s' already registered\n",
            name.c_str()
        );
        goto fail;
    }

    // connect signals
//    connect(
//        a_thread,   SIGNAL(stateChanged(Thread *, Thread::ThreadState)),
//        this,       SLOT(threadStateChangedSlot(Thread *, Thread::ThreadState))
//    );

    // register thread
    m_threads[name] = a_thread;

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

Network::NetworkState Network::getState()
{
    return m_state;
}

void Network::timer()
{
    NeuronSessionSptr   session;
    NeuronUserSptr      user;
    PropMapSptr         event;

    PWARN("MORIK's timer\n");

    user = getDbThread()
        ->getNeuronById<NeuronUser>(
            PROP_NEURON_ID(
//            "e383dcb7bca1a7bb65dc2b98a9c911fb286bcade"
//            "3a729c1614521cd605c876b720ce908cf4da5327"
              "66e28d2d3e6de0c68c5425af939a174ec0bbb34e"
            )
        );

    event = PROP_MAP();
    event->add("object_id", "62bd660af0eb31a04c91a0baf4ef67f3775a25f7");
    event->add("type",      "rerender");

    user->addEvent(event);


/*
    neuron = getDbThread()
        ->getNeuronById(
            PROP_NEURON_ID("ab2491bf464eb39dd1213b3ae9570c1ad319a5fd")
        );

    session = dynamic_pointer_cast<NeuronSession>(neuron);
    if (!session){
        goto out;
    }

    {
*/
//    }

//out:
    return;
}

void Network::setState(const Network::NetworkState &a_state)
{
    if (m_state == a_state){
        // nothing changed
        goto out;
    }
    m_state = a_state;

    if (Network::NETWORK_STATE_RUNNING == m_state){
        PINFO("network started\n");
        if (0){
            m_timer = new QTimer();
            m_timer->setInterval(15e3);
            connect(
                m_timer, SIGNAL(timeout()),
                this,    SLOT(timer())
            );
            m_timer->start();
        }
    } else if (Network::NETWORK_STATE_SHUTDOWN == m_state){
        PINFO("shutdown..\n");
    }

out:
    return;
}

int32_t Network::initHttpServer()
{
    int32_t         res, err    = -1;
    QHostAddress    addr        = QHostAddress::Any;
#ifdef HUMAN
    int32_t         port        = HUMAN_HTTP_SERVER_PORT;
#else
    int32_t         port        = HTTP_SERVER_PORT;
#endif

    m_http_server = new HttpServer(this);
    // m_http_server->setMaxPendingConnections(0);

    res = m_http_server->listen(addr, port);
    if (!res){
        PERROR("Cannot listen '%s:%d'\n",
            addr.toString().toStdString().c_str(),
            port
        );
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
    if (m_http_server){
        delete m_http_server;
        m_http_server = NULL;
    }
    goto out;
}

void Network::stopThreads()
{
    ThreadsIt   m_threads_it;

    for (m_threads_it = m_threads.begin();
        m_threads_it != m_threads.end();
        m_threads_it++)
    {
        Thread *thread = m_threads_it->second;
        thread->setState(Thread::THREAD_STATE_SHUTDOWN);
    }
}

int32_t Network::startThreads()
{
    int32_t err = -1;

    if (Network::NETWORK_STATE_INIT != m_state){
        PERROR("attempt to double init network,"
            " m_state: '%d'\n",
            m_state
        );
        goto fail;
    }

    setState(Network::NETWORK_STATE_INIT_THREADS);

    // register DB thread
    REGISTER_THREAD("db", DbThread);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    shutdown();
    goto out;
}

int32_t Network::init_kladr_db()
{
    int32_t res;

    // lock db
    kladr_db_lock();

    if (m_kladr_db){
        // already inited
        goto out;
    }

    res = sqlite3_open(KLADR_PATH, &m_kladr_db);
    if (SQLITE_OK != res){
        PWARN("Can't open database: '%s' (%s)\n",
            KLADR_PATH,
            sqlite3_errmsg(m_kladr_db)
        );
    }

out:
    // unlock db
    kladr_db_unlock();

    return res;
}

void Network::close_kladr_db()
{
    // lock db
    kladr_db_lock();

    if (m_kladr_db){
        sqlite3_close(m_kladr_db);
        m_kladr_db = NULL;
    }

    // unlock db
    kladr_db_unlock();
}

void Network::kladr_db_lock()
{
    Thread *thread = (Thread *)QThread::currentThread();

#ifdef __x86_64
    PWARN("thread: 0x%8.8lx, try to lock kladr DB\n",
        (uint64_t)thread
    );
#else
    PWARN("thread: 0x%4.4x, try to lock kladr DB\n",
        (uint32_t)thread
    );
#endif

    m_kladr_db_mutex.lock();

#ifdef __x86_64
    PWARN("thread: 0x%8.8lx, kladr DB locked\n",
        (uint64_t)thread
    );
#else
    PWARN("thread: 0x%4.4x, kladr DB locked\n",
        (uint32_t)thread
    );
#endif
}

void Network::kladr_db_unlock()
{
    Thread *thread = (Thread *)QThread::currentThread();
#ifdef __x86_64
    PWARN("thread: 0x%8.8lx, unlock kladr DB\n",
        (uint64_t)thread
    );
#else
    PWARN("thread: 0x%4.4x, unlock kladr DB\n",
        (uint32_t)thread
    );
#endif
    m_kladr_db_mutex.unlock();
}

int32_t Network::processKladrSql(
    const   string  &a_sql,
    KladrCb         a_cb,
    KladrReqInfo    &a_kladr_req_info,
    string          &a_sql_err_msg)
{
    int32_t     ret             = -1;
    char        *sql_err_msg    = NULL;

    // lock db
    kladr_db_lock();

    PWARN("executing sql: '%s'\n", a_sql.c_str());

    ret = sqlite3_exec(
        m_kladr_db,
        a_sql.c_str(),
        a_cb,
        (void *)&a_kladr_req_info,
        &sql_err_msg
    );

    PWARN("kladr_req_info.m_results.size(): '%d'\n",
        (int32_t)a_kladr_req_info.m_results->size()
    );

    if (sql_err_msg){
        a_sql_err_msg = sql_err_msg;
    }

    if (sql_err_msg){
        sqlite3_free(sql_err_msg);
        sql_err_msg = NULL;
    }

    // unlock db
    kladr_db_unlock();

    return ret;
}

int32_t Network::register_module(
    const ModuleInfo &a_info)
{
    int32_t ret = 0;

//    PWARN("register module_name: '%s'\n",
//        a_info.type
//    );

    ModuleInfoSptr info(new ModuleInfo);
    *info = a_info;

    g_modules[a_info.type] = info;

    return ret;
}

void Network::getNeuronsInfo(
    PropMapSptr a_out)
{
    ModulesIt   it;

    for (it = g_modules.begin();
        it != g_modules.end();
        it++)
    {
        ModuleInfoSptr  info = it->second;
        string          type = info->type;

        // yet empty info (only type)
        (*a_out.get())[type] = PROP_MAP();
    }
}

int32_t Network::initModules()
{
    int32_t     err = -1;
    ModulesIt   it;

    PWARN("init modules:\n");
    for (it = g_modules.begin();
        it != g_modules.end();
        it++)
    {
        ModuleInfoSptr info = it->second;
        int32_t        res  = info->init();

        PWARN("  '%s': '%s'\n",
            info->type,
            res ? "FAILED" : "OK"
        );

        if (res){
            goto fail;
        }
    }
    PWARN("\n");

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 >= err){
        err = -1;
    }
    goto out;
}

void listModules()
{
    ModulesIt it;

    PWARN("modules list:\n");
    for (it = g_modules.begin();
        it != g_modules.end();
        it++)
    {
        ModuleInfoSptr info = it->second;

        PWARN("module name: '%s'\n",
            it->first.c_str()
        );
    }
    PWARN("\n");
}

