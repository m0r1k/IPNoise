#include "class.hpp"
DEFINE_CLASS(ObjectString);

#ifndef OBJECT_STRING_HPP
#define OBJECT_STRING_HPP

#include <string>
#include <vector>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectUnique);

using namespace std;

class ObjectString
    :   public Object
{
    public:
        ObjectString(const CreateFlags &a_create_flags);
        virtual ~ObjectString();

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
        virtual string      toStringStd();
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual int32_t     loadAsProp(const char *a_dir);
        virtual int32_t     saveAsProp(const char *a_dir);

        // generic
        bool                empty();
        uint64_t            size();
        void                assign(const string &);
        const char *        c_str() noexcept;
        char                at(const int32_t   &);
        void                toLower();

        void                add(
            const string        &a_val,
            ObjectUniqueSptr    a_flags = ObjectUniqueSptr()
        );
        void                add(
            const char          &a_val,
            ObjectUniqueSptr    a_flags = ObjectUniqueSptr()
        );
        void                add(
            const char          *a_buff,
            const uint64_t      &a_len,
            ObjectUniqueSptr    a_flags = ObjectUniqueSptr()
        );
        void                add(
            ObjectStringSptr    a_val,
            ObjectUniqueSptr    a_flags = ObjectUniqueSptr()
        );

        void                clear();

        ObjectStringSptr    substr(
            const uint64_t  &a_pos,
            const uint64_t  &a_len
        );
        ObjectStringSptr    substr(
            const uint64_t  &a_pos
        );

        string              getVal();
        string              hexdump();

        operator            string();

        // static
        static void s_split(
            const char      *a_str,
            const char      *a_delim,
            vector<string>  &a_out,
            const int32_t   &a_strict = 1
        );
        static string s_align_len(
            const char      *a_str,
            const uint32_t  a_len,
            const char      a_symbol = ' '
        );
        static void s_chomp(string &);
        static void s_chomp2(string &);
        static void s_removeSpacesFromStart(string &);
        static void s_removeSpacesFromEnd(string &);
        static void s_removeSpaces(
            string          &,
            const int32_t   &a_from_start    = 1,
            const int32_t   &a_from_end      = 1
        );

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        string m_val;
};

#endif

