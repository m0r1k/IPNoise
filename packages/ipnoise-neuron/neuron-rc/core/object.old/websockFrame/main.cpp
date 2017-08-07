#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/bin/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/uint32/main.hpp"
#include "core/object/uint64/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include <ipnoise-common/log_common.h>
#include "thread.hpp"
#include "db.hpp"
#include "db/mongo.hpp"
#include "base64.hpp"

#include "core/object/websockFrame/main.hpp"

ObjectWebsockFrame::ObjectWebsockFrame(
    const Object::CreateFlags   &a_flags)
    :   Object(Object::CreateFlags(
            a_flags | Object::REMOVE_WHEN_LINKS_LOST_FORCE
        ))
{
    do_init(a_flags);
};

ObjectWebsockFrame::~ObjectWebsockFrame()
{
};

void ObjectWebsockFrame::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebsockFrame::getType()
{
    return ObjectWebsockFrame::_getType();
}

void ObjectWebsockFrame::clear()
{
    ObjectInt32Sptr  prop_parse_state;
    ObjectMapSptr    prop_flags;
    ObjectUint32Sptr prop_opcode;
    ObjectInt32Sptr  prop_mask_exist;
    ObjectUint64Sptr prop_body_length;
    ObjectVectorSptr prop_body_mask;
    ObjectBinSptr    prop_body_data;

    PROP_INT32(prop_parse_state);
    PROP_MAP(prop_flags);
    PROP_UINT32(prop_opcode);
    PROP_INT32(prop_mask_exist);
    PROP_UINT64(prop_body_length);
    PROP_VECTOR(prop_body_mask);
    PROP_BIN(prop_body_data);

    INIT_PROP(this, ParseState, prop_parse_state);
    INIT_PROP(this, Flags,      prop_flags);
    INIT_PROP(this, Opcode,     prop_opcode);
    INIT_PROP(this, MaskExist,  prop_mask_exist);
    INIT_PROP(this, BodyLength, prop_body_length);
    INIT_PROP(this, BodyMask,   prop_body_mask);
    INIT_PROP(this, BodyData,   prop_body_data);
}

void ObjectWebsockFrame::do_init_props()
{
    Object::do_init_props();
    clear();
}

void ObjectWebsockFrame::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("thread.io.read",  ObjectWebsockFrame::actionIoRead);
}

int32_t ObjectWebsockFrame::do_autorun()
{
    int32_t err = -1;

    // all ok
    err = 0;

    return err;
}

void ObjectWebsockFrame::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("parse_state",    getParseState);
    SAVE_PROP("flags",          getFlags);
    SAVE_PROP("opcode",         getOpcode);
    SAVE_PROP("mask_exist",     getMaskExist);
    SAVE_PROP("body_length",    getBodyLength);
    SAVE_PROP("body_mask",      getBodyMask);
    SAVE_PROP("body_data",      getBodyData);

    Object::getAllProps(a_props);
}

