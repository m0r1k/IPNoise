#include "class.hpp"
DEFINE_CLASS(ObjectInotify);

#ifndef OBJECT_INOTIFY_HPP
#define OBJECT_INOTIFY_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);

using namespace std;

class ObjectInotify
    :   public Object
{
    public:
        ObjectInotify(const CreateFlags &a_create_flags);
        virtual ~ObjectInotify();

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
};

#endif

