#include "class.hpp"
DEFINE_CLASS(ObjectBookmarks);

#ifndef OBJECT_BOOKMARKS_HPP
#define OBJECT_BOOKMARKS_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

#include <mongo/client/dbclient.h>
#include <ipnoise-common/log_common.h>
#include "utils.hpp"

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectMap);

using namespace std;

#define _OBJECT_BOOKMARKS(a_out, a_flags, ...)              \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectBookmarks,                                    \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_BOOKMARKS(a_out, a_flags, ...)                \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectBookmarks,                                    \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_BOOKMARKS(a_out, ...)                        \
    CREATE_OBJECT(a_out, ObjectBookmarks, ##__VA_ARGS__)

#define PROP_BOOKMARKS(a_out, ...)                          \
    CREATE_PROP(a_out, ObjectBookmarks, ##__VA_ARGS__)

class ObjectBookmarks
    :   public  Object
{
    public:
        ObjectBookmarks(const Object::CreateFlags &);
        virtual ~ObjectBookmarks();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

