#include "class.hpp"
DEFINE_CLASS(Network);

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <sqlite3.h>

#include <map>
#include <string>
#include <condition_variable>

#ifdef UI
    #include <QApplication>
    #include "ui/viewer.hpp"
#else
    #include <QCoreApplication>
#endif

#include <QTcpServer>
#include <QTimer>

#include "thread.hpp"
#include "thread/db.hpp"

#ifdef HUMAN
    #include "human/viewer.hpp"
#endif

#include "module.hpp"
#include "http_server.hpp"
//#include "neuron/neuron.hpp"
//#include "neuron/god.hpp"
//#include "neuron/shop.hpp"
//#include "neuron/core.hpp"

DEFINE_CLASS(NeuronCore);
DEFINE_CLASS(Neuron);
DEFINE_CLASS(NeuronGod);
DEFINE_CLASS(NeuronTrash);
DEFINE_CLASS(NeuronShop);
DEFINE_CLASS(NeuronUser);
DEFINE_CLASS(NeuronSession);
DEFINE_CLASS(NeuronParam);
DEFINE_CLASS(NeuronParamGroup);
DEFINE_CLASS(NeuronParamAddress);
DEFINE_CLASS(NeuronProducts);
DEFINE_CLASS(NeuronProduct);
DEFINE_CLASS(NeuronPhoto);
DEFINE_CLASS(NeuronBookmarks);
// dream land
DEFINE_CLASS(NeuronDreamland);
DEFINE_CLASS(NeuronDream);
DEFINE_CLASS(NeuronDreams);
DEFINE_CLASS(NeuronDreamWord);
// ipnoise
DEFINE_CLASS(NeuronIPNoise);

using namespace std;

typedef map<string, Thread *>   Threads;
typedef Threads::iterator       ThreadsIt;
typedef Threads::const_iterator ThreadsConstIt;

#define REGISTER_THREAD(name, class_name)           \
    do {                                            \
        int32_t res;                                \
        Thread  *thread = NULL;                     \
        thread = new class_name(                    \
            this,                                   \
            name                                    \
        );                                          \
        res = registerThread(thread);               \
        if (res){                                   \
            PERROR(                                 \
                "Cannot register thread: '%s'\n",   \
                name                                \
            );                                      \
            delete thread;                          \
            thread = NULL;                          \
            goto fail;                              \
        }                                           \
        thread->start();                            \
    } while (0);

#define KLADR_PATH "static/kladr.db"

#define HUMAN_HTTP_SERVER_PORT  8081
#define HTTP_SERVER_PORT        8080
#define HTTP_EVENT_SERVER_PORT  8079

class KladrReqInfo
{
    public:
        KladrReqInfo(){
            m_split_by_comma = 0;
        }
        PropVectorSptr  m_results;
        int32_t         m_split_by_comma;
};

typedef int32_t (*KladrCb)(
    void    *a_kladr_req_info,
    int32_t a_argc,
    char    **a_argv,
    char    **a_col_name
);

class Network
#ifdef UI
    :   public QApplication
#else
    :   public QCoreApplication
#endif
{
    Q_OBJECT

    public:
        Network(
            int32_t &a_argc,
            char    **a_argv
        );
        virtual ~Network();

        enum NetworkState {
            NETWORK_STATE_NOT_INITED = 0,
            NETWORK_STATE_INIT,
            NETWORK_STATE_INIT_THREADS,
            NETWORK_STATE_INIT_AFTER_THREADS,
            NETWORK_STATE_RUNNING,
            NETWORK_STATE_SHUTDOWN
        };

        int32_t                 doInit();
        NeuronCoreSptr          getCoreNeuron();
        NeuronCoreSptr          getCreateCoreNeuron();
        NeuronGodSptr           getGodNeuron();
        NeuronGodSptr           getCreateGodNeuron();
        void                    shutdown();
        void                    setState(
            const Network::NetworkState &
        );
        Network::NetworkState   getState();
        DbThread *              getDbThread();

        template <class T = Neuron>
        shared_ptr<T> getNeuronById(
            const PropNeuronIdSptr   a_id,
            const PropNeuronTypeSptr a_type = PropNeuronTypeSptr(),
            const int32_t            a_get_removed = 0,
            const int32_t            a_do_autorun  = 1)
        {
            shared_ptr<T> ret;
            ret = getDbThread()->getNeuronById<T>(
                a_id,
                a_type,
                a_get_removed,
                a_do_autorun
            );
            return ret;
        }

        int32_t processKladrSql(
            const   string  &a_sql,
            KladrCb         a_cb,
            KladrReqInfo    &a_kladr_req_info,
            string          &a_sql_err_msg
        );

        void threadStateChangedSlot(
            Thread *,
            Thread::ThreadState
        );

#ifdef UI
        UiViewer    * getUiViewer();
#endif

#ifdef HUMAN
        HumanViewer * getHumanViewer();
#endif

        bool isNeuronExist(
            PropNeuronIdSptr a_id
        );

        template <class T = Neuron>
        shared_ptr<T> createNeuron(
            const string    &a_type         = "",
            const int32_t   &a_do_autorun   = 1)
        {
            shared_ptr<T>               ret;
            T *                         fake = new T;
            ModuleInfoSptr              module_info;
            ModulesIt                   modules_it;
            vector<string>              types;
            vector<string>::iterator    types_it;
            string                      type;

            fake->deleteLater();

            type = fake->getType()->toString();

            types.push_back(a_type);
            types.push_back(type);
            types.push_back("core.neuron.unknown");

            for (types_it = types.begin();
                types_it != types.end();
                types_it++)
            {
                string type = *types_it;
                if (!type.size()){
                    continue;
                }

                modules_it = g_modules.find(type);
                if (g_modules.end() == modules_it){
                    PWARN("Unsupported neuron type: '%s'\n",
                        type.c_str()
                    );
                } else {
                    module_info = modules_it->second;
                    break;
                }
            }

            if (module_info){
                ret = dynamic_pointer_cast<T>(
                    module_info->object_create()
                );
            }

            if (!ret){
                PFATAL("cannot create neuron"
                    " with type: '%s'\n",
                    a_type.c_str()
                );
            }

            // rest dirty flag
            ret->setDirty(0);

            // run autorun
            if (a_do_autorun){
                ret->do_autorun();
            }

            return ret;
        };
/*
        template <class T = Neuron>
        shared_ptr<T> createNeuron()
        {
            shared_ptr<T>   ret;
            shared_ptr<T>   fake(new T);
            string          type;

            type = fake->getType()->toString();

            PWARN("createNeuron type: '%s'\n",
                type.c_str()
            );

            ret = createNeuronByType<T>(type);

            return ret;
        };
*/

        // modules
        static int32_t  register_module(const ModuleInfo &);
        void            getNeuronsInfo(PropMapSptr a_out);
        void            listModules();

    protected slots:
        void    timer();

    protected:
        void            kladr_db_lock();
        void            kladr_db_unlock();
        int32_t         init_kladr_db();
        void            close_kladr_db();

        int32_t         registerThread(Thread *);
        int32_t         startThreads();
        void            stopThreads();

        int32_t         initHttpServer();
        int32_t         initModules();

        HttpServer      *m_http_server;

    private:
        sqlite3         *m_kladr_db;
        mutex           m_kladr_db_mutex;

        Threads         m_threads;
        NetworkState    m_state;

        QTimer          *m_timer;

        mutex               m_init_lock;
        condition_variable  m_init_cv;

#ifdef UI
        UiViewer        *m_ui_viewer;
#endif
#ifdef HUMAN
        HumanViewer     *m_human_viewer;
#endif

};


#endif