int32_t ObjectWebsockFrame::parseBSON(
    mongo::BSONObj  a_obj)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;

    // parse state
    if (a_obj.hasField("parse_state")){
        int32_t         parse_state = 0;
        ObjectInt32Sptr prop_parse_state;

        parse_state = a_obj.getIntField("parse_state");
        PROP_INT32(
            prop_parse_state,
            parse_state
        );
        INIT_PROP(this, ParseState, prop_parse_state);
    }

    // flags
    if (a_obj.hasField("flags")){
        ObjectMapSptr       flags;
        mongo::BSONElement  el;
        mongo::BSONObj      flags_obj;

        PROP_MAP(flags);

        el = a_obj.getField("flags");
        if (mongo::BSONType::Object != el.type()){
            PERROR("cannot parse 'flags' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(flags_obj);

        DbMongo::bsonToProps(
            flags_obj,
            flags
        );

        INIT_PROP(this, Flags, flags);
    }

    // opcode
    if (a_obj.hasField("opcode")){
        uint32_t            opcode = 0;
        ObjectUint32Sptr    prop_opcode;

        opcode = a_obj.getIntField("opcode");
        PROP_UINT32(
            prop_opcode,
            opcode
        );
        INIT_PROP(this, Opcode, prop_opcode);
    }

    // is mask exist
    if (a_obj.hasField("mask_exist")){
        int32_t             mask_exist = 0;
        ObjectInt32Sptr     prop_mask_exist;

        mask_exist = a_obj.getIntField("mask_exist");
        PROP_INT32(
            prop_mask_exist,
            mask_exist
        );
        INIT_PROP(this, MaskExist, prop_mask_exist);
    }

    // body length
    if (a_obj.hasField("body_length")){
        uint64_t            body_length = 0;
        ObjectUint64Sptr    prop_body_length;

        body_length = a_obj.getField("body_length").Long();
        PROP_UINT64(
            prop_body_length,
            body_length
        );

        INIT_PROP(this, BodyLength, prop_body_length);
    }

    // body mask
    if (a_obj.hasField("body_mask")){
        ObjectVectorSptr    body_mask;
        mongo::BSONElement  el;
        mongo::BSONObj      body_mask_obj;

        PROP_VECTOR(body_mask);

        el = a_obj.getField("body_mask");
        if (mongo::BSONType::Array != el.type()){
            PERROR("cannot parse 'body_mask' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(body_mask_obj);

        DbMongo::bsonToProps(
            body_mask_obj,
            body_mask
        );

        INIT_PROP(this, BodyMask, body_mask);
    }

    // body data
    if (a_obj.hasField("body_data")){
        ObjectBinSptr       prop_body_data;
        const char          *data_ptr   = NULL;
        int32_t             data_len    = 0;
        string              val;
        mongo::BSONElement  el;

        el = a_obj.getField("body_data");
        if (mongo::BSONType::BinData != el.type()){
            PERROR("cannot parse 'body_data' attribute,"
                " object dump: '%s'\n",
                a_obj.toString().c_str()
            );
            goto fail;
        }

        data_ptr = el.binData(data_len);
        if (    data_ptr
            &&  (0 < data_len))
        {
            val.assign(data_ptr, data_len);
        }

        PROP_BIN(prop_body_data, val);

        INIT_PROP(this, BodyData, prop_body_data);
    }

    err = Object::parseBSON(a_obj);
    if (err){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void ObjectWebsockFrame::checkFirstByte(
    ObjectWebsockFrameSptr  a_frame,
    ObjectBinSptr           a_data,
    const string            &a_prefix)
{

    lock(a_frame->m_mutex, a_data->m_mutex);
    lock_guard<recursive_mutex> guard1(a_frame->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_data->m_mutex,  adopt_lock);

    uint8_t val = a_data->at(0);

    if (0x81 == val){
        goto out;
    }

    PFATAL(
        "---------- first%sbyte is not 0x81 (0x%x) ----------\n"
        "--- data for parse ---\n"
        "%s\n"
        "--- frame data ---\n"
        "%s\n",
        a_prefix.c_str(),
        val,
        a_data->hexdump().c_str(),
        a_frame->getBodyData()->hexdump().c_str()
    );

out:
    return;
}

//
// some docs
// http://learn.javascript.ru/websockets
// https://tools.ietf.org/html/rfc6455
//
// Attention: significant bit comes first!
// in rfc bits order is 01234567
// in real bits order is 76543210
// (FIN is 7th bit)
//
void ObjectWebsockFrame::parse(
    ObjectWebsockFrameSptr  a_frame,
    ObjectBinSptr           a_data)
{
    lock(a_frame->m_mutex, a_data->m_mutex);
    lock_guard<recursive_mutex> guard1(a_frame->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_data->m_mutex,  adopt_lock);

    uint32_t            i;
    string              frame_id = a_frame->getId();
    ObjectBinSptr       prop_unparsed;
    ObjectInt32Sptr     prop_val_int32;
    ObjectInt64Sptr     prop_val_int64;
    ObjectUint32Sptr    prop_val_uint32;
    ObjectUint64Sptr    prop_val_uint64;

    PTIMING(0.030f, "attempt to parse frame ID: '%s'\n"
        "  data_len:    '%lld'\n"
        "  parse_state: '%d'\n",
        a_frame->getId().c_str(),
        a_data->size(),
        a_frame->getParseState()->getVal()
    );

    if (a_data->empty()){
        goto out;
    }

    PDEBUG(100, "attempt to parse frame ID: '%s'\n"
        "  data_len:    '%lld'\n"
        "  parse_state: '%d'\n"
        "%s\n",
        a_frame->getId().c_str(),
        a_data->size(),
        a_frame->getParseState()->getVal(),
        a_data->hexdump().c_str()
    );

    for (i = 0; i < a_data->size(); i++){
        uint8_t val         = a_data->at(i);
        int32_t parse_state = 0;

        parse_state = a_frame->getParseState()->getVal();

        PDEBUG(100, "val: 0x%2.2x, parse_state: '%d'\n",
            (unsigned char)val,
            parse_state
        );

        switch (parse_state){
            // unparsed data
            case FRAME_PARSE_STATE_DATA_UNPARSED:
                {
                    if (!prop_unparsed){
                        PROP_BIN(prop_unparsed);
                    }
                    prop_unparsed->add(val);
                }
                break;

            // data
            case FRAME_PARSE_STATE_DATA:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectStringSptr    prop_new_body_data;
                    string              new_body_data;

                    ObjectBinSptr       body_data;
                    ObjectUint64Sptr    body_len;

                    ObjectInt32Sptr     prop_mask_exist;
                    ObjectVectorSptr    prop_body_mask;
                    int32_t             mask_exist    = 0;
                    int32_t             body_mask     = 0;
                    uint8_t             new_val       = val;

                    body_data       = a_frame->getBodyData();
                    body_len        = a_frame->getBodyLength();
                    prop_mask_exist = a_frame->getMaskExist();
                    mask_exist      = prop_mask_exist->getVal();

                    if (mask_exist){
                        prop_body_mask = a_frame->getBodyMask();
                        body_mask = dynamic_pointer_cast<ObjectInt32>(
                            prop_body_mask->at(
                                body_data->size() % 4
                            )
                        )->getVal();
                        new_val = val ^ uint8_t(body_mask);
                    }

                    // add to buffer
                    body_data->add(new_val);
                    PDEBUG(100, "XOR:\n"
                        "  i:                    '%d' (of: '%lld')\n"
                        "  body len:             '%lld'\n"
                        "  body data len         '%lld'\n"
                        "  body data (len %% 4): '%lld'\n"
                        "  val:                  '%c'\n"
                        "  new_val:              '%c'\n"
                        "  isAllReceived():      '%d'\n",
                        i,
                        a_data->size(),
                        body_len->getVal(),
                        body_data->size(),
                        (body_data->size() % 4),
                        val,
                        new_val,
                        a_frame->isAllReceived()
                    );
                    if (a_frame->isAllReceived()){
                        // go ahead
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_DATA_UNPARSED
                        );
                        a_frame->setParseState(prop_new_state);
                    }
                }
                break;

            // 4 bytes mask
            case FRAME_PARSE_STATE_MASK_4_OF_4:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint32Sptr    prop_new_body_mask;
                    ObjectVectorSptr    prop_body_mask;

                    // update body mask
                    prop_body_mask = a_frame->getBodyMask();
                    prop_body_mask->add(val);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_DATA
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_MASK_3_OF_4:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint32Sptr    prop_new_body_mask;
                    ObjectVectorSptr    prop_body_mask;

                    // update body mask
                    prop_body_mask = a_frame->getBodyMask();
                    prop_body_mask->add(val);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_MASK_4_OF_4
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_MASK_2_OF_4:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint32Sptr    prop_new_body_mask;
                    ObjectVectorSptr    prop_body_mask;

                    // update body mask
                    prop_body_mask = a_frame->getBodyMask();
                    prop_body_mask->add(val);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_MASK_3_OF_4
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_MASK_1_OF_4:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint32Sptr    prop_new_body_mask;
                    ObjectVectorSptr    prop_body_mask;

                    // update body mask
                    prop_body_mask = a_frame->getBodyMask();
                    prop_body_mask->add(val);

                    // update state
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_MASK_2_OF_4
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            // 8 bytes length
            case FRAME_PARSE_STATE_LEN_8_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;
                    ObjectInt32Sptr     prop_mask_exist;
                    int32_t             mask_exist = 0;

                    // is mask exist?
                    prop_mask_exist = dynamic_pointer_cast<ObjectInt32>(
                        a_frame->getMaskExist()
                    );
                    mask_exist = prop_mask_exist->getVal();

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 0);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    if (mask_exist){
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_MASK_1_OF_4
                        );
                        a_frame->setParseState(prop_new_state);
                    } else {
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_DATA
                        );
                        a_frame->setParseState(prop_new_state);
                    }
                }
                break;

            case FRAME_PARSE_STATE_LEN_7_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 8);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_8_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_6_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 16);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_7_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_5_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 24);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_6_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_4_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 32);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_5_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_3_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 40);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_4_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_2_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 48);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_3_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            case FRAME_PARSE_STATE_LEN_1_OF_8:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 56);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // update state
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_2_OF_8
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            // 2 bytes length
            case FRAME_PARSE_STATE_LEN_2_OF_2:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;
                    ObjectInt32Sptr     prop_mask_exist;
                    int32_t             mask_exist = 0;

                    // is mask exist?
                    prop_mask_exist = dynamic_pointer_cast<ObjectInt32>(
                        a_frame->getMaskExist()
                    );
                    mask_exist = prop_mask_exist->getVal();

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 0);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // go ahead
                    if (mask_exist){
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_MASK_1_OF_4
                        );
                        a_frame->setParseState(prop_new_state);
                    } else {
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_DATA
                        );
                        a_frame->setParseState(prop_new_state);
                    }
                }
                break;

            case FRAME_PARSE_STATE_LEN_1_OF_2:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectUint64Sptr    prop_new_body_length;
                    uint64_t            body_length = 0;

                    // update body length
                    body_length = a_frame->getBodyLength()->getVal();
                    body_length += (uint64_t(val) << 8);

                    PROP_UINT64(
                        prop_new_body_length,
                        body_length
                    );
                    a_frame->setBodyLength(prop_new_body_length);

                    // update state
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_2_OF_2
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;

            // parse body length
            case FRAME_PARSE_STATE_LEN_1_OF_1:
                {
                    ObjectInt32Sptr prop_new_state;
                    int32_t         mask_exist  = 0;
                    int32_t         body_len    = 0;

                    // is mask exist?
                    mask_exist = (val & FRAME_MASK) ? 1 : 0;
                    PROP_INT32(prop_val_int32, mask_exist);
                    a_frame->setMaskExist(prop_val_int32);

                    // body len
                    body_len = (val & FRAME_LEN);
                    if (126 == body_len){
                        // additional 2 bytes of body length
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_LEN_1_OF_2
                        );
                        a_frame->setParseState(prop_new_state);
                    } else if (127 == body_len){
                        // additional 8 bytes of body length
                        PROP_INT32(
                            prop_new_state,
                            FRAME_PARSE_STATE_LEN_1_OF_8
                        );
                        a_frame->setParseState(prop_new_state);
                    } else {
                        // <= 125
                        PROP_UINT64(prop_val_uint64, body_len);
                        a_frame->setBodyLength(prop_val_uint64);

                        // go ahead
                        if (mask_exist){
                            PROP_INT32(
                                prop_new_state,
                                FRAME_PARSE_STATE_MASK_1_OF_4
                            );
                            a_frame->setParseState(prop_new_state);
                        } else {
                            PROP_INT32(
                                prop_new_state,
                                FRAME_PARSE_STATE_DATA
                            );
                            a_frame->setParseState(prop_new_state);
                        }
                    }
                }
                break;

            case FRAME_PARSE_STATE_FIRST_BYTE:
                {
                    ObjectInt32Sptr     prop_new_state;
                    ObjectMapSptr       flags;
                    uint32_t            opcode = 0;

                    // check first byte
                    ObjectWebsockFrame::checkFirstByte(
                        a_frame,
                        a_data
                    );

                    PROP_MAP(flags);

                    // fin
                    PROP_INT32(
                        prop_val_int32,
                        (val & FRAME_FIN) ? 1 : 0
                    );
                    flags->add("fin", prop_val_int32);

                    if (!(val & FRAME_FIN)){
                        PFATAL("sorry, fragmetation yet"
                            " not supported\n"
                            "%s\n",
                            Utils::dump(
                                a_data->c_str(),
                                a_data->size()
                            ).c_str()
                        );
                    }

                    // rsv1
                    PROP_INT32(
                        prop_val_int32,
                        (val & FRAME_RSV1) ? 1 : 0
                    );
                    flags->add("rsv1", prop_val_int32);

                    // rsv2
                    PROP_INT32(
                        prop_val_int32,
                        (val & FRAME_RSV2) ? 1 : 0
                    );
                    flags->add("rsv2", prop_val_int32);

                    // rsv3
                    PROP_INT32(
                        prop_val_int32,
                        (val & FRAME_RSV3) ? 1 : 0
                    );
                    flags->add("rsv3", prop_val_int32);

                    a_frame->setFlags(flags);

                    // opcode
                    opcode = (val & FRAME_OPCODE);
                    PROP_UINT32(
                        prop_val_uint32,
                        opcode
                    );
                    a_frame->setOpcode(prop_val_uint32);

                    // go ahead
                    PROP_INT32(
                        prop_new_state,
                        FRAME_PARSE_STATE_LEN_1_OF_1
                    );
                    a_frame->setParseState(prop_new_state);
                }
                break;


            default:
                PFATAL("unsupported state: '%d'\n",
                    parse_state
                );
                break;
        }
    }

    PDEBUG(100, "parse frame ID: '%s'\n"
        "  parse data: '%s'\n"
        "  frame:      '%s'\n",
        frame_id.c_str(),
        a_data->c_str(),
        a_frame->toString().c_str()
    );

    if (a_frame->isAllReceived()){
        // frame parsing finished
        // prepare new frame
        ObjectActionSptr        action;
        ObjectBinSptr           prop_bin;
        ObjectSessionSptr       session;

        session = Object::getNeigh<ObjectSession>(
            a_frame,
            "",                     // name
            "core.object.session",  // type
            0,                      // get removed
            0                       // db lock
        );

        PWARN("all received, getBodyLength(): '%lld'\n",
            a_frame->getBodyLength()->getVal()
        );

        if (0){
            PTIMING(0.0f, "send answer\n");

            ObjectActionSptr    new_action;

            OBJECT_ACTION(new_action, "frame.parsed");
            new_action->addParam("data", a_frame->getBodyData());

            Object::addNeigh(new_action, session, 0);
            new_action->schedule();
            Object::save(new_action);
        }

        if (1){
            // create 'answer'
            int32_t                 res, fd = -1;
            ObjectWebsockFrameSptr  prop_frame;
            ObjectBinSptr           prop_bin;
            string                  dump;

            PTIMING(0.0f, "send answer\n");

            OBJECT_WEBSOCK_FRAME(prop_frame);

            prop_frame->setFlag("fin");
            prop_frame->setOpcode(1);
            prop_frame->setBodyLength(a_frame->getBodyLength());
            prop_frame->setBodyData(a_frame->getBodyData());

            prop_bin = prop_frame->serializeBin();
            dump     = prop_bin->hexdump();

            PDEBUG(60, "send answer:\n%s\n", dump.c_str());

            fd  = session->getFd()->getVal();
            res = ::write(fd, prop_bin->c_str(), prop_bin->size());

            PDEBUG(60, "after write to fd: '%d', res: '%d'\n",
                fd,
                res
            );

            //prop_frame->remove(1);
        }

/*
        if (0){
            // create 'done' frame
            ObjectActionSptr        action;
            ObjectWebsockFrameSptr  frame;

            OBJECT_WEBSOCK_FRAME(frame);

            frame->setName("done");
            frame->setFlags(a_frame->getFlags());
            frame->setOpcode(a_frame->getOpcode());
            frame->setMaskExist(a_frame->getMaskExist());
            frame->setBodyLength(a_frame->getBodyLength());
            frame->setBodyMask(a_frame->getBodyMask());
            frame->setBodyData(a_frame->getBodyData());
            Object::save(frame);

            OBJECT_ACTION(
                action,
                "core.object.websock.frame.parsed"
            );
            Object::addNeigh(action, frame);
            Object::addNeigh(action, session);
            action->schedule();
            Object::save(action);
        }
*/

        // clear frame
        a_frame->clear();

        if (prop_unparsed){
            // check first byte in unparsed data
            ObjectWebsockFrame::checkFirstByte(
                a_frame,
                prop_unparsed,
                " unparsed "
            );
            // we have unparsed data, parse
            ObjectWebsockFrame::parse(a_frame, prop_unparsed);
        }
    }

