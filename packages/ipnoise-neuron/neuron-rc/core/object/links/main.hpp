#include "class.hpp"
DEFINE_CLASS(ObjectLinks);

#ifndef OBJECT_LINKS_HPP
#define OBJECT_LINKS_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

class ObjectLinks
    :   public Object
{
    public:
        ObjectLinks(const CreateFlags &a_create_flags);
        virtual ~ObjectLinks();

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
        void            add(const char *a_path);
        void            erase(const char *a_path);
        void            setVals(ObjectMapSptr);
        ObjectMapSptr   getVals();

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
        ObjectMapSptr m_vals;
};

#endif

