#include "utils.hpp"
#include "core/neuron/neuron/main.hpp"
#include "core/neuron/session/main.hpp"
#include "network.hpp"
#include "thread/http.hpp"

#include "template.hpp"

// http://perldoc.perl.org/perlapi.html
// http://perldoc.perl.org/index-internals.html
// sv_setsv(out, perl_get_sv("main::tmp", FALSE));

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

XS(XS_main_getContext)
{
    dXSARGS;

    ContextSptr     context;
    SV              *ret            = NULL;
    PerlInterpreter *my_perl_orig   = my_perl;

    if (items != 0){
        // will show 'Usage: main::_getContext()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = ThreadHttp::getContext();

    // restore context
    my_perl = my_perl_orig;

    if (context){
        PropMapSptr map = PROP_MAP();
        context->serializeToPropMap(map);
        ret = Template::parseNeuronProps(map);
    }

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getSession)
{
    dXSARGS;

    NeuronSessionSptr   session;
    ContextSptr         context;
    PerlInterpreter     *my_perl_orig   = my_perl;
    SV                  *ret            = NULL;

    if (items != 0){
        // will show 'Usage: main::getSession()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = ThreadHttp::getContext();
    // restore context
    my_perl = my_perl_orig;

    if (context){
        session = context->getSession();
        // restore context
        my_perl = my_perl_orig;
    }

    if (session){
        //string code = "return ";
        //code += session->serialize(" => ", 1);
        //code += ";";

        // restore context
        my_perl = my_perl_orig;
        //ret = perl_eval_pv(code.c_str(), TRUE);
        ret = Template::parseNeuronProps(session);
    }

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getCurUser)
{
    dXSARGS;

    NeuronSessionSptr   session;
    ContextSptr         context;
    NeuronUserSptr      user;
    SV                  *ret            = NULL;
    PerlInterpreter     *my_perl_orig   = my_perl;

    if (items != 0){
        // will show 'Usage: main::_getCurUser()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = ThreadHttp::getContext();

    // restore context
    my_perl = my_perl_orig;

    // get user
    user = context->getUser();

    // restore context
    my_perl = my_perl_orig;

    // string code = "return ";
    // code += user->serialize(" => ", 1);
    // code += ";";

    // PWARN("code size: '%d'\n", (int32_t)code.size());

    // restore context
    my_perl = my_perl_orig;
    // ret = perl_eval_pv(code.c_str(), TRUE);
    ret = Template::parseNeuronProps(user);

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getCurNeuron)
{
    dXSARGS;

    PerlInterpreter *my_perl_orig   = my_perl;
    SV              *ret            = NULL;
    NeuronSptr      cur_neuron;

    if (items != 0){
        // will show 'Usage: main::_getCurNeuron()'
        croak_xs_usage(cv, "");
    }

    // get neuron
    cur_neuron = Template::getNeuron();
    if (cur_neuron){
        // string code = "return ";
        // code += cur_neuron->serialize(" => ", 1);
        // code += ";";
        // ret = perl_eval_pv(code.c_str(), TRUE);
        ret = Template::parseNeuronProps(cur_neuron);
    }

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getNeuronParams)
{
    dXSARGS;

    string              id;
    NeuronSptr          neuron;
    DbThread            *db_thread      = NULL;
    SV                  *ret            = NULL;
    PerlInterpreter     *my_perl_orig   = my_perl;
    STRLEN              na;

    if (items != 1){
        // will show 'Usage: main::_getNeuronParams("id")'
        croak_xs_usage(cv, "\"id\"");
    }

    db_thread = Template::getDbThread();

    // get id
    {
        SV *sv = POPs;
        if (sv){
            id = SvPVutf8(sv, na);
        }
    }

    // get neuron
    if (id.size()){
        neuron = db_thread->getNeuronById(
            PROP_NEURON_ID(id)
        );
        // restore context
        my_perl = my_perl_orig;
    }
    if (neuron){
        NParamsSptr    res(new NParams);
        NParamsConstIt res_it;

        string  code, data;
        neuron->getParams(res);
        for (res_it = res->begin();
            res_it != res->end();
            res_it++)
        {
            NeuronParamSptr cur_param = res_it->second;
            if (data.size()){
                data += ", ";
            }
            data += cur_param->serialize(" => ");
        }

        code += "return [" + data + "];";
        // restore context
        my_perl = my_perl_orig;
        // ENTER;SAVETMPS;
        ret = perl_eval_pv(code.c_str(), TRUE);
        // FREETMPS;LEAVE;
    }

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

SV * Template::parseNeuronProps(
    NeuronSptr  a_neuron)
{
    SV          *ret  = NULL;
    PropMapSptr props = PROP_MAP();
    a_neuron->getAllProps(props);
    ret = Template::parseNeuronProps(props);
    return ret;
}

SV * Template::parseNeuronProps(
    PropSptr  a_prop)
{
    SV              *ret    = NULL;
    Prop::PropType  type    = a_prop->getType();
    string          val_str = a_prop->toString();

    switch (type){
        case Prop::PROP_TYPE_DOUBLE:
            ret = newSVnv(atof(val_str.c_str()));
            PDEBUG(25,
                "parse prop type: 'double', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Prop::PROP_TYPE_TIME:
        case Prop::PROP_TYPE_INT:
            ret = newSViv(atoi(val_str.c_str()));
            PDEBUG(25,
                "parse prop type: 'int', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Prop::PROP_TYPE_STRING:
        case Prop::PROP_TYPE_NEURON_GROUP:
        case Prop::PROP_TYPE_NEURON_ID:
        case Prop::PROP_TYPE_NEURON_TYPE:
            ret = newSVpv(
                val_str.c_str(),
                val_str.size()
            );
            PDEBUG(25,
                "parse prop type: 'string', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Prop::PROP_TYPE_VECTOR:
            {
                PropVectorSptr     vector;
                PropVectorConstIt  it;
                AV                 *perl_array = newAV();

                vector = dynamic_pointer_cast<PropVector>(
                    a_prop
                );

                PDEBUG(25,
                    "parse prop type: 'vector',"
                    " val: '%s'\n",
                    val_str.c_str()
                );

                for (it = vector->begin();
                    it != vector->end();
                    it++)
                {
                    SV          *res        = NULL;
                    PropSptr    cur_val     = *it;
                    string      cur_val_str;

                    cur_val_str = cur_val->toString();
                    PDEBUG(25,
                        "parse prop type: 'vector',"
                        " cur_val: '%s'\n",
                        cur_val_str.c_str()
                    );

                    res = Template::parseNeuronProps(cur_val);
                    av_push(perl_array, res);
                }
                // ret = (SV *)perl_array;
                ret = newRV_noinc((SV *)perl_array);
            }
            break;

        case Prop::PROP_TYPE_NEURON_LINK:
        case Prop::PROP_TYPE_NEURON_LINK_EVENT:
            {
                PropMapSptr     map;
                PropMapConstIt  it;
                HV              *perl_hash = newHV();

                map = dynamic_pointer_cast<PropMap>(
                    a_prop
                );

                ret = newRV_noinc((SV *)perl_hash);
            }
            break;

        case Prop::PROP_TYPE_MAP:
            {
                PropMapSptr     map;
                PropMapConstIt  it;
                HV              *perl_hash = newHV();

                map = dynamic_pointer_cast<PropMap>(
                    a_prop
                );

                PDEBUG(25,
                    "parse prop type: 'map',"
                    " val: '%s'\n",
                    val_str.c_str()
                );

                for (it = map->begin();
                    it != map->end();
                    it++)
                {
                    SV          *res        = NULL;
                    PropSptr    cur_name    = it->first;
                    PropSptr    cur_val     = it->second;
                    string      cur_name_str;
                    string      cur_val_str;

                    cur_name_str = cur_name->toString();
                    cur_val_str  = cur_val->toString();

                    PDEBUG(25,
                        "parse prop type: 'map'"
                        " cur_name: '%s', cur_val: '%s'\n",
                        cur_name_str.c_str(),
                        cur_val_str.c_str()
                    );

                    res = Template::parseNeuronProps(cur_val);
                    hv_store(
                        perl_hash,
                        cur_name_str.c_str(),
                        cur_name_str.size(),
                        res,
                        0
                    );
                }
                ret = newRV_noinc((SV *)perl_hash);
            }
            break;

        default:
            PERROR("Cannot convert prop '%s' to perl,"
                " unsupported prop type: '%d'\n",
                val_str.c_str(),
                type
            );
            break;
    }

    if (ret){
       SvUTF8_on(ret);
    }
    return ret;
}

XS(XS_main_getNeuronById)
{
    dXSARGS;

    string              id;
    DbThread            *db_thread      = NULL;
    PerlInterpreter     *my_perl_orig   = my_perl;
    SV                  *ret            = NULL;
    STRLEN              na              = 0;

    if (items != 1){
        // will show 'Usage: main::_getNeuronById("id")'
        croak_xs_usage(cv, "\"id\"");
    }

    db_thread = Template::getDbThread();

    // get id
    {
        SV *sv = POPs;
        if (sv){
            id = SvPVutf8(sv, na);
        }
    }

    if (id.size()){
        NeuronSptr neuron;
        neuron = db_thread->getNeuronById(
            PROP_NEURON_ID(id)
        );
        // restore context
        my_perl = my_perl_orig;
        if (neuron){
            // string code = "my $ret = ";
            // code += neuron->serialize(" => ", 1);
            // code += "; use Data::Dumper; print Dumper($ret);return $ret;";
            // restore context
            my_perl = my_perl_orig;
            // sv_setsv(ret, perl_eval_pv(code.c_str(), TRUE));

            // {
            //    SV *res = Template::parseNeuronProps(neuron);
            //    ret = sv_2mortal((SV*)newRV_noinc(res));
            // }
            ret = Template::parseNeuronProps(neuron);
        }
    }

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getNeuronsInfo)
{
    dXSARGS;

    SV                  *ret            = NULL;
    PerlInterpreter     *my_perl_orig   = my_perl;
    PropNeuronTypeSptr  prop;
    string              code;

    if (items != 0){
        // will show 'Usage: main::_getNeuronInfo()'
        croak_xs_usage(cv, "");
    }

    prop = PROP_NEURON_TYPE();
    code += "return ";
    code += prop->serializeInfo(" => ");
    code += ";";

    // restore context
    my_perl = my_perl_orig;

    // ENTER;SAVETMPS;
    ret = perl_eval_pv(code.c_str(), TRUE);
    // FREETMPS;LEAVE;

    // restore context
    my_perl = my_perl_orig;

    XPUSHs(ret);
    XSRETURN(1);
}

void Template::xs_init(pTHX)
{
    dXSUB_SYS;

    newXS(
        "main::_getNeuronById",
        XS_main_getNeuronById,
         __FILE__
    );
    newXS(
        "main::_getCurNeuron",
        XS_main_getCurNeuron,
         __FILE__
    );
    newXS(
        "main::_getNeuronParams",
        XS_main_getNeuronParams,
         __FILE__
    );
    newXS(
        "main::_getContext",
        XS_main_getContext,
         __FILE__
    );
    newXS(
        "main::_getSession",
        XS_main_getSession,
         __FILE__
    );
    newXS(
        "main::_getCurUser",
        XS_main_getCurUser,
         __FILE__
    );
    newXS(
        "main::_getNeuronsInfo",
        XS_main_getNeuronsInfo,
         __FILE__
    );

    // possibly also boot DynaLoader and friends
    // perlembed has more details on this
    // and ExtUtils::Embed helps as well
    newXS(
        "DynaLoader::boot_DynaLoader",
        boot_DynaLoader,
        __FILE__
    );
}

Template::Template(
    Neuron  *a_neuron)
{
    char            buffer[512]         = {0x00};
    const char      *embedding_argv[]   = { "", "-e", "0" };
    int32_t         embedding_argc      = 3;
    PerlInterpreter *my_perl_orig       = NULL;
    string          code;

    m_prefix        = "html";
    my_perl_orig    = (PerlInterpreter *)Perl_get_context();
    m_neuron        = a_neuron;
    m_perl          = perl_alloc();

    PERL_SET_CONTEXT(m_perl);

    perl_construct(m_perl);
    perl_parse(
        m_perl,
        Template::xs_init,
        embedding_argc,
        (char **)embedding_argv,    // TODO WTF?
        NULL
    );

    PL_perl_destruct_level = 1;
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

    // perl_run(m_perl);

    // generic init
#ifdef __x86_64
    snprintf(buffer, sizeof(buffer),
        "$main::__neuron_addr = \"%lx\";",
        (uint64_t)m_neuron
    );
#else
    snprintf(buffer, sizeof(buffer),
        "$main::__neuron_addr = \"%x\";",
        (uint32_t)m_neuron
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
    processPerlCode(code);

    PERL_SET_CONTEXT(my_perl_orig);
}

Template::~Template()
{
    // setupContext();
    PERL_SET_CONTEXT(m_perl);

    // PL_perl_destruct_level = 1;
    perl_destruct(m_perl);
    perl_free(m_perl);
}

void Template::setPrefix(
    const string &a_prefix)
{
    m_prefix = a_prefix;
}

string Template::getPrefix() const
{
    return m_prefix;
}

string Template::render(
    PropNeuronIdSptr    &a_id,
    const string        &a_view)
{
    string  ret;
    string  id      = a_id->toString();
    string  prefix  = getPrefix();

    PERL_SET_CONTEXT(m_perl);

    {
        dSP;
        int count;

        ENTER;
        SAVETMPS;

        PUSHMARK(SP);
        XPUSHs(sv_2mortal(newSVpv(
            id.c_str(), id.size()
        )));
        XPUSHs(sv_2mortal(newSVpv(
            prefix.c_str(), prefix.size()
        )));
        XPUSHs(sv_2mortal(newSVpv(
            a_view.c_str(), a_view.size()
        )));
        PUTBACK;

        count = call_pv("render", G_SCALAR);

        SPAGAIN;
        if (1 != count){
            PFATAL("more than 1 argument returned from"
                " perl 'render' for neuron ID: '%s'"
                " view: '%s'\n",
                id.c_str(),
                a_view.c_str()
            );
        }

        {
            SV      *sv = POPs;
            STRLEN  na  = 0;
            ret = SvPVutf8(sv, na);
        }

        PUTBACK;
        FREETMPS;
        LEAVE;
    }

    return ret;
}

string Template::renderEmbedded(
    PropNeuronIdSptr    &a_id,
    const string        &a_view)
{
    string  ret;
    string  id      = a_id->toString();
    string  prefix  = getPrefix();

    PERL_SET_CONTEXT(m_perl);

    {
        dSP;
        int count;

        ENTER;
        SAVETMPS;

        PUSHMARK(SP);
        XPUSHs(sv_2mortal(newSVpv(
            id.c_str(), id.size()
        )));
        XPUSHs(sv_2mortal(newSVpv(
            prefix.c_str(), prefix.size()
        )));
        XPUSHs(sv_2mortal(newSVpv(
            a_view.c_str(), a_view.size()
        )));
        PUTBACK;

        count = call_pv("renderEmbedded", G_SCALAR);

        SPAGAIN;
        if (1 != count){
            PFATAL("more than 1 argument returned from"
                " perl 'render' for neuron ID: '%s'"
                " view: '%s'\n",
                id.c_str(),
                a_view.c_str()
            );
        }

        {
            SV      *sv = POPs;
            STRLEN  na  = 0;
            ret = SvPVutf8(sv, na);
        }

        PUTBACK;
        FREETMPS;
        LEAVE;
    }

    return ret;
}

void Template::setupContext()
{
    PERL_SET_CONTEXT(m_perl);
}

string Template::processPerlCode(
    const string &a_perl)
{
    string              html, code;
    STRLEN              na              = 0;
    SV                  *res            = NULL;
    PerlInterpreter     *my_perl_orig   = NULL;
    my_perl_orig = (PerlInterpreter *)Perl_get_context();

    PERL_SET_CONTEXT(m_perl);

    // res = perl_eval_pv(a_perl.c_str(), TRUE);
    // html += SvPVutf8(res, na);


    // http://cvs.epicsol.org/cgi/viewcvs.cgi/*checkout*/epic4/source/perl.c?rev=1.3

    // ENTER; SAVETMPS;
    res = perl_eval_pv(a_perl.c_str(), TRUE);
    html += SvPVutf8(res, na);
    // FREETMPS; LEAVE;

    // html += SvPV(res, na);
    // free scalar
    //SvREFCNT_dec(res);

    PDEBUG(60, "process perl code, res: '%s'\n",
        html.c_str()
    );

    PERL_SET_CONTEXT(my_perl_orig);

    return html;
}

int32_t Template::parseFile(
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
                a_out += processPerlCode(perl);
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
                a_out += processPerlCode(perl);
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

DbThread * Template::getDbThread()
{
    Thread      *thread     = NULL;
    DbThread    *db_thread  = NULL;

    thread      = (Thread *)QThread::currentThread();
    db_thread   = thread
        ->getNetwork()
        ->getDbThread();

    return db_thread;
}

NeuronSptr Template::getNeuron()
{
    SV          *sv         = NULL;
    DbThread    *db_thread  = Template::getDbThread();

    NeuronSptr  neuron;
    string      neuron_id;
    char        *neuron_id_ptr = NULL;

    sv = perl_get_sv(
        "core::neuron::neuron::main::NEURON_ID",
        FALSE
    );
    if (sv){
        neuron_id_ptr = SvPVX(sv);
    }
    if (neuron_id_ptr){
        neuron_id = neuron_id_ptr;
    }
    if (neuron_id.size()){
        neuron = db_thread->getNeuronById(
            PROP_NEURON_ID(neuron_id)
        );
    }
    return neuron;
}

