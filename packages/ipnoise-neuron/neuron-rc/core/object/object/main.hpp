#include "class.hpp"
DEFINE_CLASS(Object);

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>
#include <mutex>

#include <ipnoise-common/log_common.h>

#include "module.hpp"
#include "utils.hpp"

#define OBJECT_DIR_ACTIONS          "actions/"
#define OBJECT_DIR_CHILDREN         "children/"
#define OBJECT_DIR_PROPS            "props/"
#define OBJECT_DIR_VALS             "vals/"
#define OBJECT_FILE_VAL             "val"

#define OBJECT_ID_LEN_BYTES         20

extern EngineInfo   *g_engine_info;

DEFINE_CLASS(ObjectLinks);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectUnique);
DEFINE_CLASS(ObjectVector);

using namespace std;

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
        Object(const CreateFlags &a_create_flags);
        virtual ~Object();

        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);
        virtual void        getAllPropsDirty(ObjectMapSptr);
        virtual void        setDirty(const int32_t &);
        virtual bool        isDirty();
        virtual int32_t     loadAsProp(const char *a_path = NULL);
        virtual int32_t     load(const char *a_path = NULL);
        virtual int32_t     saveAsProp(const char *a_path = NULL);
        virtual int32_t     save(const char *a_path = NULL);
        virtual int32_t     remove(const int32_t &a_force = 1);
        virtual void        run(Object *a_thread = NULL);
        virtual ObjectStringSptr    toString();
        virtual string      toStringStd();
        virtual ObjectStringSptr serialize(
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual void        serialize(
            string      &a_out,
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual void        setObjectPath(const char *a_path);
        virtual string      getObjectPath();

        virtual string      getId(const char *a_path = NULL);
//        virtual void        getInTranslateTable(TranslateTable &);
//        virtual void        getOutTranslateTable(TranslateTable &);
//        virtual ObjectSptr  getPropVal();
        virtual void        emit(ObjectSptr);
        virtual int32_t     linkAdd(const char *a_path);
        virtual int32_t     linkErase(const char *a_path);
        virtual void        processAction(Object *);

        // generic
        recursive_mutex     m_rmutex;

        CreateFlags     getObjectFlags();
        string          getObjectName();
        void            setObjectName(const char *);
        void            setObjectLinks(ObjectLinksSptr a_val);
        ObjectLinksSptr getObjectLinks();
        void            registerActionIn(
            const char      *a_name,
            ActionHandler   handler
        );
        void            registerActionOut(
            const char *a_name
        );

        // -------- action --------
        void            setActionParams(ObjectMapSptr);
        ObjectMapSptr   getActionParams();
        ObjectSptr      getActionParam(ObjectStringSptr);
        ObjectSptr      getActionParam(
            const char *a_param_name
        );
        void            setActionParam(
            ObjectStringSptr    a_param_name,
            ObjectSptr          a_param_val
        );
        void            setActionParam(
            const char          *a_param_name,
            ObjectSptr          a_param_val
        );
        void            setActionParam(
            const char      *a_param_name,
            const int32_t   &a_param_val
        );
        void            setActionParam(
            const char      *a_param_name,
            const char      *a_param_val
        );
        void            setActionParam(
            const char      *a_param_name,
            const char      *a_param_val,
            const uint64_t  a_param_val_size
        );
        // void processActions();
        void                setActionName(const char *a_val);
        ObjectStringSptr    getActionName();
        void                setActionSrc(const char *a_val);
        ObjectStringSptr    getActionSrc();
        void                setActionDst(const char *a_val);
        ObjectStringSptr    getActionDst();

        // -------- OBJECT --------

/*
        template <class T>
        static shared_ptr<T> OBJECT(
            const char        *a_data,
            const uint64_t    &a_data_size,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            T               fake(CREATE_DEFAULT);
            string          object_type;

            object_type = fake.getType();
            if (object_type.empty()){
                PFATAL("unknown object type\n");
            }

            ret = dynamic_pointer_cast<T>(
                g_engine_info->objectCreate(
                    object_type.c_str(),
                    a_data,
                    a_data_size,
                    a_create_flags
                )
            );

            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const int32_t     &a_data,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            ret = OBJECT<T>(
                (const char *)&a_data,
                sizeof(a_data),
                a_create_flags
            );
            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const string      &a_data,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            ret = OBJECT<T>(
                a_data.c_str(),
                a_data.size(),
                a_create_flags
            );
            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            ret = OBJECT<T>(
                NULL,
                0,
                a_create_flags
            );
            return ret;
        }
*/
        // -------- OBJECT --------

        template <class T>
        static shared_ptr<T> OBJECT(
            const char        *a_data,
            const uint64_t    &a_data_size,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            T               fake(CREATE_PROP);
            string          object_type;

            object_type = fake.getType();
            if (object_type.empty()){
                PFATAL("unknown object type\n");
            }

            if (a_create_flags & CREATE_PROP){
                ret = dynamic_pointer_cast<T>(
                    g_engine_info->propCreate(
                        object_type.c_str(),
                        a_data,
                        a_data_size,
                        a_create_flags
                    )
                );
            } else {
                ret = dynamic_pointer_cast<T>(
                    g_engine_info->objectCreate(
                        object_type.c_str(),
                        a_data,
                        a_data_size,
                        a_create_flags
                    )
                );
            }

            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const int32_t     &a_data,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            char            buffer[512] = { 0x00 };
            int32_t         res;

            res = snprintf(buffer, sizeof(buffer),
                "%d",
                a_data
            );
            if (0 >= res){
                PFATAL("snprintf failed, res: '%d'\n", res);
            }
            ret = OBJECT<T>(
                buffer,
                res,
                a_create_flags
            );
            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const string      &a_data,
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            ret = OBJECT<T>(
                a_data.c_str(),
                a_data.size(),
                a_create_flags
            );
            return ret;
        }

        template <class T>
        static shared_ptr<T> OBJECT(
            const CreateFlags &a_create_flags = CREATE_DEFAULT)
        {
            shared_ptr<T>   ret;
            ret = OBJECT<T>(
                NULL,
                0,
                a_create_flags
            );
            return ret;
        }

        // static
        static string  s_generateId();

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static int32_t      s_testLoad(const char *tmp_path);
        static int32_t      s_testSave(const char *tmp_path);
        static void         s_getTests(Tests &);

    private:
        int32_t             m_is_dirty = 0;
        CreateFlags         m_create_flags;
        // m_object_path, m_name, must be std::string
        // because src/engine.cpp does't known about ObjectString
        string              m_object_path;
        string              m_name;
        ObjectLinksSptr     m_links;
        ActionsHandlers     m_actions_in;
        ObjectUniqueSptr    m_actions_out;
        ObjectMapSptr       m_action_params;
};

#endif

