#include "class.hpp"
DEFINE_CLASS(ObjectBasket);

#ifndef OBJECT_BASKET_HPP
#define OBJECT_BASKET_HPP

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
DEFINE_CLASS(ObjectHttpAnswer);

using namespace std;

#define _OBJECT_BASKET(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectBasket,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_BASKET(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectBasket,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_BASKET(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectBasket, ##__VA_ARGS__ )

#define PROP_BASKET(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectBasket, ##__VA_ARGS__ )

class ObjectBasket
    :   public  Object
{
    public:
        ObjectBasket(const Object::CreateFlags &);
        virtual ~ObjectBasket();

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
        static void actionCreateOrder(
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

