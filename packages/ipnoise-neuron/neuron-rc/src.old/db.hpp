#include "class.hpp"
DEFINE_CLASS(Db);

#ifndef DB_HPP
#define DB_HPP

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
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectType);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectVec3);

DEFINE_CLASS(Context);

using namespace std;

class Db
{
    public:
        Db();
        virtual ~Db();

        // interface
        virtual void    do_clean()  = 0;
        virtual int32_t do_init()   = 0;
        virtual void    getActionsForProcess(
            ObjectVectorSptr    a_out,
            const int32_t       &a_get_removed  = 0,
            const int32_t       &a_do_autorun   = 1
        ) = 0;
        virtual void    getObjectsForRemove(
            ObjectVectorSptr    a_out,
            const int32_t       &a_do_autorun   = 1
        ) = 0;
        virtual ObjectSptr _getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1
        ) = 0;
        virtual int32_t insert(ObjectSptr   a_object)     = 0;
        virtual int32_t update(ObjectSptr   a_object)     = 0;
        virtual void    remove(const string &a_object_id) = 0;
        virtual int32_t save(ObjectSptr     a_object);

        recursive_mutex m_mutex;

        // generic
        template <class T = Object>
        shared_ptr<T> getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_do_autorun   = 1)
        {
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T> ret;
            ret = dynamic_pointer_cast<T>(
                _getObjectById(
                    a_id,
                    a_type,
                    a_get_removed,
                    a_do_autorun
                )
            );
            return ret;
        }
};

#endif

