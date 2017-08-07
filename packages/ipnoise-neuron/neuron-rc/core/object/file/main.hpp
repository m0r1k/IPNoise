#include "class.hpp"
DEFINE_CLASS(ObjectFile);

#ifndef OBJECT_FILE_HPP
#define OBJECT_FILE_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectDir);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString)

using namespace std;

class ObjectFile
    :   public Object
{
    public:
        ObjectFile(const CreateFlags &a_create_flags);
        virtual ~ObjectFile();

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

        // generic
        void                setDir(const char *a_dir);
        ObjectDirSptr       getDir();
        void                setDir(ObjectDirSptr);
        void                setFileName(const char *a_name);
        ObjectStringSptr    getFileName();
        ObjectStringSptr    getFileNameEsc();
        void                setFileContent(ObjectStringSptr);
        ObjectStringSptr    getFileContent();
        int32_t             touch();
        ObjectStringSptr    load();

        // static

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectDirSptr       m_dir;
        ObjectStringSptr    m_file_name;
        ObjectStringSptr    m_file_name_esc;
        ObjectStringSptr    m_file_content;
};

#endif

