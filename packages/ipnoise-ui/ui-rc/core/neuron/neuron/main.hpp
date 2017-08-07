#include "class.hpp"
DEFINE_CLASS(Neuron);

#include "network.hpp"
extern Network *g_network;

#ifndef NEURON_HPP
#define NEURON_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>
#include <vector>
//#include <QObject>

#include <QKeyEvent>

#include "log.hpp"
#include "module.hpp"
#include "prop.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop/vector.hpp"
#include "prop/map.hpp"
#include "prop/neuron/link.hpp"
#include "prop/neuron/id.hpp"
#include "prop/neuron/type.hpp"
#include "prop/neuron/group.hpp"
#include "prop/neuron/acl.hpp"
#include "prop/time.hpp"
#include "prop/double.hpp"
#include "http_answer.hpp"
#include "thread/worker/db.hpp"
#include "template.hpp"

#define TEMPLATES_DIR   "templates/html/"
#define UPLOAD_DIR      "/static/upload/"

using namespace std;

DEFINE_CLASS(NeuronParam);
DEFINE_CLASS(NeuronParamGroups);

typedef map<string, NeuronParamSptr>    NParams;
typedef NParams::iterator               NParamsIt;
typedef NParams::const_iterator         NParamsConstIt;
typedef shared_ptr<NParams>             NParamsSptr;

typedef map <
    string,
    void (*)(
        Neuron *,
        HttpAnswerSptr,
        const PropMapSptr
    )
> ApiHandlers;
typedef ApiHandlers::iterator   ApiHandlersIt;

#define DEFINE_PROP(name, type)                             \
    private:                                                \
        type    m_##name;                                   \
    public:                                                 \
        void    set##name(                                  \
            const type      &a_val)                         \
        {                                                   \
            m_##name = a_val;                               \
        };                                                  \
        void    init##name(                                 \
            const type      &a_val)                         \
        {                                                   \
            set##name(a_val);                               \
            get##name()->setDirty(0);                       \
        };                                                  \
        type    get##name(){                                \
            return m_##name;                                \
        };

#define DEFINE_PARAM(name, type)                            \
    public:                                                 \
        template<typename T>                                \
        void set##name(T a_prop)                            \
        {                                                   \
            NeuronSptr param;                               \
            param = getNeigh<type>();                       \
            param->setValue(a_prop);                        \
        };                                                  \
        PropSptr get##name(){                               \
            NeuronSptr param;                               \
            param = getNeigh<type>();                       \
            return param->getValue();                       \
        };

#define SAVE_PROP(name, method)                             \
    do {                                                    \
        PropMapIt it;                                       \
        it = a_props->find(name);                           \
        if (a_props->end() != it){                          \
            /* already set */                               \
            break;                                          \
        }                                                   \
        (*(a_props.get()))[name] = method();                \
    } while (0);

#define LINK(a, b)                  \
    do {                            \
        a->addNeigh(b);             \
    } while (0);

#define LINK_AND_SAVE(a, b)         \
    do {                            \
        LINK(a, b);                 \
        a->save();                  \
        b->save();                  \
    } while (0);

#define LINK3(a, b, c)              \
    do {                            \
        a->addNeigh(b, 1, c);       \
    } while (0);

#define LINK_AND_SAVE3(a, b, c)     \
    do {                            \
        LINK3(a, b, c);             \
        a->save();                  \
        b->save();                  \
    } while (0);

class Neuron
#ifdef UI
    :   public QGLWidget
#else
    :   public QObject
