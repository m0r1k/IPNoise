#include "class.hpp"
DEFINE_CLASS(ObjectWebsockFrame);

#ifndef OBJECT_WEBSOCK_FRAME_HPP
#define OBJECT_WEBSOCK_FRAME_HPP

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
DEFINE_CLASS(ObjectBin);
DEFINE_CLASS(ObjectBodyData);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectUint32);
DEFINE_CLASS(ObjectUint64);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_WEBSOCK_FRAME(a_out, a_flags, ...)          \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectWebsockFrame,                                 \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_WEBSOCK_FRAME(a_out, a_flags, ...)            \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectWebsockFrame,                                 \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_WEBSOCK_FRAME(a_out, ...)                    \
    CREATE_OBJECT(a_out, ObjectWebsockFrame, ##__VA_ARGS__ )

#define PROP_WEBSOCK_FRAME(a_out, ...)                      \
    CREATE_PROP(a_out, ObjectWebsockFrame, ##__VA_ARGS__ )

class ObjectWebsockFrame
    :   public  Object
{
    public:
        ObjectWebsockFrame(const Object::CreateFlags &);
        virtual ~ObjectWebsockFrame();

        enum FrameBits {
            // first byte
            FRAME_FIN       = (1    << 7),
            FRAME_RSV1      = (1    << 6),
            FRAME_RSV2      = (1    << 5),
            FRAME_RSV3      = (1    << 4),
            FRAME_OPCODE    = 0x0f,
            // second byte
            FRAME_MASK      = (1    << 7),
            FRAME_LEN       = 0x7f,
        };

        enum ParseState {
            FRAME_PARSE_STATE_FIRST_BYTE    = 0,
            // 1 byte len
            FRAME_PARSE_STATE_LEN_1_OF_1,
            // 2 byte len
            FRAME_PARSE_STATE_LEN_1_OF_2,
            FRAME_PARSE_STATE_LEN_2_OF_2,
            // 8 byte len
            FRAME_PARSE_STATE_LEN_1_OF_8,   // 4
            FRAME_PARSE_STATE_LEN_2_OF_8,
            FRAME_PARSE_STATE_LEN_3_OF_8,
            FRAME_PARSE_STATE_LEN_4_OF_8,
            FRAME_PARSE_STATE_LEN_5_OF_8,
            FRAME_PARSE_STATE_LEN_6_OF_8,
            FRAME_PARSE_STATE_LEN_7_OF_8,
            FRAME_PARSE_STATE_LEN_8_OF_8,
            // mask
            FRAME_PARSE_STATE_MASK_1_OF_4,  // 12
            FRAME_PARSE_STATE_MASK_2_OF_4,
            FRAME_PARSE_STATE_MASK_3_OF_4,
            FRAME_PARSE_STATE_MASK_4_OF_4,
            // data
            FRAME_PARSE_STATE_DATA,         // 16
            // data unparsed
            FRAME_PARSE_STATE_DATA_UNPARSED
        };

//        DEFINE_PROP(Info,       ObjectMapSptr);
        DEFINE_PROP(ParseState, ObjectInt32Sptr);
        DEFINE_PROP(Flags,      ObjectMapSptr);
        DEFINE_PROP(Opcode,     ObjectUint32Sptr);
        DEFINE_PROP(MaskExist,  ObjectInt32Sptr);
        DEFINE_PROP(BodyLength, ObjectUint64Sptr);
        DEFINE_PROP(BodyMask,   ObjectVectorSptr);
        DEFINE_PROP(BodyData,   ObjectBinSptr);

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
        void    clear();

        static void checkFirstByte(
            ObjectWebsockFrameSptr  a_frame,
            ObjectBinSptr           a_data,
            const string            &a_prefix = " "
        );

        static void parse(
            ObjectWebsockFrameSptr  a_frame,
            ObjectBinSptr           a_data
        );

        void    parseJSON(
            ObjectStringSptr    a_data,
            ObjectMapSptr       a_out
        );
        bool    isAllReceived();
        bool    isFinished();
        void    setFlag(const string &a_flag_name);
        void    resetFlag(const string &a_flag_name);
        bool    getFlag(const string &a_flag_name);

        void            setOpcode(const int32_t &a_val);
        ObjectBinSptr   serializeBin();

        // static

        // api
        static void     actionIoRead(
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

