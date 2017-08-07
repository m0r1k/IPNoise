#include "class.hpp"
DEFINE_CLASS(ObjectDummy);

#ifndef OBJECT_DUMMY_HPP
#define OBJECT_DUMMY_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectDouble);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString)
DEFINE_CLASS(ObjectVector);

using namespace std;

class ObjectDummy
    :   public Object
{
    public:
        ObjectDummy(const CreateFlags &a_create_flags);
        virtual ~ObjectDummy();

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
        virtual int32_t     saveAsProp(const char *a_path = NULL);

        // generic

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
        ObjectMapSptr       m_map;
        ObjectVectorSptr    m_vector;
        ObjectStringSptr    m_string;
        ObjectInt32Sptr     m_int32;
        ObjectDoubleSptr    m_double;
};

#endif

