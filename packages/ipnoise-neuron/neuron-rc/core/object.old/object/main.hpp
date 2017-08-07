#include "class.hpp"
DEFINE_CLASS(Object);

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>

// avoid conflict stl with perl
#undef seed
#undef do_open
#undef do_close

#include <mongo/client/dbclient.h>
#include <ipnoise-common/log_common.h>
#include "utils.hpp"

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

#define TEMPLATES_DIR       "core/templates/"
#define TEMPLATES_DIR_HTML  "core/templates/html/"
#define TEMPLATES_DIR_WEBGL "core/templates/webgl/"
#define UPLOAD_DIR          "/static/upload/"

// don't forget update
// core/object/object/main.cpp
// core/object/object/main.hpp
// Makefile
//
// and don't forget add
// #include "core/object/context/main.hpp"
// in your module main.cpp file

DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectBasket);
DEFINE_CLASS(ObjectCamera);
DEFINE_CLASS(ObjectDialog);
DEFINE_CLASS(ObjectDialogs);
DEFINE_CLASS(ObjectDouble);
DEFINE_CLASS(ObjectDream);
DEFINE_CLASS(ObjectDreams);
DEFINE_CLASS(ObjectDreamLand);
DEFINE_CLASS(ObjectHttpRequest);
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectInt64);
DEFINE_CLASS(ObjectUint32);
DEFINE_CLASS(ObjectUint64);
DEFINE_CLASS(ObjectUnique);
DEFINE_CLASS(ObjectLink);
DEFINE_CLASS(ObjectLinkAction);
DEFINE_CLASS(ObjectLinkInfo);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectMedia);
DEFINE_CLASS(ObjectOpenSSL);
DEFINE_CLASS(ObjectParam);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectTemplate);
DEFINE_CLASS(ObjectTime);
DEFINE_CLASS(ObjectType);
DEFINE_CLASS(ObjectVec3);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectWebsockFrame);

DEFINE_CLASS(Thread);

using namespace std;

#define _OBJECT(a_out, a_flags, ...)                        \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        Object,                                             \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP(a_out, a_flags, ...)                          \
    _CREATE_PROP(                                           \
        a_out,                                              \
        Object,                                             \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT(a_out, ...)                                  \
    CREATE_OBJECT(a_out, Object, ##__VA_ARGS__)

#define PROP(a_out, ...)                                    \
    CREATE_PROP(a_out, Object, ##__VA_ARGS__)

typedef void (*ApiHandler)(ObjectSptr, ObjectActionSptr);
typedef map <string, ApiHandler>                ApiHandlers;
typedef ApiHandlers::iterator                   ApiHandlersIt;

#define DEFINE_PROP(name, type)                             \
    private:                                                \
        type    m_old_##name;                               \
        type    m_##name;                                   \
    public:                                                 \
        virtual void    set##name(type a_val){              \
            lock_guard<recursive_mutex> guard(m_mutex);     \
                                                            \
            m_old_##name = m_##name;                        \
            m_##name     = a_val;                           \
        };                                                  \
        virtual type    get##name(){                        \
            lock_guard<recursive_mutex> guard(m_mutex);     \
            return m_##name;                                \
        };                                                  \
        virtual type    getOld##name(){                     \
            lock_guard<recursive_mutex> guard(m_mutex);     \
            return m_old_##name;                            \
        };

#define DEFINE_PARAM(name, type)                            \
    public:                                                 \
        template<typename T>                                \
        void set##name(T a_object){                         \
            ObjectSptr param;                               \
            param = getNeigh<type>();                       \
            param->setValue(a_object);                      \
        };                                                  \
        ObjectSptr get##name(){                             \
            ObjectSptr param;                               \
            param = getNeigh<type>();                       \
            return param->getValue();                       \
        };                                                  \

typedef struct
{
    string      (*type)(void);
    string      (*type_perl)(void);
    int32_t     (*init)(void);
    void        (*destroy)(void);
    ObjectSptr  (*object_create)(void);
    uint32_t    flags;
    ObjectSptr  object;
} ModuleInfo;

