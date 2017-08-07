#include "class.hpp"
DEFINE_CLASS(ObjectInt32);

#ifndef OBJECT_INT32_HPP
#define OBJECT_INT32_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

class ObjectInt32
    :   public Object
{
    public:
        ObjectInt32(const CreateFlags &a_create_flags);
        virtual ~ObjectInt32();

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
        virtual ObjectStringSptr toString();
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual int32_t     loadAsProp(const char *a_dir);
        virtual int32_t     saveAsProp(const char *a_dir);

        // generic
        int32_t     getVal();
        void        setVal(const int32_t &);
        void        incVal(const uint32_t &a_val = 1);
        void        decVal(const uint32_t &a_val = 1);

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
        int32_t m_val;
};

#endif

