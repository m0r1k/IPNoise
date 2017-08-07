#include "class.hpp"
DEFINE_CLASS(ObjectAction);

#ifndef OBJECT_ACTION_HPP
#define OBJECT_ACTION_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

class ObjectAction
    :   public Object
{
    public:
        ObjectAction(const CreateFlags &a_create_flags);
        virtual ~ObjectAction();

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
        virtual void        run(Object *a_thread = NULL);

        // generic
        void            setSrc(const char *);
        void            setActionParams(ObjectMapSptr);
        ObjectMapSptr   getParams();
        ObjectSptr      getParam(ObjectStringSptr);
        ObjectSptr      getParam(const char *a_param_name);
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
            const char      *a_param_val,
            const uint64_t  a_param_val_size
        );

        // static

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectStringSptr    m_src;
        ObjectMapSptr       m_action_params;
};

#endif