typedef shared_ptr<ModuleInfo>      ModuleInfoSptr;
typedef map<string, ModuleInfoSptr> Modules;
typedef shared_ptr<Modules>         ModulesSptr;
typedef Modules::iterator           ModulesIt;

#define REGISTER_MODULE(info) \
    static int32_t VARIABLE_IS_NOT_USED s_module_inited = Object::register_module(info)

class CmpObject
{
    public:
        CmpObject();
        ~CmpObject();

        bool operator () (
            ObjectSptr a,
            ObjectSptr b
        );
};

class Object
{
    public:
        enum CreateFlags {
            CREATE_PROP                     = 0,       // bit 0
            CREATE_OBJECT                   = 1,       // bit 0
            REMOVE_WHEN_LINKS_LOST          = (1 << 1),// bit 1
            REMOVE_WHEN_LINKS_LOST_FORCE    = (1 << 2) // bit 2
        };
        Object(const Object::CreateFlags &a_flags);
        virtual ~Object();

        DEFINE_PROP(Lock,           ObjectStringSptr);
        DEFINE_PROP(ObjectFlags,    ObjectUint32Sptr);
        DEFINE_PROP(TimeCreate,     ObjectTimeSptr);
        DEFINE_PROP(TimeRemove,     ObjectTimeSptr);
        DEFINE_PROP(TimeModify,     ObjectTimeSptr);
        DEFINE_PROP(Name,           ObjectStringSptr);
        DEFINE_PROP(Title,          ObjectStringSptr);
        DEFINE_PROP(Descr,          ObjectStringSptr);
        DEFINE_PROP(Url,            ObjectStringSptr);
        DEFINE_PROP(LinksInfo,      ObjectMapSptr);
        DEFINE_PROP(Pos,            ObjectVec3Sptr);
        DEFINE_PROP(ViewPos,        ObjectVec3Sptr);
        DEFINE_PROP(ViewUp,         ObjectVec3Sptr);
        DEFINE_PROP(ViewStrafe,     ObjectVec3Sptr);
        DEFINE_PROP(ViewZoom,       ObjectDoubleSptr);
        DEFINE_PROP(ViewRYP,        ObjectVec3Sptr);
//        DEFINE_PROP(Signals,        ObjectVectorSptr);
//        DEFINE_PROP(Events,         ObjectVectorSptr);

//        DEFINE_PROP(Owner,          ObjectStringSptr);
//        DEFINE_PROP(Group,          ObjectGroupSptr);
//        DEFINE_PROP(Acl,            ObjectAclSptr);
//        DEFINE_PROP(NeighsBackups,  ObjectMapSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(ObjectMapSptr);
//        virtual int32_t save();
//        virtual int32_t saveParams();
//        virtual int32_t parseBSON(mongo::BSONObj);
/*
        virtual void        objectChanged(
            const string    &a_object_name,
            ObjectSptr      a_old_val,
            ObjectSptr      a_new_val
        );
*/
        // Note: toString() is not same as serialize()
        // Example, look at PROP_STRING(a_out, "test")
        // - serialize must be: "test"
        // - toString  must be: test
        // because serialize is used while converting
        // to json, perl, etc.
        virtual ObjectStringSptr  toString();
        virtual ObjectStringSptr  serialize(
            const string    &a_delim = DEFAULT_DELIMITER
        );
/*
        virtual void    render(
            ObjectHttpAnswerSptr,
            // all request's props
            ObjectMapSptr a_req_props,
            // only request's 'params'
            ObjectMapSptr a_req_params
        );
        virtual void    renderEmbedded(
            ObjectHttpAnswerSptr,
            // all request's props
            ObjectMapSptr a_req_props,
            // only request's 'params'
            ObjectMapSptr a_req_params
        );
*/
        virtual void    parseBuff(
            const char      *a_data,
            const int64_t   &a_size
        );

        ApiHandler  getHandler(const string &a_action_name);