/*
        ObjectMapSptr       props;
        ObjectStringSptr    prop_ev_image;

        PROP_MAP(props);
        parseJSON(getBodyData(), props);

        prop_ev_image = dynamic_pointer_cast<ObjectString>(
            props->get("image")
        );

        if (prop_ev_image){
            char             buffer[1024] = { 0x00 };
            FILE             *file        = NULL;
            ObjectTimeSptr   prop_time;
            string           tmp;

            PROP_TIME_NOW(prop_time);

            snprintf(buffer, sizeof(buffer),
                "/tmp/myfile_%s.bin",
                prop_time->toString().c_str()
            );

            file = fopen(buffer, "w");
            if (!file){
                PFATAL("cannot open: '%s' for write\n",
                    buffer
                );
            }

            // skip "data:image/jpeg;base64,"
            tmp = base64_decode(
                prop_ev_image->toString().substr(
                    string("data:image/jpeg;base64,").size()
                )
            );

            fwrite(tmp.c_str(), 1, tmp.size(), file);
            fclose(file);
        }
*/

out:
    Object::save(a_frame);
    return;
}

void ObjectWebsockFrame::parseJSON(
    ObjectStringSptr    a_data,
    ObjectMapSptr       a_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t res;

    res = isFinished();
    if (!res){
        PFATAL("call parseJSON, when frame not finished\n");
    }

    if (    !a_data
        ||  a_data->empty())
    {
        PFATAL("%s, empty data\n",
            getId().c_str()
        );
        goto out;
    }

    try {
        PDEBUG(60, "parse json: '%s'\n", a_data->c_str());
        mongo::BSONObj obj = mongo::fromjson(a_data->c_str());
        DbMongo::bsonToProps(
            obj,
            a_out
        );
    } catch (...) {
        PERROR("cannot parse: '%s',"
            " size: '%lld'\n",
            a_data->c_str(),
            a_data->size()
        );
    }

out:
    return;
}

