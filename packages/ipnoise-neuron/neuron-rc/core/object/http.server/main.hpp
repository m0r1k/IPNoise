#include "class.hpp"
DEFINE_CLASS(ObjectHttpServer);

#ifndef OBJECT_HTTP_SERVER_HPP
#define OBJECT_HTTP_SERVER_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectUint32);

#define DEFAULT_LISTEN_HOST "0.0.0.0"
#define DEFAULT_LISTEN_PORT 8080

using namespace std;

class ObjectHttpServer
    :   public Object
{
    public:
        ObjectHttpServer(const CreateFlags &a_create_flags);
        virtual ~ObjectHttpServer();

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
        void                setHost(const char *a_val);
        ObjectStringSptr    getHost();
        void                setPort(const uint32_t &a_port);
        ObjectUint32Sptr    getPort();

        // static
        static int32_t      s_actionListen(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionListened(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionAccepted(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionReaded(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionEvent(
            Object *a_object,
            Object *a_action
        );

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectStringSptr    m_host;
        ObjectUint32Sptr    m_port;
        ObjectMapSptr       m_pending_data;
};

#endif