        static void processAction(
            ObjectSptr,
            ObjectActionSptr
        );

/*
        virtual void    processActions(ObjectVectorSptr);

        virtual void    signal(
            ObjectHttpAnswerSptr,
            ObjectMapSptr       a_req_props,
            ObjectMapSptr       a_req_params
        );
        virtual void    event(
            ObjectHttpAnswerSptr,
            ObjectMapSptr       a_req_props,
            ObjectMapSptr       a_req_params
        );

        virtual void    beforeRequest(
            ObjectHttpAnswerSptr,
            const ObjectMapSptr a_req_props,
            const ObjectMapSptr a_req_params
        );
        virtual void    remove(
            const int32_t &a_force = 0
        );
*/
        // generic
        void                setId(const string &);
        string              getId();
        int32_t             isNew();
        void                isNew(const int32_t &);
        void                setName(const string &);
        void                getPublicProps(
            ObjectMapSptr a_objects_out
        );
//        int32_t         _save();
/*
        void            signal(
            const string    &a_method,
            ObjectMapSptr   a_params = ObjectMapSptr()
        );
        void            event(
            const string    &a_method,
            ObjectMapSptr   a_params = ObjectMapSptr()
        );
*/
        ObjectVec3Sptr  getViewVector();
        void            rotateViewVector(
            ObjectDoubleSptr  a_yaw,
            ObjectDoubleSptr  a_pitch
        );
        void            accel(ObjectVec3Sptr a_accel);
        void            setDirty(const int32_t &);
        bool            isRemoved();

        void            registerAction(
            const string    &,
            ApiHandler      a_cb
        );

/*
        void            registerSignal(
            const string &,
            void (*a_cb)(
                Object *,
                ObjectHttpAnswerSptr,
                const ObjectMapSptr,  // a_req_props
                const ObjectMapSptr   // a_req_params
            )
        );
        void            registerEvent(
            const string &,
            void (*a_cb)(
                Object *,
                ObjectHttpAnswerSptr,
                const ObjectMapSptr,  // a_req_props
                const ObjectMapSptr   // a_req_params
            )
        );
*/

//        int32_t             saveProps(ObjectMapSptr);

//        void                addAction(ObjectActionSptr);
//        ObjectActionSptr    addAction(const string &);

//        void    getActions(ObjectVectorSptr);
//        void    getUnlinkActions(ObjectVectorSptr);

        // signals
//        void                addSignal(ObjectMapSptr);
//        ObjectMapSptr       addSignal(const string &);
//        void                clearSignals();
//        ObjectVectorSptr    getClearSignals();

        // events
//        void                sendApiActionPropsChanged(
//            ObjectMapSptr  a_objects
//        );
//        void                addEvent(ObjectMapSptr);
//        ObjectMapSptr       addEvent(const string &);
//        void                clearEvents();
//        ObjectVectorSptr    getClearEvents();

        // serialize
//        string              serializeWithParams(
//            const string    &
//        );
//        void                serializeToObjectMap(
//            ObjectMapSptr,
//            const int32_t   &a_serialize_params = 0
//        );

        // dirty
        void                getAllPropsDirty(ObjectMapSptr);
        bool                isDirty();

        void                checkDbLock();

        // templates
        string              getView(ObjectMapSptr a_params);
        ObjectTemplateSptr  getTemplate(ObjectMapSptr a_params);

        // bookmarks
        void    addToBookmarks();
        void    delFromBookmarks();

        // view
        void    strafe(const double &a_val);
        void    updateSphere();

/*
        void    getParams(
            ObjectMapSptr   a_out,
            const string    &a_name         = "",
            const int32_t   a_get_removed   = 0
        );


        // param
        template <class T = ObjectParam>
        shared_ptr <T> getParam(
            const string    &a_name,
            const int32_t   a_get_removed = 0)
        {
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr <T>              ret;
            vector< shared_ptr <T> >    res;
            ssize_t                     size = 0;

            _getParams(a_name, res);
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
                    " for name: '%s', object ID: '%s'\n",
                    a_name.c_str(),
                    getId().c_str()
                );
            }
            return ret;
        }

        template <class T = ObjectParam>
        shared_ptr <T>  createParam(const string &a_name){
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr <T> param;
            param = ThreadDb::get()->createObject<T>();
            param->setName(a_name);
            Object::link(param.get(), this);
            return param;
        }

        template <class T = ObjectParam>
        shared_ptr <T>  getCreateParam(const string &a_name){
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr <T> param;
            param = getParam<T>(a_name);
            if (!param){
                param = createParam<T>(a_name);
            }
            return param;
        }
*/