bool ObjectWebsockFrame::isAllReceived()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    bool                ret = false;
    ObjectBinSptr       body_data;
    ObjectUint64Sptr    prop_body_length;
    uint64_t            body_length = 0;

    body_data        = getBodyData();
    prop_body_length = getBodyLength();
    body_length      = prop_body_length->getVal();

    if (    0 < body_length
        &&  body_length == body_data->size())
    {
        ret = true;
    }

    //PWARN("body_length: '%lld',"
    //    " body_data->size(): '%lld',"
    //    " ret: '%d'\n",
    //    body_length,
    //    body_data->size(),
    //    ret
    //);

    return ret;
}

bool ObjectWebsockFrame::isFinished()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    bool            ret     = false;
    ObjectMapSptr   flags   = getFlags();

    if (isAllReceived()){
        ret = true;
    }

    return ret;
}

void ObjectWebsockFrame::setFlag(
    const string &a_flag_name)
{
    lock_guard<recursive_mutex> guard(mutex);

    ObjectMapSptr flags;

    flags = getFlags();
    flags->add(a_flag_name, 1);
}

void ObjectWebsockFrame::resetFlag(
    const string &a_flag_name)
{
    lock_guard<recursive_mutex> guard(mutex);

    ObjectMapSptr flags;

    flags = getFlags();
    flags->add(a_flag_name, 0);
}

