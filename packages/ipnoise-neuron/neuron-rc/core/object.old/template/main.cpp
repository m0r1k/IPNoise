#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"

#include "core/object/template/main.hpp"

ObjectTemplate::ObjectTemplate(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectTemplate::~ObjectTemplate()
{
}

void ObjectTemplate::do_init(
    const Object::CreateFlags   &a_flags)
{
    m_prefix = "html";
}

string ObjectTemplate::getType()
{
    return ObjectTemplate::_getType();
}

void ObjectTemplate::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.template");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectTemplate::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectTemplate::setObject(
    const Object *a_object)
{
    char    buffer[512] = {0x00};
    string  code;

    m_object = a_object;

    // generic init
#ifdef __x86_64
    snprintf(buffer, sizeof(buffer),
        "$main::__object_addr = \"%lx\";",
        (uint64_t)m_object
    );
#else
    snprintf(buffer, sizeof(buffer),
        "$main::__object_addr = \"%x\";",
        (uint32_t)m_object
    );
#endif
    code += buffer;
/*
#ifdef __x86_64
    snprintf(buffer, sizeof(buffer),
        "$main::__template_addr = \"%lx\";",
        (uint64_t)this
    );
#else
    snprintf(buffer, sizeof(buffer),
        "$main::__template_addr = \"%x\";",
        (uint32_t)this
    );
#endif
    code += buffer;
*/
    code += "use utf8;";
    code += "use Encode;";
    code += "use core::templates::perl::generic"
        " qw(render renderEmbedded);";
    code += "return '';";

    Thread::processPerlCode(code);
}

void ObjectTemplate::setPrefix(
    const string &a_prefix)
{
    m_prefix = a_prefix;
}

string ObjectTemplate::getPrefix()
{
    return m_prefix;
}

string ObjectTemplate::render(
    const string    &a_object_id,
    const string    &a_view)
{
    string          ret;
    string          prefix  = getPrefix();
    ObjectMapSptr   args;

    PTIMING(0.0f, "render: '%s'\n",
        a_object_id.c_str()
    );

    if (a_object_id.empty()){
        PFATAL("missing argument: 'a_object_id'\n");
    }
    if (prefix.empty()){
        PFATAL("missing 'prefix'\n");
    }
    if (a_view.empty()){
        PFATAL("missing argument: 'a_view'\n");
    }

    PROP_MAP(args);

    args->add("id",         a_object_id);
    args->add("prefix",     prefix);
    args->add("view",       a_view);

    ret = Thread::processPerlSub("render", args);

    return ret;
}

string ObjectTemplate::renderEmbedded(
    const string    &a_object_id,
    const string    &a_view)
{
    string          ret;
//    string          id      = a_id->toString();
    string          prefix  = getPrefix();
    ObjectMapSptr   args;

    PTIMING(0.010f, "renderEmbedded: '%s'\n",
        a_object_id.c_str()
    );

    if (a_object_id.empty()){
        PFATAL("missing argument: 'a_object_id'\n");
    }
    if (prefix.empty()){
        PFATAL("missing 'prefix'\n");
    }
    if (a_view.empty()){
        PFATAL("missing argument: 'a_view'\n");
    }

    PROP_MAP(args);

    args->add("id",         a_object_id);
    args->add("prefix",     prefix);
    args->add("view",       a_view);

    ret = Thread::processPerlSub("renderEmbedded", args);

    return ret;
}

int32_t ObjectTemplate::parseFile(
    const string    &a_fname,
    string          &a_out)
{
    int32_t err         = -1;
    FILE    *file       = NULL;
    char    *tmp        = NULL;
    size_t  n           = 0;
    string  perl;

    file = fopen(a_fname.c_str(), "r");
    if (!file){
        PERROR("cannot open file: '%s' for read\n",
            a_fname.c_str()
        );
        goto fail;
    }

    while (getline(&tmp, &n, file) > 0){
        string              line  = tmp;
        string::size_type   pos   = string::npos;

        // free tmp buffer
        free(tmp);
        tmp = 0;
        n   = 0;

        // may be end of perl?
        if (perl.size()){
            pos = line.find("</perl>");
            if (string::npos != pos){
                if (pos){
                    perl += line.substr(0, pos);
                    line.erase(0, pos - 1);
                }
                line.erase(0, sizeof("</perl>"));
                a_out += Thread::processPerlCode(perl);
                perl = "";
            } else {
                perl += line;
                continue;
            }
        }

        // may be start of perl?
        do {
            // search start
            pos = line.find("<perl>");
            if (string::npos == pos){
                a_out += line;
                break;
            }

            a_out += line.substr(0, pos);
            line.erase(0, pos + sizeof("<perl>") - 1);

            // search end
            pos = line.find("</perl>");
            if (string::npos != pos){
                if (pos){
                    perl += line.substr(0, pos);
                    line.erase(0, pos - 1);
                }
                line.erase(0, sizeof("</perl>"));
                a_out += Thread::processPerlCode(perl);
                a_out += line;
                line = "";
                perl = "";
            } else {
                // add space, so perl will be have size9)
                perl += " " + line;
            }
        } while (1);
    }

    fclose(file);

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

// static

ObjectSptr ObjectTemplate::getObject()
{
    ObjectSptr          object;
    string              object_id;
    SV                  *sv             = NULL;
    char                *object_id_ptr  = NULL;
    ObjectContextSptr   context;

    context = Thread::getCurContext();

    sv = perl_get_sv(
        "core::object::object::main::OBJECT_ID",
        FALSE
    );
    if (sv){
        object_id_ptr = SvPVX(sv);
    }
    if (object_id_ptr){
        object_id = object_id_ptr;
    }
    if (object_id.size()){
        object = context->getObjectById(
            object_id
        );
    }

    return object;
}

// ---------------- module ----------------

string ObjectTemplate::_getType()
{
    return "core.object.template";
}

string ObjectTemplate::_getTypePerl()
{
    return "core::object::template::main";
}

int32_t ObjectTemplate::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectTemplate::_destroy()
{
}

ObjectSptr ObjectTemplate::_object_create()
{
    ObjectSptr object;
    _OBJECT_TEMPLATE(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectTemplate::_getType,
    .type_perl      = ObjectTemplate::_getTypePerl,
    .init           = ObjectTemplate::_init,
    .destroy        = ObjectTemplate::_destroy,
    .object_create  = ObjectTemplate::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