        recursive_mutex         m_mutex;

        // static
        static string           generateId();
        static void             createEvent(
            const string    &a_name,
            ObjectMapSptr   a_out_props,
            ObjectMapSptr   a_out_params
        );
        static void             getPublicProps(
            ObjectMapSptr       a_objects_out,
            ObjectMapSptr       a_objects_in
        );
        static ObjectVec3Sptr getObjectVec3(
            const ObjectMapSptr a_map,
            const string        &a_param_name
        );

        // createObject
        static ObjectSptr _createObject(
            const string    &a_type,
            const int32_t   &a_do_autorun   = 1,
            const string    &a_id           = ""
        );

        template <class T = Object>
        static shared_ptr<T> createObject(
            const string    &a_type,
            const int32_t   &a_do_autorun   = 1,
            const string    &a_id           = "")
        {
            shared_ptr<T> ret = dynamic_pointer_cast<T>(
                _createObject(
                    a_type,
                    a_do_autorun,
                    a_id
                )
            );
            return ret;
        };

        // neighs
/*
        ObjectLinkSptr  addNeigh(
            Object          *a_object,
            ObjectLinkSptr  a_link = ObjectLinkSptr(),
            const int32_t   &a_create_backlink = 1
        );
        ObjectLinkSptr  addNeigh(
            ObjectSptr      a_object,
            ObjectLinkSptr  a_link = ObjectLinkSptr(),
            const int32_t   &a_create_backlink = 1
        );
*/
//        void    removeLink(const string         &a_object_id);
        static void    neighUnlink(
            ObjectSptr      a_object_src,
            const string    &a_neigh_id,
            const int32_t   &a_force    = 1
        );
        static void    neighUnlink(
            ObjectSptr      a_object_src,
            ObjectSptr      a_object_dst,
            const int32_t   &a_force    = 1
        );
        static void    neighUnlinkAll(
            const int32_t    &a_force   = 1
        );
        static void    neighDelete(
            ObjectSptr      a_object_src,
            const string    &a_neigh_id,
            const int32_t   &a_force    = 1
        );

        bool    hasLink(
            const string &a_object_id
        );
        void    getLinks(
            ObjectVectorSptr a_out,
            const string     &a_name        = "",
            const int32_t    &a_get_removed = 0,
            const int32_t    &a_db_lock     = 0,
            ObjectUniqueSptr a_link_types   = ObjectUniqueSptr(),
            ObjectUniqueSptr a_skip         = ObjectUniqueSptr()
        );

        void    getNeighs(
            ObjectVectorSptr a_out,
            const string     &a_name        = "",
            const int32_t    &a_get_removed = 0,
            const int32_t    &a_db_lock     = 0,
            ObjectUniqueSptr a_object_types = ObjectUniqueSptr(),
            ObjectUniqueSptr a_link_types   = ObjectUniqueSptr(),
            ObjectUniqueSptr a_skip         = ObjectUniqueSptr()
        );

        static ObjectSptr _getNeigh(
            ObjectSptr       a_object_src,
            const string     &a_name        = "",
            const string     &a_type        = "",
            const int32_t    &a_get_removed = 0,
            const int32_t    &a_db_lock     = 0,
            ObjectUniqueSptr a_link_types   = ObjectUniqueSptr(),
            ObjectUniqueSptr a_skip         = ObjectUniqueSptr()
        );

        template <class T1 = Object>
        static shared_ptr<T1> getNeigh(
            ObjectSptr       a_object_src,
            const string     &a_name        = "",
            const string     &a_type        = "",
            const int32_t    &a_get_removed = 0,
            const int32_t    &a_db_lock     = 0,
            ObjectUniqueSptr a_link_types   = ObjectUniqueSptr(),
            ObjectUniqueSptr a_skip         = ObjectUniqueSptr())
        {
            //lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T1> ret;

            ret = dynamic_pointer_cast<T1>(
                _getNeigh(
                    a_object_src,
                    a_name,
                    a_type,
                    a_get_removed,
                    a_db_lock,
                    a_link_types,
                    a_skip
                )
            );

            return ret;
        }

