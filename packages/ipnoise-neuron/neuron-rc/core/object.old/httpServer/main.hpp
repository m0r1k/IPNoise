#include "class.hpp"
DEFINE_CLASS(ObjectHttpServer);

#ifndef OBJECT_HTTP_SERVER_HPP
#define OBJECT_HTTP_SERVER_HPP

#include <stdint.h>

// libevent
#include <event2/util.h>

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

using namespace std;

#define HTTP_SERVER_PORT    8080

#define _OBJECT_HTTP_SERVER(a_out, a_flags, ...)            \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectHttpServer,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_HTTP_SERVER(a_out, a_flags, ...)              \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectHttpServer,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_HTTP_SERVER(a_out, ...)                      \
    CREATE_OBJECT(a_out, ObjectHttpServer, ##__VA_ARGS__)

#define PROP_HTTP_SERVER(a_out, ...)                        \
    CREATE_PROP(a_out, ObjectHttpServer, ##__VA_ARGS__)

class ObjectHttpServer
    :   public Object
{
    public:
        ObjectHttpServer(const Object::CreateFlags &);
        virtual ~ObjectHttpServer();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
//        virtual void    processEvents(ObjectVectorSptr);

        // generic

        // static

        // api
        static void     actionListen(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionAccept(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );

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

