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
#include <core/object/microcodeLocation/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/uint32/main.hpp>

#include "main.hpp"

ObjectMicrocode::ObjectMicrocode(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectMicrocode::~ObjectMicrocode()
{
}

string ObjectMicrocode::getType()
{
    return ObjectMicrocode::s_getType();
}

int32_t ObjectMicrocode::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_prop(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    registerOperators();

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

int32_t ObjectMicrocode::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    registerOperators();

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

ObjectSptr ObjectMicrocode::copy()
{
    ObjectMicrocodeSptr ret;

    ret = OBJECT<ObjectMicrocode>();

    return ret;
}

void ObjectMicrocode::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

void ObjectMicrocode::registerOperators()
{
    m_operators["var"] = ObjectMicrocode::s_operatorVar;
    m_operators["="]   = ObjectMicrocode::s_operatorAssign;
    m_operators[";"]   = ObjectMicrocode::s_operatorRun;
}

int32_t ObjectMicrocode::parse(
    ObjectMicrocodeLocationSptr a_location,
    ObjectMapSptr               a_vars)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMicrocodeLocationSptr location;

    int32_t     res, err    = -1;
    State       state       = STATE_OPERNAME_WAIT;
    string      opername;
    char        c;

    if (!a_location){
        PFATAL("missing argument: 'a_location'\n");
    }

    if (!a_vars){
        PFATAL("missing argument: 'a_vars'\n");
    }

    do {
        c = a_location->readChar();
        if ('\0' == c){
            break;
        }

        //PWARN("c: '%c', state: '%d'\n", c, state);

        switch (state){
            case STATE_OPERNAME_COLLECT:
                if (    ' '     != c
                    &&  '\n'    != c
                    &&  '\r'    != c)
                {
                    opername += c;
                } else {
                    OperatorsIt it;

                    it = m_operators.find(opername);
                    if (m_operators.end() == it){
                        PERROR("unsupported operator: '%s'"
                            " at %s\n",
                            opername.c_str(),
                            location->toString()->c_str()
                        );
                        goto fail;
                    }
                    res = it->second(
                        a_location,
                        a_vars
                    );
                    if (res){
                        err = res;
                        goto fail;
                    }

                    //a_vars->add("cur_opername", opername);

                    opername = "";
                    state    = STATE_OPERNAME_WAIT;

                    //PWARN("location: '%s'\n",
                    //    a_location->toString()->c_str()
                    //);
                }
                break;

            case STATE_OPERNAME_WAIT:
                if (    ' '     == c
                    ||  '\n'    == c
                    ||  '\r'    == c)
                {
                    break;
                } else {
                    state       = STATE_OPERNAME_COLLECT;
                    opername    = c;
                    location    = dynamic_pointer_cast
                        <ObjectMicrocodeLocation>(
                            a_location->copy()
                        );
                    PWARN("1 ------ %s\n", a_location->toString()->c_str());
                    PWARN("2 ------ %s\n", location->toString()->c_str());
                }
                break;

            default:
                PFATAL("unknown state: '%d'\n", state);
                break;
        }
    } while (c);

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

int32_t ObjectMicrocode::parseFile(
    const char *a_fname)
{
    int32_t res, err = -1;

    ObjectFileSptr              file;
    ObjectStringSptr            data;
    ObjectMapSptr               vars;
    ObjectMicrocodeLocationSptr location;

    file = OBJECT<ObjectFile>();
    file->setDir(".");
    file->setFileName(a_fname);

    // load file
    data = file->load();
    if (!data){
        PERROR("cannot load: '%s'\n",
            a_fname
        );
        goto fail;
    }

    // init location
    location = OBJECT<ObjectMicrocodeLocation>();
    location->setFile(a_fname);
    location->setData(data);

    // init vars
    vars = OBJECT<ObjectMap>();

    // parse
    res = parse(location, vars);
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

// ---------------- static ----------------

int32_t ObjectMicrocode::s_operatorVar(
    ObjectMicrocodeLocationSptr a_location,
    ObjectMapSptr               a_vars)
{
    int32_t     err         = -1;
    char        c           = '\0';
    State       state       = STATE_VAR_NAME_WAIT;
    string      var_name;

    ObjectMicrocodeLocationSptr location;
    location = dynamic_pointer_cast<ObjectMicrocodeLocation>(
        a_location->copy()
    );

    do {
        c = a_location->readChar();
        if ('\0' == c){
            break;
        }

        PDEBUG(50, "c: '%c', state: '%d'\n",
            c,
            state
        );

        switch (state){
            case STATE_VAR_NAME_COLLECT:
                if (    ' '  != c
                    &&  '\r' != c
                    &&  '\n' != c)
                {
                    var_name += c;
                } else {
                    ObjectMapIt it;

                    if (var_name.empty()){
                        PERROR("missing variable name\n"
                            "%s\n",
                            location->toString()->c_str()
                        );
                        goto fail;
                    }

                    //it = a_vars->find(var_name);
                    //if (a_vars->end() != it){
                    //    PERROR("attempt to define twice,"
                    //        " variable: '%s' at %s,"
                    //        " first define here: %s\n",
                    //        var_name.c_str(),
                    //        location->toString()->c_str(),
                    //        it->second->toString()->c_str()
                    //    );
                    //    goto fail;
                    //}

                    // store var
                    //a_vars->add("cur_left_location", location);
                    a_vars->add("cur_left",          var_name);

                    err = 0;
                    goto out;
                }
                break;

            case STATE_VAR_NAME_WAIT:
                if (' ' != c){
                    state       = STATE_VAR_NAME_COLLECT;
                    var_name    = c;
                }
                break;

            default:
                PFATAL("unknown state: '%d'\n", state);
                break;
        }
    } while (c);

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

int32_t ObjectMicrocode::s_operatorAssign(
    ObjectMicrocodeLocationSptr a_location,
    ObjectMapSptr               a_vars)
{
    string      var_name;
    int32_t     err         = -1;
    char        c           = '\0';
    State       state       = STATE_ASSIGN_WAIT;
    string      value;

    ObjectMicrocodeLocationSptr location;
    location = dynamic_pointer_cast<ObjectMicrocodeLocation>(
        a_location->copy()
    );

    do {
        c = a_location->readChar();
        if ('\0' == c){
            break;
        }

        PDEBUG(50, "c: '%c', state: '%d'\n",
            c,
            state
        );

        switch (state){
            case STATE_ASSIGN_STRING:
                if ('"' != c){
                    value += c;
                } else {
                    ObjectMapIt it;
                    //ObjectStringSptr    var_name;

                    //PWARN("var_val: '%s'\n", var_val.c_str());

                    if (value.empty()){
                        PERROR("missing variable value\n"
                            "%s\n",
                            location->toString()->c_str()
                        );
                        goto fail;
                    }

                    //var_name = dynamic_pointer_cast
                    //    <ObjectString>(
                    //        a_vars->get("cur_var")
                    //    );

                    // store var
                    //PWARN("var name: '%s', var val: '%s'\n",
                    //    var_name->c_str(),
                    //    var_val.c_str()
                    //);
                    //a_vars->add(varname, location);
                    a_vars->add("cur_right", value);

                    err = 0;
                    goto out;
                }
                break;

            case STATE_ASSIGN_WAIT:
                if (' ' == c){
                    break;
                } else if ( '0' <= c
                    &&      '9' >= c)
                {
                    state   = STATE_ASSIGN_DIGIT;
                    value   = c;
                } else if ('"' == c){
                    state   = STATE_ASSIGN_STRING;
                } else if ( 'a' <= c
                    &&      'z' >= c)
                {
                    state   = STATE_ASSIGN_VAR;
                    value   = c;
                } else {
                    PERROR("unsupported symbol: '%c'"
                        " at '%s'\n",
                        c,
                        a_location->toString()->c_str()
                    );
                }
                break;

            default:
                PFATAL("unknown state: '%d'\n", state);
                break;
        }
    } while (c);

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

int32_t ObjectMicrocode::s_operatorRun(
    ObjectMicrocodeLocationSptr a_location,
    ObjectMapSptr               a_vars)
{
    int32_t             err = -1;
    ObjectStringSptr    res;

    res = a_vars->toString();

    PWARN("a_vars: '%s'\n", res->c_str());

    //a_location->readChar();

    // all ok
    err = 0;

    return err;
}

// ---------------- tests ----------------

int32_t ObjectMicrocode::s_test_parse(
    const char *a_tmp_path)
{
    int32_t res, err = -1;

    ObjectMicrocodeSptr microcode;
    string              fname = "2.txt";

    microcode = OBJECT<ObjectMicrocode>();
    res       = microcode->parseFile(fname.c_str());
    if (res){
        PERROR("cannot parse microcode from: '%s'\n",
            fname.c_str()
        );
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

// ---------------- module ----------------

string ObjectMicrocode::s_getType()
{
    return "core.object.microcode";
}

int32_t ObjectMicrocode::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectMicrocode::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectMicrocode::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectMicrocode(a_create_flags));

    return object;
}

void ObjectMicrocode::s_getTests(
    Tests &a_out)
{
    TestInfo    test;

    test = TestInfo();
    test.name         = "attempt to parse microcode";
    test.success_code = 0;
    test.run          = ObjectMicrocode::s_test_parse;
    //a_out.push_back(test);
}

ModuleInfo core_object_microcode = {
    .getType        = ObjectMicrocode::s_getType,
    .init           = ObjectMicrocode::s_init,
    .shutdown       = ObjectMicrocode::s_shutdown,
    .objectCreate   = ObjectMicrocode::s_objectCreate,
    .getTests       = ObjectMicrocode::s_getTests
};