bool ObjectWebsockFrame::getFlag(
    const string &a_flag_name)
{
    lock_guard<recursive_mutex> guard(mutex);

    ObjectMapSptr   flags;
    ObjectInt32Sptr val;
    int32_t         ret = 0;

    flags  = getFlags();
    val    = dynamic_pointer_cast<ObjectInt32>(
        flags->get(a_flag_name)
    );

    if (val){
        // :)
        ret = val->getVal();
    }

    return ret ? 1 : 0;
}

void ObjectWebsockFrame::setOpcode(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(mutex);

    ObjectUint32Sptr prop_val;
    PROP_UINT32(prop_val, a_val);
    setOpcode(prop_val);
}

ObjectBinSptr ObjectWebsockFrame::serializeBin()
{
    lock_guard<recursive_mutex> guard(mutex);

    ObjectBinSptr   bin;
    bool            fin  = 0;
    bool            rsv1 = 0;
    bool            rsv2 = 0;
    bool            rsv3 = 0;
    int32_t         opcode;
    bool            mask_exist;
    uint64_t        len = 0;
    ObjectBinSptr   body;

    fin         = getFlag("fin");
    rsv1        = getFlag("rsv1");
    rsv2        = getFlag("rsv2");
    rsv3        = getFlag("rsv3");
    opcode      = getOpcode()->getVal();
    mask_exist  = getMaskExist()->getVal() ? 1 : 0;
    len         = getBodyLength()->getVal();
    body        = getBodyData();

    PROP_BIN(bin);

    // first byte
    bin->add(uint8_t(
            uint8_t(fin  << 7)
        +   uint8_t(rsv1 << 6)
        +   uint8_t(rsv2 << 5)
        +   uint8_t(rsv3 << 4)
        +   uint8_t(opcode & FRAME_OPCODE)
    ));

    // second byte
    if (125 >= len){
        bin->add(uint8_t(
                uint8_t(mask_exist << 7)
            +   uint8_t(len & FRAME_LEN)
        ));
    } else if (65535 >= len){
        // additional 2 bytes of body length (code 126)
        bin->add(uint8_t(
                uint8_t(mask_exist << 7)
            +   uint8_t(126 & FRAME_LEN)
        ));
        bin->add(uint8_t(
            (len & 0xff00) >> 8
        ));
        bin->add(uint8_t(
            (len & 0x00ff) >> 0
        ));
    } else if (65535 < len){
        // additional 8 bytes of body length (code 127)
        bin->add(uint8_t(
                uint8_t(mask_exist << 7)
            +   uint8_t(127 & FRAME_LEN)
        ));
        bin->add(uint8_t(
            uint64_t(len & 0xff00000000000000) >> 56
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x00ff000000000000) >> 48
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x0000ff0000000000) >> 40
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x000000ff00000000) >> 32
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x00000000ff000000) >> 24
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x0000000000ff0000) >> 16
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x000000000000ff00) >> 8
        ));
        bin->add(uint8_t(
            uint64_t(len & 0x00000000000000ff) >> 0
        ));
    }

    bin->add(body->getVal());

    //PWARN("bin->length: '%lld'\n", bin->size());

    return bin;
}

