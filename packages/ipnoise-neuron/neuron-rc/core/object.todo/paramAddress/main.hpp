#include "class.hpp"
DEFINE_CLASS(ObjectParamAddress);

#ifndef OBJECT_PARAM_ADDRESS_HPP
#define OBJECT_PARAM_ADDRESS_HPP

#include <stdio.h>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

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
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectParam);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_PARAM_ADDRESS(a_out, a_flags, ...)          \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectParamAddress,                                 \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_PARAM_ADDRESS(a_out, a_flags, ...)            \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectParamAddress,                                 \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_PARAM_ADDRESS(a_out, ...)                    \
    CREATE_OBJECT(a_out, ObjectParamAddress, ##__VA_ARGS__)

#define PROP_PARAM_ADDRESS(a_out, ...)                      \
    CREATE_PROP(a_out, ObjectParamAddress, ##__VA_ARGS__)

class ObjectParamAddress
    :   public  ObjectParam
{
    public:
        ObjectParamAddress(const Object::CreateFlags &);
        virtual ~ObjectParamAddress();

        DEFINE_PROP(Levels, ObjectVectorSptr);

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

        // api
        static void actionUpdate(
            Object          *a_object,
            ObjectAction    *a_action
        );
        static void actionGetLevel(
            Object          *a_object,
            ObjectAction    *a_action
        );
        static void actionGetRegionTypes(
            Object          *a_object,
            ObjectAction    *a_action
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        static  int32_t  kladrSqlCb(
            void    *a_kladr_req_info,
            int32_t a_argc,
            char    **a_argv,
            char    **a_col_name
        );

        void    do_init(const Object::CreateFlags &);
};

#endif

