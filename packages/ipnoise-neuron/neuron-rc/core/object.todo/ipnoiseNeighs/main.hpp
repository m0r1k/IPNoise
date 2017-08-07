#include "class.hpp"
DEFINE_CLASS(ObjectIPNoiseNeighs);

#ifndef OBJECT_IPNOISE_NEIGHS_HPP
#define OBJECT_IPNOISE_NEIGHS_HPP

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
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectIPNoiseNeigh);

using namespace std;

#define _OBJECT_IPNOISE_NEIGHS(a_out, a_flags, ...)         \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectIPNoiseNeighs,                                \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_IPNOISE_NEIGHS(a_out, a_flags, ...)           \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectIPNoiseNeighs,                                \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_IPNOISE_NEIGHS(a_out, ...)                   \
    CREATE_OBJECT(a_out, ObjectIPNoiseNeighs, ##__VA_ARGS__ )

#define PROP_IPNOISE_NEIGHS(a_out, ...)                     \
    CREATE_PROP(a_out, ObjectIPNoiseNeighs, ##__VA_ARGS__ )

class ObjectIPNoiseNeighs
    :   public  Object
{
    public:
        ObjectIPNoiseNeighs(const Object::CreateFlags &);
        virtual ~ObjectIPNoiseNeighs();

        DEFINE_PROP(Huid, ObjectStringSptr);

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
        ObjectIPNoiseNeighSptr  createIPNoiseNeigh();

        // static

        // api
        static void actionUpdate(
            Object          *a_object,
            ObjectAction    *a_action
        );
        static void actionAddNewNeigh(
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
        void    do_init(const Object::CreateFlags &);
};

#endif

