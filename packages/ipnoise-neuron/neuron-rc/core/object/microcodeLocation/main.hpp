#include "class.hpp"
DEFINE_CLASS(ObjectMicrocodeLocation);

#ifndef OBJECT_MICROCODE_LOCATION_HPP
#define OBJECT_MICROCODE_LOCATION_HPP

#include <string>
#include <vector>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectUint32);
DEFINE_CLASS(ObjectString);

using namespace std;

class ObjectMicrocodeLocation
    :   public Object
{
    public:
        ObjectMicrocodeLocation(
            const CreateFlags &a_create_flags
        );
        virtual ~ObjectMicrocodeLocation();

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
        virtual ObjectStringSptr    toString();

        // generic
        uint32_t            getNearLineStart(const uint32_t &);
        uint32_t            getNearLineEnd(const uint32_t &);
        ObjectStringSptr    getNearLine();
        char                readChar();

        ObjectStringSptr    getFile();
        void                setFile(const char *);
        ObjectStringSptr    getData();
        void                setData(ObjectStringSptr);
        ObjectUint32Sptr    getLine();
        void                setLine(ObjectUint32Sptr);
        void                setLine(const uint32_t &);
        ObjectUint32Sptr    getPos();
        void                setPos(ObjectUint32Sptr);
        void                setPos(const uint32_t &);

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
        // don't forget update 'copy' method
        ObjectStringSptr    m_file;
        ObjectStringSptr    m_data;
        ObjectUint32Sptr    m_line;
        ObjectUint32Sptr    m_pos;
};

#endif

