#include "class.hpp"
DEFINE_CLASS(ObjectDir);

#ifndef OBJECT_DIR_HPP
#define OBJECT_DIR_HPP

#include <string>
#include <vector>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

class ObjectDir
    :   public Object
{
    public:
        ObjectDir(const CreateFlags &a_create_flags);
        virtual ~ObjectDir();

        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);
        virtual ObjectStringSptr toString();
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );

        // generic
        int32_t file_touch(const char *a_file_name);
        FILE *  file_open(
            const char *a_fname,
            const char *a_mode
        );

        // static
        static ObjectStringSptr s_escape_file_name(
            const char *
        );
        static int32_t s_mkdir(
            const char          *a_path,
            const int32_t       &a_recursive = 0
        );
        static int32_t s_mkdir(
            ObjectVectorSptr    a_dirs,
            const int32_t       &a_recursive = 0
        );
        static int32_t s_rm(
            const char          *a_path,
            const int32_t       &a_recursive        = 0,
            const int32_t       &a_remove_top_dir   = 1
        );
        static int32_t s_recreate_dir(
            const char          *a_path
        );

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  int32_t     s_test_mkdir(const char *a_tmp_path);
        static  int32_t     s_test_mkdirr(const char *a_tmp_path);
        static  int32_t     s_test_rm(const char *a_tmp_path);
        static  int32_t     s_test_rmr(const char *a_tmp_path);
        static  void        s_getTests(Tests &);

    private:
        ObjectStringSptr m_val;
};

#endif

