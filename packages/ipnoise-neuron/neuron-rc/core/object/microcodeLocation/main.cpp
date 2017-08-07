#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/file/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/uint32/main.hpp>

#include "main.hpp"

ObjectMicrocodeLocation::ObjectMicrocodeLocation(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectMicrocodeLocation::~ObjectMicrocodeLocation()
{
}

string ObjectMicrocodeLocation::getType()
{
    return ObjectMicrocodeLocation::s_getType();
}

int32_t ObjectMicrocodeLocation::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    PFATAL("object cannot be prop\n");

    res = Object::do_init_as_prop(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectMicrocodeLocation::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    // don't forget update 'copy' method
    m_file = OBJECT<ObjectString>();
    m_data = OBJECT<ObjectString>();
    m_line = OBJECT<ObjectUint32>();
    m_pos  = OBJECT<ObjectUint32>();

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void ObjectMicrocodeLocation::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        // don't forget update 'copy' method
        a_out->add("microcode_location_file", m_file);
        a_out->add("microcode_location_data", m_data);
        a_out->add("microcode_location_line", m_line);
        a_out->add("microcode_location_pos",  m_pos);
        Object::getAllProps(a_out);
    }
}

// ---------------- file ----------------

ObjectStringSptr ObjectMicrocodeLocation::getFile()
{
    return m_file;
}

void ObjectMicrocodeLocation::setFile(
    const char *a_val)
{
    m_file = OBJECT<ObjectString>(a_val);
}

// ---------------- data ----------------

ObjectStringSptr ObjectMicrocodeLocation::getData()
{
    return m_data;
}

void ObjectMicrocodeLocation::setData(
    ObjectStringSptr    a_val)
{
    m_data = a_val;
}

// ---------------- line ----------------

ObjectUint32Sptr ObjectMicrocodeLocation::getLine()
{
    return m_line;
}

void ObjectMicrocodeLocation::setLine(
    ObjectUint32Sptr    a_val)
{
    m_line = a_val;
}

void ObjectMicrocodeLocation::setLine(
    const uint32_t  &a_val)
{
    m_line = OBJECT<ObjectUint32>(a_val);
}

// ---------------- pos ----------------

ObjectUint32Sptr ObjectMicrocodeLocation::getPos()
{
    return m_pos;
}

void ObjectMicrocodeLocation::setPos(
    ObjectUint32Sptr    a_val)
{
    m_pos = a_val;
}

void ObjectMicrocodeLocation::setPos(
    const uint32_t  &a_val)
{
    m_pos = OBJECT<ObjectUint32>(a_val);
}

// ---------------- copy ----------------

ObjectSptr ObjectMicrocodeLocation::copy()
{
    ObjectMicrocodeLocationSptr ret;

    ret = OBJECT<ObjectMicrocodeLocation>();

    ret->setFile(m_file->c_str());
    ret->setData(m_data); // don't copy m_data
    ret->setLine(m_line->getVal());
    ret->setPos(m_pos->getVal());

    PWARN("after line: '%u',"
        " pos: '%u',"
        " copy: '%s'\n",
        m_line->getVal(),
        m_pos->getVal(),
        ret->toString()->c_str()
    );

    return ret;
}

char ObjectMicrocodeLocation::readChar()
{
    char        c;
    uint32_t    pos = m_pos->getVal();

    c = m_data->at(pos);

    m_pos->incVal();

    if ('\n' == c){
        m_line->incVal();
    }

    return c;
}

ObjectStringSptr ObjectMicrocodeLocation::toString()
{
    ObjectStringSptr    ret;
    ObjectStringSptr    near_line;
    char                buffer[65535] = { 0x00 };
    uint32_t            line_pos      = 0;
    uint32_t            file_pos      = m_pos->getVal();

    if (!m_data){
        PFATAL("m_data is NULL\n");
    }

    ret       = OBJECT<ObjectString>();
    near_line = getNearLine();
    line_pos  = file_pos - getNearLineStart(file_pos);

    snprintf(buffer, sizeof(buffer),
        "file: '%s',"
        " line: '%u',"
        " line pos: '%u',"
        " file pos: '%u'\n"
        "%s\n",
        m_file->c_str(),
        m_line->getVal(),
        line_pos,
        m_pos->getVal(),
        near_line->c_str()
    );

    ret->add(buffer);

    return ret;
}

uint32_t ObjectMicrocodeLocation::getNearLineStart(
    const uint32_t &a_pos)
{
    uint32_t pos = a_pos;

    if (!m_data){
        PFATAL("m_data is NULL\n");
    }

    // search start of line
    while (0 < pos){
        char c = m_data->at(--pos);
        if (    '\0' == c
            ||  '\n' == c)
        {
            pos++;
            break;
        }
    }

    return pos;
}

uint32_t ObjectMicrocodeLocation::getNearLineEnd(
    const uint32_t &a_pos)
{
    uint32_t    pos        = a_pos;
    uint64_t    data_size  = m_data->size();

    if (!m_data){
        PFATAL("m_data is NULL\n");
    }

    // search start of line
    while (pos < data_size){
        char c = m_data->at(pos++);
        if (    '\0' == c
            ||  '\n' == c)
        {
            break;
        }
    }

    return pos;
}

ObjectStringSptr ObjectMicrocodeLocation::getNearLine()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr    ret;

    uint32_t    i;
    uint32_t    start = 0;
    uint32_t    end   = 0;
    uint32_t    pos   = m_pos->getVal();

    if (!m_data){
        PFATAL("m_data is NULL\n");
    }

    // search start of line
    start = getNearLineStart(pos);

    // search end of line
    end = getNearLineEnd(pos);

    ret = m_data->substr(start, end - start);

    for (i = start; i < pos; i++){
        ret->add(" ");
    }
    ret->add("^-- here");

    PDEBUG(50, "start: '%u', end: '%u'\n"
        "%s\n",
        start,
        end,
        ret->c_str()
    );

    return ret;
}

// ---------------- static ----------------

// ---------------- tests ----------------

// ---------------- module ----------------

string ObjectMicrocodeLocation::s_getType()
{
    return "core.object.microcode.location";
}

int32_t ObjectMicrocodeLocation::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectMicrocodeLocation::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectMicrocodeLocation::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectMicrocodeLocation(
        a_create_flags
    ));

    return object;
}

void ObjectMicrocodeLocation::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_microcode_location = {
    .getType        = ObjectMicrocodeLocation::s_getType,
    .init           = ObjectMicrocodeLocation::s_init,
    .shutdown       = ObjectMicrocodeLocation::s_shutdown,
    .objectCreate   = ObjectMicrocodeLocation::s_objectCreate,
    .getTests       = ObjectMicrocodeLocation::s_getTests
};

