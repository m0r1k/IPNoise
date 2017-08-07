#include "class.hpp"
DEFINE_CLASS(DbMongo);

#ifndef DB_MONGO_HPP
#define DB_MONGO_HPP

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <stdio.h>

#include <map>
#include <memory>
#include <mutex>

#include <mongo/client/dbclient.h>

#include "log.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectType);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectVec3);

DEFINE_CLASS(Db);
DEFINE_CLASS(Context);

using namespace std;

typedef map <string, ObjectSptr>    ObjectsById;
typedef ObjectsById::iterator       ObjectsByIdIt;
typedef ObjectsById::const_iterator ObjectsByIdConstIt;

class DbMongo
    :   public Db
{
    public:
        DbMongo();
        virtual ~DbMongo();

        // interface
        virtual void    do_clean();
        virtual int32_t do_init();
        virtual void    getActionsForProcess(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_do_autorun   = 1
        );
        virtual void    getObjectsForRemove(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun   = 1
        );
        virtual ObjectSptr _getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1
        );
        virtual int32_t     insert(ObjectSptr   a_object);
        virtual int32_t     update(ObjectSptr   a_object);
        virtual void        remove(const string &a_object_id);
        virtual int32_t     save(ObjectSptr     a_object);

        // generic
//        void    cleanActionObjects();
        void    unlockLocationObjects();
        void    cleanLocationObjects();
        void    getActionsForCurThread(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_do_autorun   = 1
        );
        void    getActionsForAnyThread(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_do_autorun   = 1
        );
        void    getObjectsForRemoveExpired(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun   = 1
        );
/*
        void    getObjectsForRemoveLocation(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun   = 1
        );
        void    getObjectsForRemoveLock(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun   = 1
        );
*/

        // static
        static void bsonToProps(
            mongo::BSONObj  a_obj,
            ObjectSptr      a_props
        );

        static ObjectVectorSptr getArrayFieldAsVectorOfDouble(
            mongo::BSONObj  a_obj,
            const string    &a_field_name
        );

        static ObjectVec3Sptr getArrayFieldAsPropVec3(
            mongo::BSONObj  a_obj,
            const string    &a_field_name
        );

        static ObjectVec3Sptr getMapFieldAsPropVec3(
            mongo::BSONObj  a_obj,
            const string    &a_field_name
        );


    protected:

        friend class ObjectContext;
        friend class ObjectTransaction;

        ObjectSptr      getCreateObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0
        );
        bool            isObjectExist(
            const string    &a_id
        );
        ObjectSptr      getObjectByIdFromDb(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_do_autorun   = 1
        );

        void getObjectsByPos(
            ObjectVec3Sptr      a_pos_min,
            ObjectVec3Sptr      a_pos_max,
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_db_lock      = 0,
            const int32_t       &a_do_autorun   = 1
        );

        void _processCommand(
            ObjectVectorSptr    a_out,
            const string        &a_cmd,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_db_lock      = 0,
            const int32_t       &a_do_autorun   = 1
        );

    private:
        mongo::DBClientConnection   m_mongo_db;
};

#endif

