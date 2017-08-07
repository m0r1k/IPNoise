#include "class.hpp"
DEFINE_CLASS(ObjectIPNoiseSkb);

#ifndef OBJECT_IPNOISE_SKB_HPP
#define OBJECT_IPNOISE_SKB_HPP

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
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_IPNOISE_SKB(a_out, a_flags, ...)            \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectIPNoiseSkb,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_IPNOISE_SKB(a_out, a_flags, ...)              \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectIPNoiseSkb,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_IPNOISE_SKB(a_out, ...)                      \
    CREATE_OBJECT(a_out, ObjectIPNoiseSkb, ##__VA_ARGS__)

#define PROP_IPNOISE_SKB(a_out, ...)                        \
    CREATE_PROP(a_out, ObjectIPNoiseSkb, ##__VA_ARGS__)

class ObjectIPNoiseSkb
    :   public  Object
{
    public:
        ObjectIPNoiseSkb(const Object::CreateFlags &);
        virtual ~ObjectIPNoiseSkb();

        DEFINE_PROP(InLinkName,     ObjectStringSptr);
        DEFINE_PROP(InLinkIndex,    ObjectInt32Sptr);
        DEFINE_PROP(InContent,      ObjectStringSptr);
        DEFINE_PROP(From,           ObjectStringSptr);
        DEFINE_PROP(To,             ObjectStringSptr);

 //       DEFINE_PROP(DstHuid, ObjectStringSptr);
 //       DEFINE_PROP(SrcHuid, ObjectStringSptr);
 //       DEFINE_PROP(LLAddr,  ObjectStringSptr);
 //       DEFINE_PROP(DevName, ObjectStringSptr);

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
//        string  getLinkAddress();
//        string  getLinkName();

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