#endif
{
    Q_OBJECT

    friend class DbThread;

    public:
        Neuron();
        virtual ~Neuron();

        DEFINE_PROP(Id,             PropNeuronIdSptr);
        DEFINE_PROP(Removed,        PropIntSptr); // TODO use time of delete here
        DEFINE_PROP(Type,           PropNeuronTypeSptr);
        DEFINE_PROP(TimeAccess,     PropTimeSptr);
        DEFINE_PROP(Name,           PropStringSptr);
        DEFINE_PROP(Title,          PropStringSptr);
        DEFINE_PROP(Descr,          PropStringSptr);
        DEFINE_PROP(Url,            PropStringSptr);
        DEFINE_PROP(NeighsInfo,     PropMapSptr);
        DEFINE_PROP(PosX,           PropDoubleSptr);
        DEFINE_PROP(PosY,           PropDoubleSptr);
        DEFINE_PROP(PosZ,           PropDoubleSptr);
#ifdef HUMAN
        DEFINE_PROP(HumanVal,       PropDoubleSptr);
        DEFINE_PROP(HumanNervePath, PropStringSptr);
#endif

//        DEFINE_PROP(Owner,          PropNeuronIdSptr);
//        DEFINE_PROP(Group,          PropNeuronGroupSptr);
//        DEFINE_PROP(Acl,            PropNeuronAclSptr);
//        DEFINE_PROP(NeighsBackups,  PropMapSptr);

        // neuron
        virtual void        getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t     save();
        virtual int32_t     saveParams();
        virtual int32_t     parseBSON(mongo::BSONObj);
        virtual int32_t     do_autorun();
        virtual void        keyPressEvent(
            QKeyEvent   *a_ev
        );

        // render
        virtual void    render(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual void    renderEmbedded(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual void    renderOpenGl(
            const int32_t   &a_with_name,
            const int32_t   &a_selected
        );

        // api
        virtual void    api(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual void    beforeRequest(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual int32_t apiObjectCreate(
            HttpAnswerSptr,
            const PropMapSptr
        );

        // api cur neuron
        static void apiLoad(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiDelete(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiNeighUnlink(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiRender(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiRenderEmbedded(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiGetEvents(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiGetNeighs(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiBookmark(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiUpdateEventsSubscr(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // generic
        Network     *           getNetwork();
        DbThread    *           getDbThread();

        UiViewer            *   getViewer();
        const QGLContext    *   getViewerContext();
        void                    setViewer(UiViewer *);

        void            setDirty(const int32_t &);
        bool            isRemoved();
        void            remove();
        void            registerApiMethod(
            const string            &,
            void (*a_cb)(
                Neuron *,
                HttpAnswerSptr,
                const PropMapSptr
            )
        );
        void            sendApiEventPropsChanged(
            PropMapSptr  a_props
        );
        string          generateId() const;
        int32_t         saveProps(PropMapSptr);
        void            serializeToPropMap(
            PropMapSptr,
            const int32_t   &a_serialize_params = 0
        );
        string          serialize(
            const string    &a_delim = ": ",
            const int32_t   &a_serialize_params = 0
        );
        string          toString();
        string          getView(
            const PropMapSptr a_params
        );
        TemplateSptr    getTemplate(
            const PropMapSptr a_params
        );

        void            addToBookmarks();
        void            delFromBookmarks();

        // neighs
        template <class T = PropNeuronLink>
        void    addNeigh(
            Neuron          *a_neuron,
            const int32_t   &a_create_backlink = 1,
            shared_ptr<T>   a_link = shared_ptr<T>(new T))
        {
            // create link to a_neuron
            do {
                PropMapSptr         links;
                string              link_type;

                // may be already exist?
                if (m_NeighsInfo->has(a_neuron->getId())){
                    // already exist
                    break;
                }

                a_link->addNeigh(a_neuron);

                link_type = a_link->getType();

                links = PROP_MAP();
                (*(links.get()))[PROP_STRING(link_type)] = a_link;

                m_NeighsInfo->add(a_neuron->getId(), links);

                PDEBUG(50, "process %s->addNeigh(%s)\n"
                    " a_link: '%s'\n"
                    " a_create_backlink: '%d'\n"
                    " link_type: '%s'\n"
                    " m_NeighsInfo: '%s'\n",
                    this->getId()->toString().c_str(),
                    a_neuron->getId()->toString().c_str(),
                    a_link->serialize().c_str(),
                    a_create_backlink,
                    link_type.c_str(),
                    m_NeighsInfo->serialize().c_str()
                );

            } while (0);

            // create backlink
            if (a_create_backlink){
                a_neuron->addNeigh<T>(
                    this,
                    0
                );
            }

            return;
        }

        template <class T = PropNeuronLink>
        void        addNeigh(
            NeuronSptr      a_neuron,
            const int32_t   &a_create_backlink = 1,
            shared_ptr<T>   a_link = PROP_NEURON_LINK())
        {
            addNeigh<T>(
                a_neuron.get(),
                a_create_backlink,
                a_link
            );
        }

        void        neighUnlink(
            PropNeuronIdSptr,
            const int32_t       &a_add_to_bookmarks = 1
        );
        void        neighDelete(PropNeuronIdSptr);
        void        neighDelete(Neuron *);

        // lock
        void lock(){
            string              id;
            PropNeuronIdSptr    prop_id = getId();

            if (prop_id.get()){
                id = prop_id->toString();
            }

#ifdef __x86_64
            PDEBUG(50, "neuron: 0x%lx, ID: '%s', try to lock\n",
                (uint64_t)this,
                id.c_str()
            );
#else
            PDEBUG(50, "neuron: 0x%x, ID: '%s', try to lock\n",
                (uint32_t)this,
                id.c_str()
            );
#endif

            m_mutex.lock();

#ifdef __x86_64
            PDEBUG(50, "neuron: 0x%lx, ID: '%s', locked\n",
                (uint64_t)this,
                getId()->toString().c_str()
            );
#else
            PDEBUG(50, "neuron: 0x%x, ID: '%s', locked\n",
                (uint32_t)this,
                getId()->toString().c_str()
            );
#endif
        }

        void unlock(){
#ifdef __x86_64
            PDEBUG(50, "neuron: 0x%lx, ID: '%s', unlock\n",
                (uint64_t)this,
                getId()->toString().c_str()
            );
#else
            PDEBUG(50, "neuron: 0x%x, ID: '%s', unlock\n",
                (uint32_t)this,
                getId()->toString().c_str()
            );
#endif
            m_mutex.unlock();
        }

// ------------------------------------

        /*
        template <class T>
        shared_ptr<T> getNeigh(
            const int32_t a_get_removed = 0)
        {
            shared_ptr<T>           ret;
            vector< shared_ptr<T> > res;
            getNeighs<T>(res);
            if (res.size()){
                shared_ptr<T> tmp;
                tmp = *(res.begin());
                if (    !tmp->isRemoved()
                    ||  a_get_removed)
                {
                    ret = tmp;
                }
                if (1 < res.size()){
                    PERROR("more than 1 neuron found for type: '%s',"
                        " neuron ID: '%s'\n",
                        tmp->getType()->toString().c_str(),
                        tmp->getId()->toString().c_str()
                    );
                }
            }
            return ret;
        }
        */

        // thread unsafe
        template <class T1 = Neuron>
        void _getNeighs(
            vector< shared_ptr<T1> >    &a_out,
            const string                &a_name = "",
            const int32_t               a_get_removed = 0,
            PropNeuronTypeSptr          a_neuron_type = PropNeuronTypeSptr(),
            PropMapSptr                 a_link_types  = PropMapSptr(),
            PropMapSptr                 a_skip = PropMapSptr())
        {
            PropMapSptr     neighs;
            PropMapIt       neighs_it;
            PropMapConstIt  skip_it;

            // PWARN("_getNeighs, neuron ID: '%s'\n",
            //    getId()->toString().c_str()
            // );

            neighs = getNeighsInfo();
            // TODO XXX соседи в neighs каждый раз в разном порядке,
            // нам нужно гарантировать порядок
            // а так как данные о соседях хранятся в хеше,
            // то нужно будет добавить свойство "следующий номер связи" к нейрону
            // а также добавить свойство "номер связи" к самой связи
            // таким образом при добавлении соседа, сосездается новая связь,
            // с номером хранящимся в свойстве "следующий номер связи"
            for (neighs_it = neighs->begin();
                neighs_it != neighs->end();
                neighs_it++)
            {
                string          neuron_id;
                shared_ptr<T1>  neuron;
                string          neuron_name;
                PropStringSptr  prop_neuron_name;
                PropMapSptr     links;
                PropMapIt       link_types_it;
                PropMapIt       links_it;

                neuron_id = neighs_it->first->toString();
                links     = dynamic_pointer_cast<PropMap>(
                    neighs_it->second
                );

                // PWARN("neuron_id: '%s'\n", neuron_id.c_str());

                if (a_skip){
                    skip_it = a_skip->find(neuron_id);
                    if (a_skip->end() != skip_it){
                        continue;
                    }
                }

                if (a_link_types){
                    int32_t found = 0;
                    // we have requested neighs with some link type
                    // check what we have such types
                    for (link_types_it = a_link_types->begin();
                        link_types_it != a_link_types->end();
                        link_types_it++)
                    {
                        PropStringSptr link_type;
                        link_type = dynamic_pointer_cast<PropString>(
                            link_types_it->first
                        );
                        if (!link_type){
                            PWARN("link type is not PropString,"
                                " it dump below:'%s'\n",
                                link_types_it->first->toString().c_str()
                            );
                            continue;
                        }
                        links_it = links->find(link_type);
                        if (links->end() != links_it){
                            // type found
                            found = 1;
                            break;
                        }
                    }
                    if (!found){
                        continue;
                    }
                }

                neuron = g_network->getNeuronById<T1>(
                    PROP_NEURON_ID(neuron_id),
                    a_neuron_type,
                    a_get_removed
                );
                if (!neuron){
                    // skip if type not for us
                    continue;
                }

                if (    neuron->isRemoved()
                    &&  !a_get_removed)
                {
                    // skip removed (as default)
                    continue;
                }

                // get and check neuron name
                if (a_name.size()){
                    prop_neuron_name = neuron->getName();
                    if (!prop_neuron_name){
                        continue;
                    }
                    neuron_name = prop_neuron_name->toString();
                    if (neuron_name != a_name){
                        continue;
                    }
                }

                // collect
                a_out.push_back(neuron);

                PDEBUG(50, "getNeighs,"
                    " neuron_name: '%s' (%s)"
                    " neuron_id: '%s'\n",
                    neuron_name.c_str(),
                    neuron ? "found" : "not found",
                    neuron_id.c_str()
                );
            }
        }

        template <class T1 = Neuron>
        void getNeighs(
            vector< shared_ptr<T1> >    &a_out,
            const string                &a_name = "",
            const int32_t               a_get_removed = 0,
            PropNeuronTypeSptr          a_neuron_type = PropNeuronTypeSptr(),
            PropMapSptr                 a_link_types  = PropMapSptr(),
            PropMapSptr                 a_skip = PropMapSptr())
        {
//            lock();
            _getNeighs<T1>(
                a_out,
                a_name,
                a_get_removed,
                a_neuron_type,
                a_link_types,
                a_skip
            );
//            unlock();
        }

        template <class T1 = Neuron>
        shared_ptr<T1> getNeigh(
            const string        &a_name       = "",
            const int32_t       a_get_removed = 0,
            PropNeuronTypeSptr  a_neuron_type = PropNeuronTypeSptr(),
            PropMapSptr         a_link_types  = PropMapSptr(),
            PropMapSptr         a_skip        = PropMapSptr())
        {
            shared_ptr<T1>           ret;
            vector< shared_ptr<T1> > res;

            getNeighs<T1>(
                res,
                a_name,
                a_get_removed,
                a_neuron_type,
                a_link_types,
                a_skip
            );

            if (res.size()){
                shared_ptr<T1> tmp;
                tmp = *(res.begin());
                if (    !tmp->isRemoved()
                    ||  a_get_removed)
                {
                    ret = tmp;
                }
                if (1 < res.size()){
                    PERROR("more than 1 neuron found for name: '%s',"
                        " neuron ID: '%s'\n",
                        tmp->getName()->toString().c_str(),
                        tmp->getId()->toString().c_str()
                    );
                }
            }
            return ret;
        }

        template <class T = Neuron>
        shared_ptr<T> createNeigh(
            const string    &a_name = "")
        {
            shared_ptr<T> ret;

            ret = g_network->createNeuron<T>();
            if (a_name.size()){
                ret->setName(PROP_STRING(a_name));
            }
            LINK_AND_SAVE(this, ret);

            return ret;
        }

        template <class T = Neuron>
        shared_ptr<T> getCreateNeigh(
            const string    &a_name       = "",
            const int32_t   a_get_removed = 0)
        {
            shared_ptr<T>   ret;
            shared_ptr<T>   fake(new T);
            string          type;

            type = fake->getType()->toString();
            ret  = getNeigh<T>(
                a_name,
                a_get_removed
            );

            if (!ret){
                ret = createNeigh<T>(a_name);
                if (!ret){
                    PFATAL("cannot create neigh"
                        " with type: '%s',"
                        " name:'%s',"
                        " get_removed: '%d'\n",
                        type.c_str(),
                        a_name.c_str(),
                        a_get_removed
                    );
                }
            }
            return ret;
        }

        /*
        template <class T>
        void getNeighs(
            vector< shared_ptr<T> > &a_out,
            const int32_t           a_get_removed = 0)
        {
            lock();

            PropMapSptr neighs = getNeighs();
            PropMapIt   it;

            for (it = neighs->begin();
                it != neighs->end();
                it++)
            {
                string          neuron_id;
                shared_ptr<T>   neuron;

                neuron_id = it->first->toString();
                neuron    = dynamic_pointer_cast<T>(
                    getDbThread()->getNeuronById(
                        PROP_NEURON_ID(neuron_id)
                    )
                );

                if (!neuron){
                    // skip if type not for us
                    continue;
                }

                if (    neuron->isRemoved()
                    &&  !a_get_removed)
                {
                    // skip removed (as default)
                    continue;
                }

                // collect
                a_out.push_back(neuron);

                PDEBUG(50, "getNeighs, neuron_id: '%s' (%s)\n",
                    neuron_id.c_str(),
                    neuron ? "found" : "not found"
                );
            }

            unlock();
        }

        template <class T = Neuron>
        shared_ptr <T>  createNeigh(){
            shared_ptr <T> neigh;
            neigh = getDbThread()->createNeuron<T>();
            LINK_AND_SAVE(neigh, this);
            return neigh;
        }

        template <class T = Neuron>
        shared_ptr <T>  getCreateNeigh(){
            shared_ptr <T> neigh;
            neigh = getNeigh<T>();
            if (!neigh){
                neigh = createNeigh<T>();
            }
            return neigh;
        }
        */


// ------------------------------------

        void    getParams(NParamsSptr);

        template <class T = NeuronParam>
        void getParams(
            const string                &a_name,
            vector< shared_ptr <T> >    &a_out,
            const int32_t               a_get_removed = 0)
        {
            NeuronParamSptr     param;
            NParamsSptr         params(new NParams);
            NParamsConstIt      params_it;

            // request all params
            getParams(params);

//            lock();

            for (params_it = params->begin();
                params_it != params->end();
                params_it++)
            {
                shared_ptr <T>  neuron;
                if (a_name != params_it->first){
                    continue;
                }
                neuron = dynamic_pointer_cast<T>(
                    params_it->second
                );
                if (!neuron){
                    continue;
                }
                if (    neuron->isRemoved()
                    &&  !a_get_removed)
                {
                    continue;
                }
                a_out.push_back(neuron);
            }

//            unlock();
        }

        // param
        template <class T = NeuronParam>
        shared_ptr <T> getParam(
            const string    &a_name,
            const int32_t   a_get_removed = 0)
        {
            shared_ptr <T>              ret;
            vector< shared_ptr <T> >    res;
            ssize_t                     size = 0;

            getParams(a_name, res);
            size = res.size();
            if (size >= 1){
                shared_ptr <T> tmp;
                tmp = *(res.begin());
                if (    !tmp->isRemoved()
                    ||  a_get_removed)
                {
                    ret = tmp;
                }
            }
            if (size > 1){
                PWARN("more than one param found"
                    " for name: '%s', neuron ID: '%s'\n",
                    a_name.c_str(),
                    getId()->toString().c_str()
                );
            }
            return ret;
        }

        template <class T = NeuronParam>
        shared_ptr <T>  createParam(const string &a_name){
            shared_ptr <T> param;
            param = g_network->createNeuron<T>();
            param->setName(PROP_STRING(a_name));
            LINK(param, this);
            return param;
        }

        template <class T = NeuronParam>
        shared_ptr <T>  getCreateParam(const string &a_name){
            shared_ptr <T> param;
            param = getParam<T>(a_name);
            if (!param){
                param = createParam<T>(a_name);
            }
            return param;
        }

#ifdef HUMAN
        void setHumanVal(
            const double &a_val
        );
        void setPosX(
            const double &
        );
        void setPosY(
            const double &
        );
        void setPosZ(
            const double &
        );
        void setPos(
            const double &a_x,
            const double &a_y,
            const double &a_z
        );
#endif

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

//        DbThread        *m_db_thread;
        int32_t         m_is_new;
        int32_t         m_is_dirty;
        mutex           m_mutex;

    protected:
        UiViewer        *m_viewer;

    private:
        void            do_init();
        ApiHandlers     m_handlers;
};

#endif

