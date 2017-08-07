#include "class.hpp"
DEFINE_CLASS(ObjectDir);

#ifndef OBJECT_DIR_HPP
#define OBJECT_DIR_HPP

#include <stdint.h>
#include <stdio.h>

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
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_DIR(a_out, a_flags, ...)                    \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectDir,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_DIR(a_out, a_flags, ...)                      \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectDir,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_DIR(a_out, ...)                        \
    CREATE_OBJECT(a_out, ObjectDir, ##__VA_ARGS__)

#define PROP_DIR(a_out, ...)                          \
    CREATE_PROP(a_out, ObjectDir, ##__VA_ARGS__)

class ObjectDir
    :   public Object
{
    public:
        ObjectDir(const Object::CreateFlags &);
        ObjectDir(
            const Object::CreateFlags   &,
            ObjectStringSptr            a_val
        );
        ObjectDir(
            const Object::CreateFlags   &,
            const char                  *a_val
        );
        virtual ~ObjectDir();

        DEFINE_PROP(Dirs, ObjectVectorSptr);

        // interface
        virtual string              getType();
        virtual void                do_init_props();
        virtual void                do_init_api();
        virtual void                getAllProps(
            ObjectMapSptr a_props
        );
        virtual ObjectStringSptr    toString();
        virtual string              serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );
        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        );
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        );
        virtual SV *    toPerl();

        // generic
        void             setPath(const char *);
        bool             isAbsolutePath();
//        bool             isDirectory();
//        bool             isFile();
//        ObjectVectorSptr getDirs();
//        ObjectStringSptr getDir();
//        ObjectStringSptr getFileName();
        void             addPath(ObjectDirSptr);
        void             addPath(ObjectStringSptr);
        void             addDir(const char *);
        void             addDir(ObjectStringSptr);
        void             addDir(const string &);

        int32_t          mkdir(
            const char      *a_sub_path  = "",
            const int32_t   &a_recursive = 1,
            const int32_t   &a_mode      = 0775
        );
//        bool            isExistDir();
//        bool            isExistFile();
        bool            isExist();
        FILE *          fopen(
            const char *a_fname,
            const char *a_mode
        );
        int32_t             unlink(const char *a_fname);
        ObjectStringSptr    read(const char *a_fname);

        // static
        static bool    s_isExistDir(const char *);

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        void    do_init(const Object::CreateFlags &a_flags);
};

#endif

