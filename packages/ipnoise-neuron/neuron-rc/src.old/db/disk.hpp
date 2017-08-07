#include "class.hpp"
DEFINE_CLASS(DbDisk);

#ifndef DB_DISK_HPP
#define DB_DISK_HPP

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <stdio.h>

#include <map>
#include <memory>
#include <mutex>

#include "log.hpp"
#include "thread.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectDir);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectType);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectVec3);

DEFINE_CLASS(Db)
DEFINE_CLASS(Context);

using namespace std;

// dir must have '/' at end
#define DB_DIR_ROOT     "db/"
#define DB_DIR_OBJECTS  DB_DIR_ROOT"objects/"
#define DB_DIR_ACTIONS  DB_DIR_ROOT"actions/"

class DbDisk
    :   public Db
{
    public:
        DbDisk();
        virtual ~DbDisk();

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
        virtual int32_t insert(ObjectSptr   a_object);
        virtual int32_t update(ObjectSptr   a_object);
        virtual void    remove(const string &a_object_id);

        // generic
        void    getActionsForCurThread(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed,
            const int32_t       &a_do_autorun
        );
        void    getActionsForAnyThread(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed,
            const int32_t       &a_do_autorun
        );
        void    getObjectsForRemoveExpired(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun
        );

        void    unlockLocationObjects();
        void    cleanLocationObjects();

        ObjectDirSptr  getObjectDir(
            const char *a_object_id
        );
        ObjectDirSptr  getObjectDir(
            const string &a_object_id
        );
        ObjectDirSptr  getCreateObjectDir(
            const char *a_object_id
        );
        ObjectDirSptr  getCreateObjectDir(
            const string &a_object_id
        );

        static void loadProp(
            ObjectDirSptr   a_dir,
            const char      *a_name,
            ObjectSptr      a_val
        );

        static void saveProp(
            ObjectDirSptr   a_dir,
            const char      *a_name,
            ObjectSptr      a_val
        );

        // static
};

#endif

