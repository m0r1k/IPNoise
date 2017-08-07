#include "class.hpp"
DEFINE_CLASS(ObjectIPNoise);

#ifndef OBJECT_IPNOISE_HPP
#define OBJECT_IPNOISE_HPP

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
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectIPNoiseLinks);

using namespace std;

#define _OBJECT_IPNOISE(a_out, a_flags, ...)                \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectIPNoise,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_IPNOISE(a_out, a_flags, ...)                  \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectIPNoise,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_IPNOISE(a_out, ...)                          \
    CREATE_OBJECT(a_out, ObjectIPNoise, ##__VA_ARGS__ )

#define PROP_IPNOISE(a_out, ...)                            \
    CREATE_PROP(a_out, ObjectIPNoise, ##__VA_ARGS__ )

class ObjectIPNoise
    :   public  Object
{
    public:
        ObjectIPNoise(const Object::CreateFlags &);
        virtual ~ObjectIPNoise();

        DEFINE_PROP(DstHuid, ObjectStringSptr);
        DEFINE_PROP(SrcHuid, ObjectStringSptr);
        DEFINE_PROP(LLAddr,  ObjectStringSptr);
        DEFINE_PROP(DevName, ObjectStringSptr);

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
        ObjectIPNoiseLinksSptr  getIPNoiseLinksObject();
        ObjectIPNoiseLinksSptr  getCreateIPNoiseLinksObject();

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