// ---------------- static ----------------

void ObjectWebsockFrame::actionIoRead(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    ThreadSptr          cur_thread  = Thread::getCurThread();
    ObjectContextSptr   context     = cur_thread->contextGet();

    ObjectWebsockFrameSptr  frame;
    string                  frame_id;
    ObjectBinSptr           prop_data;

    frame       = dynamic_pointer_cast<ObjectWebsockFrame>(a_object);
    frame_id    = frame->getId();

    // get data
    prop_data = dynamic_pointer_cast<ObjectBin>(
        a_action->getParam("data")
    );

    if (prop_data->empty()){
        goto out;
    }

    ObjectWebsockFrame::parse(frame, prop_data);

/*
    if (prop_data->size()){
        // we have not parsed data
        // create new frame
        ObjectWebsockFrameSptr  new_frame;
        ObjectActionSptr        action;
        new_frame = frame->getCreateNeigh<ObjectWebsockFrame>(
            "core.object.websock.frame",    // type
            "pending"                       // name
        );

        // create action
        OBJECT_ACTION(action, "thread.io.read");

        action->addParam("data", prop_data);
        new_frame->addAction(action);

        new_frame->scheduleCurThread();
        new_frame->save();
    }
*/

out:
    return;
}

// ---------------- module ----------------

string ObjectWebsockFrame::_getType()
{
    return "core.object.websock.frame";
}

string ObjectWebsockFrame::_getTypePerl()
{
    return "core::object::websockFrame::main";
}

int32_t ObjectWebsockFrame::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebsockFrame::_destroy()
{
}

ObjectSptr ObjectWebsockFrame::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBSOCK_FRAME(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebsockFrame::_getType,
    .type_perl      = ObjectWebsockFrame::_getTypePerl,
    .init           = ObjectWebsockFrame::_init,
    .destroy        = ObjectWebsockFrame::_destroy,
    .object_create  = ObjectWebsockFrame::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

