#include "class.hpp"
DEFINE_CLASS(ObjectTemplate)

#ifndef OBJECT_TEMPLATE_HPP
#define OBJECT_TEMPLATE_HPP

#include <stdint.h>

#include <string>

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
DEFINE_CLASS(Context);

using namespace std;

#define _OBJECT_TEMPLATE(a_out, a_flags, ...)               \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectTemplate,                                     \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_TEMPLATE(a_out, a_flags, ...)                 \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectTemplate,                                     \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_TEMPLATE(a_out, ...)                         \
    CREATE_OBJECT(a_out, ObjectTemplate, ##__VA_ARGS__)

#define PROP_TEMPLATE(a_out, ...)                           \
    CREATE_PROP(a_out, ObjectTemplate, ##__VA_ARGS__)

class ObjectTemplate
    :   public Object
{
    public:
        ObjectTemplate(const Object::CreateFlags &);
        virtual ~ObjectTemplate();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();

        // generic
//        void    setupContext();
        void    setPrefix(const string &);
        string  getPrefix();
        void    setObject(const Object *);
        string  render(
            const string    &a_object_id,
            const string    &a_view = "index"
        );

        string  renderEmbedded(
            const string    &a_object_id,
            const string    &a_view = "widget"
        );

        int32_t parseFile(
            const string    &a_fname,
            string          &a_out
        );

        // static
        static ObjectSptr   getObject();
//        static SV *         toPerl(ObjectSptr);

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    protected:

    private:
        const Object    *m_object;
        string          m_prefix;

        void    do_init(const Object::CreateFlags &a_flags);
};

#endif