        static ObjectSptr _createNeigh(
            ObjectSptr      a_object_src,
            const string    &a_type,
            const string    &a_name         = "",
            const int32_t   &a_do_autorun   = 1
        );

        template <class T = Object>
        static shared_ptr<T> createNeigh(
            ObjectSptr      a_object_src,
            const string    &a_type         = "",
            const string    &a_name         = "",
            const int32_t   &a_do_autorun   = 1)
        {
            //lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T>  ret = dynamic_pointer_cast<T>(
                _createNeigh(
                    a_object_src,
                    a_type,
                    a_name,
                    a_do_autorun
                )
            );

            return ret;
        }
/*
        static ObjectSptr _getNeigh(
            ObjectSptr      a_object_src,
            const string    &a_name,
            const string    &a_type,
            const int32_t   &a_get_removed,
            const int32_t   &a_db_lock
        );
*/
        static ObjectSptr _getCreateNeigh(
            ObjectSptr      a_object_src,
            const string    &a_type,
            const string    &a_name        = "",
            const int32_t   &a_get_removed = 0,
            const int32_t   &a_db_lock     = 0,
            const int32_t   &a_do_autorun  = 1
        );

        template <class T = Object>
        static shared_ptr<T> getCreateNeigh(
            ObjectSptr      a_object_src,
            const string    &a_type,
            const string    &a_name        = "",
            const int32_t   &a_get_removed = 0,
            const int32_t   &a_db_lock     = 0,
            const int32_t   &a_do_autorun  = 1)
        {
            //lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T> ret = dynamic_pointer_cast<T>(
                _getCreateNeigh(
                    a_object_src,
                    a_type,
                    a_name,
                    a_get_removed,
                    a_db_lock,
                    a_do_autorun
                )
            );
            return ret;
        }

        static ObjectLinkSptr  addNeigh(
            ObjectSptr      a_object_src,
            ObjectSptr      a_object_dst,
            const int32_t   &a_create_backlink = 1,
            ObjectLinkSptr  a_link = ObjectLinkSptr()
        );

/*
        void    neighUnlink(
            ObjectSptr      a_object_src,
            const string    &a_neigh_id,
            const int32_t   &a_force
        );
*/
        static void     neighUnlinkAll(
            ObjectSptr      a_object,
            const int32_t   &a_force    = 1
        );
        static void     removeLink(
            ObjectSptr      a_object,
            const string    &a_link_id
        );
        static void     remove(
            ObjectSptr      a_object_src,
            const int32_t   &a_force    = 1
        );
        static void     save(ObjectSptr a_object_src);

        // actions
        static void actionCreate(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionUpdate(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionLoad(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionDelete(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionNeighUnlink(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionRender(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionRenderEmbedded(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionGetEvents(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionGetNeighs(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionGetCreateNeigh(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionBookmark(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionUpdateEventsSubscr(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionAccel(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionRotateViewVector(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionMove(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

        // modules
        static ModuleInfoSptr   getModuleInfoByType(
            const string &
        );
        static int32_t  register_module(const ModuleInfo &);
        static void     moduleInfoToMap(
            ModuleInfoSptr  a_info,
            ObjectMapSptr   a_out
        );
        static void             getModulesInfo(
            ObjectMapSptr a_out
        );
        static ObjectMapSptr    getModuleInfo(
            const string  &a_object_type
        );
        static void     listModules();
        static int32_t  initModules();

    protected:
        CreateFlags     m_flags;

//        void            processSignals(ObjectVectorSptr);
//        void            processEvents(ObjectVectorSptr);

    private:
        string          m_id;

        ApiHandlers     m_action_handlers;
//        ApiHandlers     m_signal_handlers;
//        ApiHandlers     m_event_handlers;

        int32_t         m_is_new;
        int32_t         m_is_dirty;

        void            do_init(const Object::CreateFlags &);

        static ObjectLinkSptr link(
            Object          *a_object,
            Object          *b_object,
            ObjectLinkSptr  a_link = ObjectLinkSptr()
        );

        static ObjectLinkSptr linkAndSave(
            Object          *a_object,
            Object          *b_object,
            ObjectLinkSptr  a_link = ObjectLinkSptr()
        );
};

#endif

