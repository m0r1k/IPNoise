#include "class.hpp"
DEFINE_CLASS(ObjectUser);

#ifndef OBJECT_USER_HPP
#define OBJECT_USER_HPP

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
DEFINE_CLASS(ObjectBasket);
DEFINE_CLASS(ObjectBookmarks);
DEFINE_CLASS(ObjectCamera);
DEFINE_CLASS(ObjectDialog);
DEFINE_CLASS(ObjectDialogs);
DEFINE_CLASS(ObjectDream);
DEFINE_CLASS(ObjectDreams);
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectOrder);
DEFINE_CLASS(ObjectOrders);
DEFINE_CLASS(ObjectParamAddress);
DEFINE_CLASS(ObjectParamAddress);
DEFINE_CLASS(ObjectSession);
DEFINE_CLASS(ObjectShop);
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_USER(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectUser,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_USER(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectUser,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_USER(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectUser, ##__VA_ARGS__ )

#define PROP_USER(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectUser, ##__VA_ARGS__ )

class ObjectUser
    :   public  Object
{
    public:
        ObjectUser(const Object::CreateFlags &);
        virtual ~ObjectUser();

        DEFINE_PROP(Password, ObjectStringSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual void    beforeRequest(
            ObjectHttpAnswerSptr    a_answer,
            const ObjectMapSptr     a_req_props,
            const ObjectMapSptr     a_req_params
        );

        // generic
        bool                    isAdmin();
//        void                    addToBookmarks(Object *);
//        void                    delFromBookmarks(
//            const string &a_neigh_id
//        );
//        ObjectBookmarksSptr     getCreateBookmarks();
//        ObjectBookmarksSptr     getBookmarks();

//        ObjectParamAddressSptr  getCreateAddress(
//            const string &a_object_id
//        );
//        ObjectParamAddressSptr  getAddress(
//            const string &a_object_id
//        );
//        ObjectCameraSptr        getCamera();
//        ObjectCameraSptr        getCreateCamera();

//        ObjectBasketSptr        getBasket();
//        ObjectBasketSptr        getCreateBasket();
//        ObjectDreamsSptr        getDreamsObject();
//        ObjectDreamsSptr        getCreateDreamsObject();
//        ObjectDialogsSptr       getDialogsObject();
//        ObjectDialogsSptr       getCreateDialogsObject();
//        ObjectOrdersSptr        getOrdersObject();
//        ObjectOrdersSptr        getCreateOrdersObject();
//        ObjectShopSptr          getShopObject();
//        ObjectShopSptr          getCreateShopObject();
//        void                    addAction(ObjectActionSptr);

        void    getSessions(
            ObjectVectorSptr,
            const int32_t &a_db_lock = 0
        );

        // static

        // api
        static void actionUpdate(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void actionGetCreateAddress(
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

