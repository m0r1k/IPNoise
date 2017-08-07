#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <unistd.h> // for gethostname

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "core/object/object/main.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/bin/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/objectLink/main.hpp"
//#include "core/object/objectLinkAction/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
//#include "core/object/thread/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/int64/main.hpp"
#include "core/object/uint64/main.hpp"
#include "core/object/unique/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/vector/main.hpp"
#include "db.hpp"
#include "db/disk.hpp"
#include "ipnoise-common/ipnoise.h"

#include "thread.hpp"

recursive_mutex     g_threads_mutex;
ThreadsByName       g_threads_by_name;
ThreadsByAddr       g_threads_by_addr;

Contexts            g_contexts;
recursive_mutex     g_contexts_mutex;

recursive_mutex     g_kladr_db_mutex;
sqlite3             *g_kladr_db = NULL;

Thread::Thread(
    const string    &a_name)
{
    m_name          = a_name;
    m_addr          = 0;
    m_state         = Thread::THREAD_STATE_CREATED;
    m_timer         = NULL;
    m_event_base    = NULL;
    m_dns_base      = NULL;
    m_std_thread    = NULL;
    m_perl          = NULL;

    PROP_CONTEXT(m_context);

    // setup timer;
    m_timer_interval_max = 1;   // max interval 1 second
    setTimerInterval(1);

    if (!a_name.size()){
        PFATAL("missing argument: 'a_name'\n");
    }
}

Thread::~Thread()
{
//    string      thread_name = getName();
//    uint64_t    thread_addr = getAddr();

//    lock_guard<recursive_mutex> guard(g_threads_mutex);
//    g_threads_by_name.erase(thread_name);
//    g_threads_by_addr.erase(thread_addr);

    if (m_timer){
        evtimer_del(m_timer);
        m_timer = NULL;
    }
    if (m_std_thread){
        delete m_std_thread;
        m_std_thread = NULL;
    }

    // TODO do attempt to move subscribers from here
    // to another thread
    // 20141028 morik@
    while (!m_accepts.empty()){
        AcceptsIt               accepts_it;
        struct evconnlistener   *listener = NULL;

        accepts_it = m_accepts.begin();
        listener   = accepts_it->second;
        evconnlistener_free(listener);

        m_accepts.erase(accepts_it);
    }

    while (!m_bevs.empty()){
        BevsIt              bevs_it;
        struct bufferevent  *bev = NULL;

        bevs_it = m_bevs.begin();
        bev     = bevs_it->second;

        Thread::close(bev);
    }

    freeLibEvent();

    // PL_perl_destruct_level = 1;
    PERL_SET_CONTEXT(m_perl);
    perl_destruct(m_perl);
    perl_free(m_perl);

    if ("main" == getName()){
        closeKladrDb();
    }
}

string Thread::getLocation()
{
    char    buffer[512]             = { 0x00 };
    char    hostname[HOST_NAME_MAX] = { 0x00 };
    int32_t hostlen                 = 0;
    int32_t res, i;

    res = gethostname(hostname, sizeof(hostname));
    if (res){
        PERROR("cannot get hostname, res: '%d'\n", res);
    }
    hostlen = (uint32_t)strlen(hostname);

    for (i = 0; i < hostlen; i++){
        char cur_c = hostname[i];
        if (':' == cur_c){
            PFATAL("hostname cannot contain: ':' symbol"
                " (hostname: '%s')\n",
                hostname
            );
        }
    }

    snprintf(buffer, sizeof(buffer),
        "%s:0x%llx:%s",
        hostname,
        getAddr(),
        getName().c_str()
    );

    return buffer;
}

string Thread::getName()
{
    return m_name;
}

void Thread::setAddr(
    const uint64_t  &a_addr)
{
    if (!a_addr){
        PDEBUG(30, "thread: '%s',"
            " attempt to set NULL thread addr\n",
            getName().c_str()
        );
    }

    m_addr = a_addr;
}

uint64_t Thread::getAddr()
{
    return m_addr;
}

Thread::ThreadState Thread::getState()
{
    return m_state;
}

void Thread::freeLibEvent()
{
    if (m_event_base){
        event_base_free(m_event_base);
        m_event_base = NULL;
    }

    if (m_dns_base){
        int32_t fail_requests = 0;
        evdns_base_free(
            m_dns_base,
            fail_requests
        );
        m_dns_base = NULL;
    }
}

int32_t Thread::initLibEvent()
{
    int32_t err = -1;

    freeLibEvent();

    // event base
    m_event_base = event_base_new();
    if (!m_event_base){
        PERROR("thread: '%s' (0x%llx),"
            " event_base_new failed\n",
            getName().c_str(),
            getAddr()
        );
        goto fail;
    }

    // event dns
    m_dns_base = evdns_base_new(
        m_event_base,
        1   // initialize_nameservers
    );
    if (!m_dns_base){
        PERROR("thread: '%s' (0x%llx),"
            " evdns_base_new failed\n",
            getName().c_str(),
            getAddr()
        );
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

int32_t Thread::initDb()
{
    int32_t res, err = -1;

    // create db
    m_db = DbSptr(new DbDisk());
    if (!m_db){
        PERROR("thread: '%s' (0x%llx)"
            " cannot init DB,"
            " cannot allocate memory\n",
            getName().c_str(),
            getAddr()
        );
        err = -ENOMEM;
        goto fail;
    }

    // init db
    res = m_db->do_init();
    if (res){
        PERROR("thread: '%s' (0x%llx)"
            " cannot init DB, res: '%d'\n",
            getName().c_str(),
            getAddr(),
            res
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

int32_t Thread::initKladrDb()
{
    lock_guard<recursive_mutex> guard(g_kladr_db_mutex);

    int32_t res, err = -1;

    if (g_kladr_db){
        // already inited
        goto out;
    }

    res = sqlite3_open(KLADR_PATH, &g_kladr_db);
    if (SQLITE_OK != res){
        PWARN("Can't open database: '%s' (%s)\n",
            KLADR_PATH,
            sqlite3_errmsg(g_kladr_db)
        );
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

void Thread::closeKladrDb()
{
    if (g_kladr_db){
        lock_guard<recursive_mutex> guard(g_kladr_db_mutex);

        sqlite3_close(g_kladr_db);
        g_kladr_db = NULL;
    }
}

int32_t Thread::processKladrSql(
    const string    &a_sql,
    KladrCb         a_cb,
    KladrReqInfo    &a_kladr_req_info,
    string          &a_sql_err_msg)
{
    lock_guard<recursive_mutex> guard(g_kladr_db_mutex);

    int32_t ret             = -1;
    char    *sql_err_msg    = NULL;

    if (!g_kladr_db){
        PFATAL("kladr DB not inited\n");
    }

    PDEBUG(50, "executing sql: '%s'\n", a_sql.c_str());

    ret = sqlite3_exec(
        g_kladr_db,
        a_sql.c_str(),
        a_cb,
        (void *)&a_kladr_req_info,
        &sql_err_msg
    );

    PDEBUG(50, "kladr_req_info.m_results.size(): '%d'\n",
        (int32_t)a_kladr_req_info.m_results->size()
    );

    if (sql_err_msg){
        a_sql_err_msg = sql_err_msg;
    }

    if (sql_err_msg){
        sqlite3_free(sql_err_msg);
        sql_err_msg = NULL;
    }

    return ret;
}

// ---------------- perl support ----------------

// http://perldoc.perl.org/perlapi.html
// http://perldoc.perl.org/index-internals.html
// sv_setsv(out, perl_get_sv("main::tmp", FALSE));

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

XS(XS_main_getContext)
{
    dXSARGS;
    SV                  *ret = NULL;
    ObjectContextSptr   context;

    if (0 != items){
        // will show 'Usage: main::_getContext()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = Thread::getCurContext();

    if (context){
        //ObjectMapSptr map;
        //PROP_MAP(map);
        //context->serializeToObjectMap(map);
        ret = context->toPerl();
    }

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getSession)
{
    dXSARGS;

    ObjectSessionSptr   session;
    ObjectContextSptr   context;
    SV                  *ret = NULL;

    if (0 != items){
        // will show 'Usage: main::getSession()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = Thread::getCurContext();
    session = context->getSession();

    if (session){
        ret = session->toPerl();
    }

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getCurUser)
{
    dXSARGS;

    ObjectSessionSptr   session;
    ObjectContextSptr   context;
    ObjectUserSptr      user;
    SV                  *ret = NULL;

    if (0 != items){
        // will show 'Usage: main::_getCurUser()'
        croak_xs_usage(cv, "");
    }

    // get context
    context = Thread::getCurContext();

    // get user
    user = context->getUser();

    // string code = "return ";
    // code += user->serialize(" => ", 1);
    // code += ";";

    // PWARN("code size: '%d'\n", (int32_t)code.size());

    if (user){
        ret = user->toPerl();
    }

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getCurObject)
{
    dXSARGS;

    SV          *ret       = NULL;
    ObjectSptr  cur_object;

    if (0 != items){
        // will show 'Usage: main::_getCurObject()'
        croak_xs_usage(cv, "");
    }

    // TODO XXX FIXME
    cur_object = ObjectContext::getCurUser();
    if (cur_object){
        ret = cur_object->toPerl();
    }

    XPUSHs(ret);
    XSRETURN(1);
}

/*
XS(XS_main_getObjectParams)
{
    dXSARGS;

    string              id;
    ObjectSptr          object;
    SV                  *ret = NULL;
    STRLEN              na;
    ObjectContextSptr   context = Thread::getCurContext();

    if (items != 1){
        // will show 'Usage: main::_getObjectParams("id")'
        croak_xs_usage(cv, "\"id\"");
    }

    // get id
    {
        SV *sv = POPs;
        if (sv){
            id = SvPVutf8(sv, na);
        }
    }

    // get object
    if (id.size()){
        object = context->getObjectById(id);
    }
    if (object){
        NParamsSptr    res(new NParams);
        NParamsConstIt res_it;

        string  code, data;
        object->getParams(res);
        for (res_it = res->begin();
            res_it != res->end();
            res_it++)
        {
            ObjectParamSptr cur_param = res_it->second;
            if (data.size()){
                data += ", ";
            }
            data += cur_param->serialize(" => ");
        }

        code += "return [" + data + "];";
        // ENTER;SAVETMPS;
        ret = perl_eval_pv(code.c_str(), TRUE);
        // FREETMPS;LEAVE;
    }

    XPUSHs(ret);
    XSRETURN(1);
}
*/

/*
SV * ObjectTemplate::toPerl(
    ObjectSptr  a_object)
{
    SV            *ret    = NULL;
    ObjectMapSptr objects;

    PROP_MAP(objects);

    a_object->getAllObjects(objects);
    ret = objects->toPerl();
    return ret;
}
*/

/*
SV * ObjectTemplate::toPerl(
    ObjectSptr  a_object)
{
    SV                  *ret    = NULL;
    Object::ObjectType  type    = a_object->getType();
    string              val_str = a_object->toString();

    switch (type){
        case Object::OBJECT_TYPE_DOUBLE:
            ret = newSVnv(atof(val_str.c_str()));
            PDEBUG(25,
                "parse object type: 'double', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Object::OBJECT_TYPE_TIME:
        case Object::OBJECT_TYPE_INT32:
            ret = newSViv(atoi(val_str.c_str()));
            PDEBUG(25,
                "parse object type: 'int32', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Object::OBJECT_TYPE_INT64:
            ret = newSViv(atoll(val_str.c_str()));
            PDEBUG(25,
                "parse object type: 'int64', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Object::OBJECT_TYPE_STRING:
        case Object::OBJECT_TYPE_OBJECT_GROUP:
        case Object::OBJECT_TYPE_OBJECT_ID:
        case Object::OBJECT_TYPE_OBJECT_TYPE:
            ret = newSVpv(
                val_str.c_str(),
                val_str.size()
            );
            PDEBUG(25,
                "parse object type: 'string', val: '%s'\n",
                val_str.c_str()
            );
            break;

        case Object::OBJECT_TYPE_VECTOR:
            {
                ObjectVectorSptr    vector;
                ObjectVectorIt      it;
                AV                  *perl_array = newAV();

                vector = dynamic_pointer_cast<ObjectVector>(
                    a_object
                );

                PDEBUG(25,
                    "parse object type: 'vector',"
                    " val: '%s'\n",
                    val_str.c_str()
                );

                for (it = vector->begin();
                    it != vector->end();
                    it++)
                {
                    SV          *res        = NULL;
                    ObjectSptr  cur_val     = *it;
                    string      cur_val_str;

                    cur_val_str = cur_val->toString();
                    PDEBUG(25,
                        "parse object type: 'vector',"
                        " cur_val: '%s'\n",
                        cur_val_str.c_str()
                    );

                    res = ObjectTemplate::serializeToPerl(cur_val);
                    av_push(perl_array, res);
                }
                // ret = (SV *)perl_array;
                ret = newRV_noinc((SV *)perl_array);
            }
            break;

        case Object::OBJECT_TYPE_VEC3:
            {
                ObjectVec3Sptr  object_pos;
                AV              *perl_array = newAV();
                SV              *res        = NULL;

                object_pos = dynamic_pointer_cast<ObjectVec3>(
                    a_object
                );

                PDEBUG(25,
                    "parse object type: 'pos',"
                    " val: '%s'\n",
                    val_str.c_str()
                );

                if (object_pos){
                    res = newSVnv(object_pos->getX());
                    av_push(perl_array, res);

                    res = newSVnv(object_pos->getY());
                    av_push(perl_array, res);

                    res = newSVnv(object_pos->getZ());
                    av_push(perl_array, res);
                } else {
                    PERROR("cannot parse object: 'pos'\n");
                }

                ret = newRV_noinc((SV *)perl_array);
            }
            break;

        case Object::OBJECT_TYPE_OBJECT_LINK:
        case Object::OBJECT_TYPE_OBJECT_LINK_EVENT:
            {
                ObjectMapSptr       map;
                ObjectMapConstIt    it;
                HV                  *perl_hash = newHV();

                map = dynamic_pointer_cast<ObjectMap>(
                    a_object
                );

                ret = newRV_noinc((SV *)perl_hash);
            }
            break;

        case Object::OBJECT_TYPE_MAP:
            {
                ObjectMapSptr   map;
                ObjectMapIt     it;
                HV              *perl_hash = newHV();

                map = dynamic_pointer_cast<ObjectMap>(
                    a_object
                );

                PDEBUG(25,
                    "parse object type: 'map',"
                    " val: '%s'\n",
                    val_str.c_str()
                );

                for (it = map->begin();
                    it != map->end();
                    it++)
                {
                    SV          *res        = NULL;
                    ObjectSptr  cur_name    = it->first;
                    ObjectSptr  cur_val     = it->second;
                    string      cur_name_str;
                    string      cur_val_str;

                    cur_name_str = cur_name->toString();
                    cur_val_str  = cur_val->toString();

                    PDEBUG(25,
                        "parse object type: 'map'"
                        " cur_name: '%s', cur_val: '%s'\n",
                        cur_name_str.c_str(),
                        cur_val_str.c_str()
                    );

                    res = ObjectTemplate::serializeToPerl(cur_val);
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
            PERROR("Cannot convert object '%s' to perl,"
                " unsupported object type: '%d'\n",
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
*/

XS(XS_main_getModuleInfo)
{
    dXSARGS;

    SV                  *ret    = NULL;
    STRLEN              na      = 0;
    ObjectContextSptr   context = Thread::getCurContext();
    SV                  *sv     = NULL;

    string              object_type;
    ObjectMapSptr       object_info;

    if (1 != items){
        croak_xs_usage(cv, "\"object_type\"");
    }

    // get object type
    sv          = POPs;
    object_type = SvPVutf8(sv, na);

    if (!object_type.empty()){
        object_info = Object::getModuleInfo(object_type);
        if (object_info){
            ret = object_info->toPerl();
        }
    }

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_pinfo)
{
    dXSARGS;

    string  msg;
    SV      *ret    = NULL;
    STRLEN  na      = 0;
    SV      *sv     = NULL;

    // get args
    if (1 != items){
        croak_xs_usage(cv, "\"msg\"");
    }
    sv  = POPs;
    msg = SvPVutf8(sv, na);

    PINFO("%s", msg.c_str());

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_pdebug)
{
    dXSARGS;

    string      msg;
    int32_t     debug_level = 0;
    SV          *ret        = NULL;
    SV          *sv         = NULL;
    STRLEN      na          = 0;

    if (2 != items){
        croak_xs_usage(cv, "debug_level, \"msg\"");
    }

    // get debug_level
    sv          = POPs;
    debug_level = SvIV(sv);

    // get message
    sv          = POPs;
    msg         = SvPVutf8(sv, na);

    PDEBUG(debug_level, "%s", msg.c_str());

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_pwarn)
{
    dXSARGS;

    string  msg;
    SV      *ret    = NULL;
    STRLEN  na      = 0;
    SV      *sv     = NULL;

    // get args
    if (1 != items){
        croak_xs_usage(cv, "\"msg\"");
    }

    sv  = POPs;
    msg = SvPVutf8(sv, na);

    PWARN("%s", msg.c_str());

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_perror)
{
    dXSARGS;

    string  msg;
    SV      *ret    = NULL;
    STRLEN  na      = 0;
    SV      *sv     = NULL;

    // get args
    if (1 != items){
        croak_xs_usage(cv, "\"msg\"");
    }

    sv  = POPs;
    msg = SvPVutf8(sv, na);

    PERROR("%s", msg.c_str());

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_pfatal)
{
    dXSARGS;

    string  msg;
    SV      *ret    = NULL;
    STRLEN  na      = 0;
    SV      *sv     = NULL;

    // get args
    if (1 != items){
        croak_xs_usage(cv, "\"msg\"");
    }

    sv  = POPs;
    msg = SvPVutf8(sv, na);

    PFATAL("%s", msg.c_str());

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getObjectById)
{
    dXSARGS;

    string              id;
    SV                  *ret    = NULL;
    STRLEN              na      = 0;
    ObjectContextSptr   context = Thread::getCurContext();
    SV                  *sv     = NULL;

    if (1 != items){
        // will show 'Usage: main::_getObjectById("id")'
        croak_xs_usage(cv, "\"id\"");
    }

    // get id
    sv = POPs;
    id = SvPVutf8(sv, na);

    if (id.size()){
        ObjectSptr object;
        object = context->getObjectById(id);
        if (object){
            // string code = "my $ret = ";
            // code += object->serialize(" => ", 1);
            // code += "; use Data::Dumper; print Dumper($ret);return $ret;";
            // sv_setsv(ret, perl_eval_pv(code.c_str(), TRUE));

            // {
            //    SV *res = ObjectTemplate::toPerl(object);
            //    ret = sv_2mortal((SV*)newRV_noinc(res));
            // }
            ret = object->toPerl();
        }
    }

    XPUSHs(ret);
    XSRETURN(1);
}

XS(XS_main_getModulesInfo)
{
    dXSARGS;

    SV              *ret = NULL;
    ObjectMapSptr   modules_info;

    if (0 != items){
        // will show 'Usage: main::_getModulesInfo()'
        croak_xs_usage(cv, "");
    }

    PROP_MAP(modules_info);
    Object::getModulesInfo(modules_info);

    ret = modules_info->toPerl();

    XPUSHs(ret);
    XSRETURN(1);
}
void Thread::xs_init(pTHX)
{
    dXSUB_SYS;

    newXS("main::_getModuleInfo", XS_main_getModuleInfo, __FILE__);
    newXS("main::_pinfo",  XS_main_pinfo,   __FILE__);
    newXS("main::_pdebug", XS_main_pdebug,  __FILE__);
    newXS("main::_pwarn",  XS_main_pwarn,   __FILE__);
    newXS("main::_perror", XS_main_perror,  __FILE__);
    newXS("main::_pfatal", XS_main_pfatal,  __FILE__);
    newXS("main::_getObjectById", XS_main_getObjectById, __FILE__);
    newXS("main::_getCurObject",  XS_main_getCurObject, __FILE__);
    // newXS("main::_getObjectParams", XS_main_getObjectParams, __FILE__);
    newXS("main::_getContext", XS_main_getContext, __FILE__);
    newXS("main::_getSession", XS_main_getSession, __FILE__);
    newXS("main::_getCurUser", XS_main_getCurUser, __FILE__);
    newXS("main::_getModulesInfo", XS_main_getModulesInfo, __FILE__);

    // possibly also boot DynaLoader and friends
    // perlembed has more details on this
    // and ExtUtils::Embed helps as well
    newXS(
        "DynaLoader::boot_DynaLoader",
        boot_DynaLoader,
        __FILE__
    );
}

int32_t Thread::initPerl()
{
    int32_t err = -1;

    const char  *embedding_argv[]   = { "", "-e", "0" };
    int32_t     embedding_argc      = 3;
    // PerlInterpreter *my_perl_orig       = NULL;

    m_perl = perl_alloc();

    // my_perl_orig = (PerlInterpreter *)Perl_get_context();
    // TODO FIXME XXX
    // now 1 thread - 1 perl, so we can remove it
    // 201410 morik@
    // PERL_SET_CONTEXT(m_perl);

    perl_construct(m_perl);
    perl_parse(
        m_perl,
        Thread::xs_init,
        embedding_argc,
        (char **)embedding_argv,    // TODO WTF?
        NULL
    );

    PL_perl_destruct_level = 1;
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

    // perl_run(m_perl);
    //PERL_SET_CONTEXT(my_perl_orig);

    // all ok
    err = 0;

    return err;
}

string Thread::_processPerlCode(
    const string    &a_perl)
{
    string  html, code;
    STRLEN  na      = 0;
    SV      *res    = NULL;

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

    return html;
}

string Thread::_processPerlSub(
    const string    &a_sub_name,
    ObjectMapSptr   a_arg)
{
    string          ret;
    ObjectMapSptr   arg = a_arg;

    PTIMING(0.0f, "sub name: '%s'\n",
        a_sub_name.c_str()
    );

    if (a_sub_name.empty()){
        PFATAL("missing argument: 'a_sub_name'\n");
    }

    if (!arg){
        PROP_MAP(arg);
    }

    PERL_SET_CONTEXT(m_perl);

    {
        dSP;
        int32_t count   = 0;
        SV      *sv     = NULL;
        STRLEN  na      = 0;

        ENTER;
        SAVETMPS;

        PUSHMARK(SP);
        XPUSHs(sv_2mortal(arg->toPerl()));
//        XPUSHs(sv_2mortal(newSVpv(
//            id.c_str(), id.size()
//        )));
//        XPUSHs(sv_2mortal(newSVpv(
//            prefix.c_str(), prefix.size()
//        )));
//        XPUSHs(sv_2mortal(newSVpv(
//            a_view.c_str(), a_view.size()
//        )));
        PUTBACK;

        count = call_pv(a_sub_name.c_str(), G_SCALAR);

        SPAGAIN;
        if (1 != count){
            PFATAL("more than 1 argument returned from"
                " perl function: '%s'\n",
                a_sub_name.c_str()
            );
        }

        sv  = POPs;
        ret = SvPVutf8(sv, na);

        PUTBACK;
        FREETMPS;
        LEAVE;
    }

    return ret;
}

int32_t Thread::start()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;

    string      thread_name = getName();
    uint64_t    thread_addr = getAddr();

    if (Thread::THREAD_STATE_CREATED != m_state){
        PWARN("thread: '%s' (0x%llx)"
            " invalid thread state: '%d'\n",
            thread_name.c_str(),
            thread_addr,
            m_state
        );
        goto out;
    }

    setState(Thread::THREAD_STATE_INIT);

    PDEBUG(30, "start thread: '%s' (0x%llx)\n",
        thread_name.c_str(),
        thread_addr
    );

    if (m_std_thread){
        PWARN("thread: '%s' (0x%llx),"
            " m_std_thread is not NULL\n",
            thread_name.c_str(),
            thread_addr
        );
        delete m_std_thread;
        m_std_thread = NULL;
    }

    m_std_thread = new std::thread(Thread::run, this);
    if (!m_std_thread){
        PERROR("cannot allocate memory\n");
        goto fail;
    }

    m_std_thread->detach();

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

void Thread::stop()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (Thread::THREAD_STATE_RUNNING != m_state){
        PWARN("thread: '%s' (0x%llx)"
            " invalid thread state: '%d'\n",
            getName().c_str(),
            getAddr(),
            m_state
        );
        goto out;
    }

    setState(Thread::THREAD_STATE_SHUTDOWNING);

out:
    return;
}

void Thread::loopbreak()
{
    // this must be called only from thread
    // where event_base was started (e.g. from timer)
    PDEBUG(30, "stop thread: '%s'"
        " (0x%llx) state: '%d'\n",
        getName().c_str(),
        getAddr(),
        getState()
    );

    if (m_event_base){
        event_base_loopbreak(m_event_base);
    }
}

void Thread::join()
{
    m_std_thread->join();
}

bool Thread::joinable() const noexcept
{
    bool ret = false;
    if (m_std_thread){
        ret = m_std_thread->joinable();
    }
    return ret;
}

void Thread::detach()
{
    m_std_thread->detach();
}

int32_t Thread::loop_before()
{
    int32_t err = 0;
    // thread init, call before loop
    return err;
}

void Thread::loop()
{
    // thread default main loop
    event_base_dispatch(m_event_base);
}

void Thread::loop_tick()
{
}

int32_t Thread::loop_after()
{
    int32_t err = 0;
    // thread exit code
    return err;
}

void Thread::getTimerInterval(
    struct timeval &a_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_out.tv_sec  = m_timer_interval.tv_sec;
    a_out.tv_usec = m_timer_interval.tv_usec;
}

double Thread::getTimerInterval()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double          ret = 0.0f;
    struct timeval  res;

    getTimerInterval(res);
    ret = res.tv_sec + res.tv_usec / 1e6;

    return ret;
}

double Thread::getTimerIntervalMax()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    return m_timer_interval_max;
}

void Thread::setTimerInterval(
    const struct timeval &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_timer_interval = a_val;
}

void Thread::setTimerInterval(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    struct timeval  tv;
    double          val = a_val;

    if (0 > val){
        PFATAL("invalid timer interval: '%f'\n",
            val
        );
    }

    // -1 == disable
    if (    0 <= m_timer_interval_max
        &&  m_timer_interval_max < val)
    {
        val = m_timer_interval_max;
    }

    Utils::init_timeval(tv, val);
    setTimerInterval(tv);
}

void Thread::setTimerIntervalMax(
    const double &a_val)
{
    // set -1 to disable
    m_timer_interval_max = a_val;
}

void Thread::addTimerInterval(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double cur_val = getTimerInterval();
    double new_val = cur_val + a_val;

    setTimerInterval(new_val);
}

void Thread::subTimerInterval(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double cur_val = getTimerInterval();
    double new_val = cur_val - a_val;

    if (0 > new_val){
        new_val = 0.0f;
    }

    setTimerInterval(new_val);
}

void Thread::speedUp(
    const double &a_interval)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    setTimerInterval(a_interval);
}

void Thread::speedDown()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = getTimerInterval();
    if (!val){
        val = 0.001f;
    } else {
        val *= 1.02f;
    }
    setTimerInterval(val);
}

int32_t Thread::run(
    Thread *a_thread)
{
    int32_t     res, err        = -1;
    uint64_t    cur_thread_addr = Thread::getCurThreadAddr();
    string      new_thread_name = a_thread->getName();

    {
        // ok we are in thread
        lock_guard<recursive_mutex> guard(g_threads_mutex);

        // store in index 'addr to name'
        g_threads_by_addr[cur_thread_addr] = g_threads_by_name[new_thread_name];
    }

    // update state
    a_thread->setState(Thread::THREAD_STATE_INIT_THREAD);

    // store current thread addr
    a_thread->setAddr(cur_thread_addr);

    PDEBUG(30, "started thread: '%s'"
        " (0x%llx) run\n",
        new_thread_name.c_str(),
        cur_thread_addr
    );

    // init libevent
    res = a_thread->initLibEvent();
    if (res){
        err = res;
        PERROR("thread: '%s' (0x%llx),"
            " libevent init failed, res: '%d'\n",
            new_thread_name.c_str(),
            cur_thread_addr,
            res
        );
        goto fail;
    }

    // init kladr
    res = a_thread->initKladrDb();
    if (res){
        PWARN("cannot init kladr db\n");
    }

    // init perl
    res = a_thread->initPerl();
    if (res){
        err = res;
        PERROR("cannot init perl support\n");
        goto fail;
    }

    // init db
    res = a_thread->initDb();
    if (res){
        PERROR("thread: '%s' (0x%llx),"
            " db init failed, res: '%d'\n",
            new_thread_name.c_str(),
            cur_thread_addr,
            res
        );
        goto fail;
    }

    PINFO("thread: '%s' (0x%llx),"
        " initing..\n",
        a_thread->getName().c_str(),
        cur_thread_addr
    );

    // call init things
    res = a_thread->loop_before();
    if (res){
        err = res;
        PERROR("cannot init thread: '%s' (%llx),"
            " loop_before failed with code: '%d'\n",
            a_thread->getName().c_str(),
            cur_thread_addr,
            res
        );
        goto fail;
    }

    // init timer
    a_thread->m_timer = evtimer_new(
        a_thread->m_event_base,
        Thread::timer_cb,
        (void*)a_thread
    );
    evtimer_add(
        a_thread->m_timer,
        &a_thread->m_timer_interval
    );

    // update state
    a_thread->setState(Thread::THREAD_STATE_RUNNING);

    PINFO("thread: '%s' (0x%llx),"
        " running..\n",
        a_thread->getName().c_str(),
        cur_thread_addr
    );

    // call 'loop'
    a_thread->loop();

    a_thread->setState(Thread::THREAD_STATE_SHUTDOWNING);

    // state Thread::THREAD_STATE_SHUTDOWNING already set
    // in stop()

    PINFO("thread: '%s' (0x%llx),"
        " shutdown..\n",
        a_thread->getName().c_str(),
        cur_thread_addr
    );

    // call clean things
    a_thread->loop_after();

    // all ok
    err = 0;

    a_thread->setState(Thread::THREAD_STATE_SHUTDOWN);

out:
    return err;
fail:
    a_thread->setState(THREAD_STATE_ERROR_CANNOT_INIT);
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void Thread::setState(
    const Thread::ThreadState &a_state)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_state = a_state;
}

DbSptr Thread::getDb()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_db;
}

void Thread::getParents(
    vector<string>  &a_out)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ParentsIt it;

    for (it = m_parents.begin();
        it != m_parents.end();
        it++)
    {
        string parent_name = it->first;
        a_out.push_back(parent_name);
    }
}

void Thread::addParent(
    const string &a_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_parents[a_name] = 1;
}

void Thread::addParent(
    ThreadSptr a_thread)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string name = a_thread->getName();
    addParent(name);
}

void Thread::close(
    struct bufferevent *a_bev)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t fd = bufferevent_getfd(a_bev);

    bufferevent_free(a_bev);

    m_bevs.erase(fd);
    m_io_subscr.erase(a_bev);
}

void Thread::close(
    const int32_t   &a_fd)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    BevsIt it;

    if (0 > a_fd){
        goto out;
    }

    it = m_bevs.find(a_fd);
    if (m_bevs.end() != it){
        struct bufferevent *bev = NULL;
        bev = it->second;
        close(bev);
    } else {
        // for just accepted sockets bev yet not exist,
        // so just close fd
        ::close(a_fd);
    }

out:
    return;
}

ObjectContextSptr Thread::contextGet()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_context;
}

void Thread::contextSet(
    ObjectContextSptr a_context)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_context = a_context;
}

ObjectContextSptr Thread::contextReset()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectContextSptr context;
    PROP_CONTEXT(context);
    m_context = context;
    return context;
}

void Thread::contextTransCommit()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    if (m_context){
        m_context->transactionCommit();
    }
}

void Thread::accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    Thread            *thread = static_cast<Thread *>(a_ctx);
    ObjectContextSptr context = thread->contextGet();
    ObjectMapSptr     accept_subscr;
    ObjectMapIt       accept_subscr_it;
    int32_t           res;

    PTIMING(0.0f, "accept\n");

    // make socket non-blocked
    res = evutil_make_socket_nonblocking(a_fd);
    if (res < 0){
        PERROR("Cannot make fd: '%d' not blocking\n",
            a_fd
        );
        goto fail;
    }

    accept_subscr = thread->getAcceptSubsr(a_listener);
    if (accept_subscr){
again:
        for (accept_subscr_it = accept_subscr->begin();
            accept_subscr_it != accept_subscr->end();
            accept_subscr_it++)
        {
            ObjectSptr          object;
            ObjectActionSptr    action;
            ObjectInt32Sptr     prop_fd;
            ObjectStringSptr    prop_thread_name;
            ObjectUint64Sptr    prop_thread_addr;
            string              object_id;

            object_id = accept_subscr_it->first->toString();

            PDEBUG(50, "send action 'accept' to '%s'\n",
                object_id.c_str()
            );

            {
                PTIMING(0.0f, "get object: '%s'\n",
                    object_id.c_str()
                );

                // search object
                object = context->getObjectById(object_id);
                if (!object){
                    // object not found, maybe was removed?
                    // unsubscribe
                    accept_subscr->erase(object_id);
                    goto again;
                }
            }

            // create action
            OBJECT_ACTION(action, "thread.io.accept");

            PROP_INT32(prop_fd, a_fd);
            PROP_STRING(
                prop_thread_name,
                Thread::getCurThreadName()
            );
            PROP_UINT64(
                prop_thread_addr,
                Thread::getCurThreadAddr()
            );

            action->addParam("fd",          prop_fd);
            action->addParam("thread_name", prop_thread_name);
            action->addParam("thread_addr", prop_thread_addr);

            // send action
            // object not locked, so don't create backlink
            Object::addNeigh(action, object, 0);
            action->scheduleCurThread();
            context->transactionAddForSave(action);
            //action->save();

            //object->addAction(action);
            //object->scheduleCurThread();
            //object->schedule();
            //object->save();
        }
    }

    context->transactionCommit();
    thread->contextReset();

out:
    return;
fail:
    goto out;
}

void Thread::read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    Thread *thread = static_cast<Thread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_mutex);

    char              buffer[65535] = { 0x00 };
    ObjectContextSptr context       = thread->contextGet();
    int32_t           res, fd       = bufferevent_getfd(a_bev);
    ObjectMapSptr     accept_subscr;
    ObjectMapIt       accept_subscr_it;
    ObjectBinSptr     prop_data;
    string            data;

    do {
        res = bufferevent_read(
            a_bev,
            buffer,
            sizeof(buffer)
        );
        if (0 < res){
            data.append(buffer, res);
        }
    } while (res > 0);

    PDEBUG(50, "thread: '%s' (%llx)"
        " was read '%d' byte(s)"
        " from fd: '%d'\n"
        "%s"
        "\n",
        thread->getName().c_str(),
        thread->getAddr(),
        data.size(),
        fd,
        Utils::dump(data).c_str()
    );

    if (!data.empty()){
        PROP_BIN(prop_data, data);
again:
        ObjectMapSptr   io_subscr;
        ObjectMapIt     io_subscr_it;

        io_subscr = thread->getIoSubsr(a_bev);
        for (io_subscr_it = io_subscr->begin();
            io_subscr_it != io_subscr->end();
            io_subscr_it++)
        {
            ObjectSptr          object;
            ObjectActionSptr    action;
            ObjectInt32Sptr     prop_fd;
            ObjectStringSptr    prop_thread_name;
            ObjectUint64Sptr    prop_thread_addr;
            string              object_id;

            object_id = io_subscr_it->first->toString();

            PDEBUG(50, "send action 'thread.io.read' to '%s'\n",
                object_id.c_str()
            );

            // search object (without lock)
            object = context->getObjectById(object_id);
            if (!object){
                // object not found, maybe was removed?
                // unsubscribe
                io_subscr->erase(object_id);
                goto again;
            }

            // create action
            OBJECT_ACTION(action, "thread.io.read");

            PROP_INT32(prop_fd, fd);
            PROP_STRING(prop_thread_name, Thread::getCurThreadName());
            PROP_UINT64(prop_thread_addr, Thread::getCurThreadAddr());

            action->addParam("fd",          prop_fd);
            action->addParam("thread_name", prop_thread_name);
            action->addParam("thread_addr", prop_thread_addr);
            action->addParam("data",        prop_data);

            // send action
            // object not locked, so don't create backlink
            Object::addNeigh(action, object, 0);
            action->scheduleCurThread();
            context->transactionAddForSave(action);
            //action->save();
        }
    }

    context->transactionCommit();
    thread->contextReset();
}

void Thread::write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    Thread *thread = static_cast<Thread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_mutex);

    ObjectContextSptr context = thread->contextGet();
    int32_t           fd      = bufferevent_getfd(a_bev);

    PWARN("Thread::write_cb, fd: '%d'\n", fd);

    ObjectMapSptr   io_subscr;
    ObjectMapIt     io_subscr_it;

    PDEBUG(40, "thread: '%s' (0x%llx) write_cb\n",
        thread->getName().c_str(),
        thread->getAddr()
    );

again:
    io_subscr = thread->getIoSubsr(a_bev);
    for (io_subscr_it = io_subscr->begin();
        io_subscr_it != io_subscr->end();
        io_subscr_it++)
    {
        ObjectSptr          object;
        ObjectActionSptr    action;
        ObjectInt32Sptr     prop_fd;
        ObjectStringSptr    prop_thread_name;
        ObjectUint64Sptr    prop_thread_addr;
        ObjectUniqueSptr    prop_flags;
        string              object_id;

        object_id = io_subscr_it->first->toString();

        PDEBUG(50, "send action 'thread.io.write' to '%s'\n",
            object_id.c_str()
        );

        // search object
        object = context->getObjectById(object_id);
        if (!object){
            // object not found, maybe was removed?
            // unsubscribe
            io_subscr->erase(object_id);
            goto again;
        }

        // create action
        OBJECT_ACTION(action, "thread.io.write");

        PROP_INT32(prop_fd, fd);
        PROP_STRING(prop_thread_name, Thread::getCurThreadName());
        PROP_UINT64(prop_thread_addr, Thread::getCurThreadAddr());
        PROP_UNIQUE(prop_flags);

        prop_flags->add("write");

        action->addParam("fd",          prop_fd);
        action->addParam("thread_name", prop_thread_name);
        action->addParam("thread_addr", prop_thread_addr);
        action->addParam("flags",       prop_flags);

        // send action
        // object not locked, so don't create backlink
        Object::addNeigh(action, object, 0);
        action->scheduleCurThread();
        context->transactionAddForSave(action);
        //action->save();
    }

    context->transactionCommit();
    thread->contextReset();
}

void Thread::event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx)
{
    Thread *thread = static_cast<Thread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_mutex);

    ObjectContextSptr context = thread->contextGet();
    int32_t           fd      = bufferevent_getfd(a_bev);

    ObjectMapSptr   io_subscr;
    ObjectMapIt     io_subscr_it;

    PDEBUG(40, "thread: '%s' (0x%llx)\n"
        "  fd:      '%d'\n"
        "  a_what:  '%d'\n",
        thread->getName().c_str(),
        thread->getAddr(),
        fd,
        a_what
    );

again:
    io_subscr = thread->getIoSubsr(a_bev);
    for (io_subscr_it = io_subscr->begin();
        io_subscr_it != io_subscr->end();
        io_subscr_it++)
    {
        ObjectSptr          object;
        ObjectActionSptr    action;
        ObjectInt32Sptr     prop_fd;
        ObjectStringSptr    prop_thread_name;
        ObjectUint64Sptr    prop_thread_addr;
        ObjectUniqueSptr    prop_flags;
        string              object_id;

        object_id = io_subscr_it->first->toString();

        PDEBUG(50, "send action 'event' to '%s'\n",
            object_id.c_str()
        );

        // search object
        object = context->getObjectById(object_id);
        if (!object){
            // object not found, maybe was removed?
            // unsubscribe
            io_subscr->erase(object_id);
            goto again;
        }

        // create action
        OBJECT_ACTION(action, "thread.io.event");

        PROP_INT32(prop_fd, fd);
        PROP_STRING(prop_thread_name, Thread::getCurThreadName());
        PROP_UINT64(prop_thread_addr, Thread::getCurThreadAddr());
        PROP_UNIQUE(prop_flags);

        if (BEV_EVENT_READING & a_what){
            // error encountered while reading
            prop_flags->add("reading");
        }
        if (BEV_EVENT_WRITING & a_what){
            // error encountered while writing
            prop_flags->add("writing");
        }
        if (BEV_EVENT_EOF & a_what){
            // eof file reached
            prop_flags->add("eof");
        }
        if (BEV_EVENT_ERROR & a_what){
            // unrecoverable error encountered
            prop_flags->add("error");
        }
        if (BEV_EVENT_TIMEOUT & a_what){
            // user-specified timeout reached
            prop_flags->add("timeout");
        }
        if (BEV_EVENT_CONNECTED & a_what){
            // connect operation finished
            prop_flags->add("connected");
        }

        action->addParam("fd",          prop_fd);
        action->addParam("thread_name", prop_thread_name);
        action->addParam("thread_addr", prop_thread_addr);
        action->addParam("flags",       prop_flags);

        // send action
        // object not locked, so don't create backlink
        Object::addNeigh(action, object, 0);
        action->scheduleCurThread();
        context->transactionAddForSave(action);
        //action->save();
    }

    if (    (BEV_EVENT_EOF     & a_what)
        &&  (BEV_EVENT_READING & a_what))
    {
        // connection was closed
        PDEBUG(10, "thread: '%s' (%llx)"
            " connection was closed by remote part"
            " (what: '%d')\n",
            thread->getName().c_str(),
            thread->getAddr(),
            a_what
        );
        goto close;
    }

    if (BEV_EVENT_ERROR & a_what){
        // connection was closed
        PDEBUG(10, "thread: '%s' (%llx)"
            " connection was closed because of error"
            " (what: '%d')\n",
            thread->getName().c_str(),
            thread->getAddr(),
            a_what
        );
        goto close;
    }

    if (BEV_EVENT_TIMEOUT & a_what){
        // connection was closed
        PDEBUG(10, "thread: '%s' (%llx)"
            " connection was closed because of timeout"
            " (what: '%d')\n",
            thread->getName().c_str(),
            thread->getAddr(),
            a_what
        );
        goto close;
    }

out:
    context->transactionCommit();
    thread->contextReset();

    return;
close:
    thread->close(a_bev);
    goto out;
}

// ---------------- evconn ----------------

struct evconnlistener * Thread::getEvConn(
    const string    &a_addr,
    const int32_t   &a_port)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char                    buffer[512] = { 0x00 };
    struct evconnlistener   *listener   = NULL;
    AcceptsIt               accepts_it;

    snprintf(buffer, sizeof(buffer),
        "%s:%d",
        a_addr.c_str(),
        a_port
    );

    accepts_it = m_accepts.find(buffer);
    if (m_accepts.end() != accepts_it){
        listener = accepts_it->second;
    }

    return listener;
}

struct evconnlistener * Thread::getCreateEvConn(
    const string    &a_addr,
    const int32_t   &a_port)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    struct evconnlistener   *listener = NULL;
    struct sockaddr_in      addr;

    // maybe exist
    listener = getEvConn(a_addr, a_port);
    if (listener){
        goto out;
    }

    memset((char *)&addr, 0x00, sizeof(addr));

    // TODO XXX FIXME
    // remove hardcode INADDR_LOOPBACK
    // 20141028 morik@
    addr.sin_addr.s_addr  = htonl(INADDR_LOOPBACK);
    addr.sin_family       = AF_INET;
    addr.sin_port         = htons(a_port);

    listener = evconnlistener_new_bind(
    //listener = evconnlistener_hostos_new_bind(
        getEvBase(),
        Thread::accept_cb,
        (void *)this,
	    LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        -1,
	    (struct sockaddr*)&addr,
	    sizeof(addr)
    );
	if (!listener){
		PERROR("cannot not create a listener at %s:%d\n",
            a_addr.c_str(),
            a_port
        );
        goto fail;
	}

out:
    return listener;
fail:
    if (listener){
        evconnlistener_free(listener);
        listener = NULL;
    }
    goto out;
}

// ---------------- accept ----------------

ObjectMapSptr Thread::getAcceptSubsr(
    struct evconnlistener   *a_listener)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    AcceptsSubscrIt it;
    ObjectMapSptr   prop_map;

    it = m_accepts_subscr.find(a_listener);
    if (m_accepts_subscr.end() != it){
        prop_map = it->second;
    }

    return prop_map;
}

void Thread::getCreateAcceptSubsr(
    struct evconnlistener   *a_listener,
    const string            &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr prop_map;

    prop_map = getAcceptSubsr(a_listener);
    if (!prop_map){
        // subscribe
        ObjectStringSptr    prop_object_id;
        ObjectStringSptr    prop_not_used;

        PROP_MAP(prop_map);
        PROP_STRING(prop_object_id, a_object_id);
        PROP_STRING(prop_not_used);

        prop_map->add(prop_object_id, prop_not_used);

        m_accepts_subscr[a_listener] = prop_map;
    }
}

int32_t Thread::subscrAccept(
    ObjectSptr      a_object,
    const string    &a_addr,
    const int32_t   &a_port)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t                 err       = -1;
    struct evconnlistener   *listener = NULL;

    string object_id = a_object->getId();

    PWARN("object ID: '%s',"
        " request subscr for accept from: '%s':%d\n",
        object_id.c_str(),
        a_addr.c_str(),
        a_port
    );

    // maybe already created?
    listener = getCreateEvConn(a_addr, a_port);
    if (!listener){
        PERROR("cannot create listener for %s:%d\n",
            a_addr.c_str(),
            a_port
        );
        goto fail;
    }

    getCreateAcceptSubsr(listener, object_id);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 < err){
        err = -1;
    }
    goto out;
}

// ---------------- bev ----------------

struct bufferevent * Thread::getBev(
    const int32_t   &a_fd)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    struct bufferevent  *bev = NULL;
    BevsIt              it;

    it = m_bevs.find(a_fd);
    if (m_bevs.end() != it){
        bev = it->second;
    }

    return bev;
}

struct bufferevent * Thread::getCreateBev(
    const int32_t   &a_fd)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    struct bufferevent *bev = NULL;

    bev = getBev(a_fd);
    if (bev){
        goto out;
    }

    bev = bufferevent_socket_new(
        getEvBase(),
        a_fd,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!bev){
        PERROR("cannot create bufferevent_socket_new"
            " for fd: '%d'\n",
            a_fd
        );
        goto fail;
    }

    // store bev
    m_bevs[a_fd] = bev;

out:
    return bev;
fail:
    if (bev){
        bufferevent_free(bev);
        bev = NULL;
    }
    goto out;
}

// ---------------- IO subscr ----------------

ObjectMapSptr Thread::getIoSubsr(
    struct bufferevent *a_bev)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    IoSubscrIt      it;
    ObjectMapSptr   prop_map;

    it = m_io_subscr.find(a_bev);
    if (m_io_subscr.end() != it){
        prop_map = it->second;
    }

    return prop_map;
}

void Thread::getCreateIoSubsr(
    struct bufferevent  *a_bev,
    const string        &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr prop_map;

    prop_map = getIoSubsr(a_bev);
    if (!prop_map){
        // subscribe
        ObjectStringSptr    prop_object_id;
        ObjectStringSptr    prop_not_used;

        PROP_MAP(prop_map);
        PROP_STRING(prop_object_id, a_object_id);
        PROP_STRING(prop_not_used);

        prop_map->add(prop_object_id, prop_not_used);

        m_io_subscr[a_bev] = prop_map;
    }
}

int32_t Thread::subscrIo(
    const string    &a_object_id,
//    Object          *a_object,
    const int32_t   &a_fd)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t             err     = -1;
    struct bufferevent  *bev    = NULL;

//    string object_id = a_object->getId()->toString();

    PWARN("object ID: '%s',"
        " subscr io for fd: '%d'\n",
        a_object_id.c_str(),
        a_fd
    );

    if (0 > a_fd){
        PERROR("invalid fd: '%d'\n", a_fd);
        goto fail;
    }

    // maybe already created?
    bev = getCreateBev(a_fd);
    if (!bev){
        PERROR("cannot create buffer event for fd: '%d'\n",
            a_fd
        );
        goto fail;
    }

    getCreateIoSubsr(bev, a_object_id);

    bufferevent_setcb(
        bev,
        Thread::read_cb,
        Thread::write_cb,
        Thread::event_cb,
        this
    );

    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    if (bev){
        bufferevent_free(bev);
        bev = NULL;
    }
    goto out;
}

// ---------------- evbase ----------------

struct event_base * Thread::getEvBase()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_event_base;
}

// ---------------- static ----------------

void Thread::timer_cb(
    evutil_socket_t     a_fd,
    short               a_event,
    void                *a_arg)
{
    Thread  *thread     = (Thread *)a_arg;
    int32_t reschedule  = 1;

    PDEBUG(60, "thread: '%s' (0x%llx)"
        " tick\n",
        thread->getName().c_str(),
        thread->getAddr()
    );

    switch (thread->getState()){
        case Thread::THREAD_STATE_RUNNING:
            thread->loop_tick();
            break;

        case Thread::THREAD_STATE_SHUTDOWNING:
            reschedule = 0;
            thread->loopbreak();
            break;

        default:
            break;
    }

    if (reschedule){
        evtimer_add(
            thread->m_timer,
            &thread->m_timer_interval
        );
    }
}

string Thread::getCurThreadName()
{
    ThreadSptr thread = Thread::getCurThread();
    return thread->getName();
}

uint64_t Thread::getCurThreadAddr()
{
    thread::id  cur_thread_id   = this_thread::get_id();
    uint64_t    cur_thread_addr = 0;

    {
        stringstream tmp;
        tmp << cur_thread_id;
        cur_thread_addr = atoll(tmp.str().c_str());
    }

    return cur_thread_addr;
}

ThreadSptr Thread::getCurThread()
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    uint64_t    cur_thread_addr = 0;
    ThreadSptr  thread;

    cur_thread_addr = Thread::getCurThreadAddr();
    thread          = Thread::getThreadByAddr(cur_thread_addr);

    return thread;
}

ThreadSptr Thread::getThreadByAddr(
    const uint64_t &a_addr)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadSptr      ret;
    ThreadsByAddrIt it;

    it = g_threads_by_addr.find(a_addr);
    if (g_threads_by_addr.end() != it){
        ret = it->second;
    }

    return ret;
}

ThreadSptr Thread::getThreadByName(
    const string &a_name)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadSptr      ret;
    ThreadsByNameIt it;

    it = g_threads_by_name.find(a_name);
    if (g_threads_by_name.end() != it){
        ret = it->second;
    }

    return ret;
}

void Thread::startThreads(
    const ThreadsByName &a_threads)
{
    ThreadsByNameConstIt it;

    uint32_t total_count = a_threads.size();

    while (1){
        uint32_t running_count = 0;

        for (it = a_threads.begin();
            it != a_threads.end();
            it++)
        {
            ThreadSptr          thread = it->second;
            //string              name   = thread->getName();
            //uint64_t            addr   = thread->getAddr();
            Thread::ThreadState state  = thread->getState();

            if (Thread::THREAD_STATE_CREATED == state){
                thread->start();
            } else if (Thread::THREAD_STATE_RUNNING == state){
                running_count++;
            }
        }

        if (total_count == running_count){
            break;
        }

        usleep(0.5*1e6);
    }
}

void Thread::stopThreads(
    const ThreadsByName &a_threads)
{
    ThreadsByNameConstIt it;

    for (it = a_threads.begin();
        it != a_threads.end();
        it++)
    {
        ThreadSptr          thread = it->second;
        //string              name   = thread->getName();
        //uint64_t            addr   = thread->getAddr();
        Thread::ThreadState state  = thread->getState();

        if (Thread::THREAD_STATE_RUNNING == state){
            thread->stop();
        }
    }
}

void Thread::getThreadsWithoutParents(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;

    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr      thread      = it->second;
        string          thread_name = thread->getName();
        vector<string>  parents;

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        thread->getParents(parents);
        if (!parents.empty()){
            continue;
        }
        a_out[thread_name] = thread;
    }
}

void Thread::getThreadsWithRunningParents(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;

    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr      thread      = it->second;
        string          thread_name = thread->getName();
        int32_t         all_running = 1;

        vector<string>              parents;
        vector<string>::iterator    parents_it;

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        thread->getParents(parents);
        for (parents_it = parents.begin();
            parents_it != parents.end();
            parents_it++)
        {
            string              parent_name = *parents_it;
            ThreadSptr          parent;
            Thread::ThreadState state;

            parent = Thread::getThreadByName(parent_name);
            state  = parent->getState();

            if (Thread::THREAD_STATE_RUNNING != state){
                all_running = 0;
                break;
            }
        }

        if (all_running){
            a_out[thread_name] = thread;
        }
    }
}

void Thread::getThreadsForRun(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    getThreadsWithoutParents(a_out,     a_skip_main);
    getThreadsWithRunningParents(a_out, a_skip_main);
}

void Thread::getThreadsRunning(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;

    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr          thread       = it->second;
        string              thread_name  = thread->getName();
        Thread::ThreadState thread_state = thread->getState();

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        if (Thread::THREAD_STATE_RUNNING == thread_state){
            a_out[thread_name] = thread;
        }
    }
}

void Thread::removeShutdownedThreads(
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;

again:
    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr          thread       = it->second;
        string              thread_name  = thread->getName();
        uint64_t            thread_addr  = thread->getAddr();
        Thread::ThreadState thread_state = thread->getState();

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        if (Thread::THREAD_STATE_SHUTDOWN >= thread_state){
            g_threads_by_name.erase(thread_name);
            g_threads_by_addr.erase(thread_addr);
            goto again;
        }
    }
}

void Thread::getThreadsAll(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;

    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr      thread      = it->second;
        string          thread_name = thread->getName();
        vector<string>  parents;

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        a_out[thread_name] = thread;
    }
}

void Thread::startAllThreads()
{
    ThreadsByNameIt it;
    ThreadsByName   threads_for_start;
    ThreadsByName   threads_running;
    ThreadsByName   threads_all;

    PDEBUG(10, "request start all threads\n");

    do {
        threads_for_start.clear();
        threads_running.clear();
        threads_all.clear();

        Thread::getThreadsForRun(threads_for_start, 1);
        Thread::getThreadsRunning(threads_running,  1);
        Thread::getThreadsAll(threads_all,          1);

        if (!threads_for_start.empty()){
            PDEBUG(10, "attempt to start threads: %d/%d/%d"
                " (all, start now, running)\n",
                threads_all.size(),
                threads_for_start.size(),
                threads_running.size()
            );
            startThreads(threads_for_start);
        }

        Thread::removeShutdownedThreads();

        usleep(0.5*1e6);
    } while (threads_all.size() != threads_running.size());

    PINFO("all threads started\n");
}

void Thread::getThreadsWithoutChildren(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt outer_it;
    ThreadsByNameIt inner_it;

    for (outer_it = g_threads_by_name.begin();
        outer_it != g_threads_by_name.end();
        outer_it++)
    {
        ThreadSptr      thread      = outer_it->second;
        string          thread_name = thread->getName();
        vector<string>  parents;
        int32_t         have_children = 0;

        if (    a_skip_main
            &&  "main" == thread_name)
        {
            continue;
        }

        // search in all parent's lists
        for (inner_it = g_threads_by_name.begin();
            inner_it != g_threads_by_name.end();
            inner_it++)
        {
            ThreadSptr  cur_thread      = inner_it->second;
            string      cur_thread_name = thread->getName();

            vector<string>              parents;
            vector<string>::iterator    parents_it;

            cur_thread->getParents(parents);
            for (parents_it = parents.begin();
                parents_it != parents.end();
                parents_it++)
            {
                string  parent_name = *parents_it;

                if (thread_name == parent_name){
                    have_children = 1;
                    break;
                }
            }

            if (have_children){
                break;
            }
        }

        if (!have_children){
            a_out[thread_name] = thread;
        }
    }
}

void Thread::getThreadsForStop(
    ThreadsByName   &a_out,
    const int32_t   &a_skip_main)
{
    getThreadsWithoutChildren(a_out, a_skip_main);
}

void Thread::stopAllThreads()
{
//    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadsByNameIt it;
    ThreadsByName   threads_for_stop;
    ThreadsByName   threads_running;
    ThreadsByName   threads_all;

    PDEBUG(10, "request stop all threads\n");

    do {
        threads_for_stop.clear();
        threads_running.clear();
        threads_all.clear();

        Thread::getThreadsForStop(threads_for_stop, 1);
        Thread::getThreadsRunning(threads_running,  1);
        Thread::getThreadsAll(threads_all,          1);

        if (!threads_for_stop.empty()){
            PDEBUG(10, "attempt to stop threads: %d/%d/%d"
                " (all, stop now, running)\n",
                threads_all.size(),
                threads_for_stop.size(),
                threads_running.size()
            );
            stopThreads(threads_for_stop);
        }

        Thread::removeShutdownedThreads();

        usleep(0.5*1e6);
    } while (!threads_all.empty());

    PINFO("all threads stopped\n");
}

void Thread::joinAll()
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);
    ThreadsByNameIt it;
    for (it = g_threads_by_name.begin();
        it != g_threads_by_name.end();
        it++)
    {
        ThreadSptr thread = it->second;
        if (thread->joinable()){
            thread->join();
        }
    }
}

ObjectContextSptr Thread::getCurContext()
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadSptr        cur_thread = Thread::getCurThread();
    ObjectContextSptr context;

    if (cur_thread){
        context = cur_thread->contextGet();
    }

    return context;
}

void Thread::setCurContext(
    ObjectContextSptr a_context)
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadSptr  cur_thread = Thread::getCurThread();
    cur_thread->contextSet(a_context);
}

ObjectContextSptr Thread::resetCurContext()
{
    lock_guard<recursive_mutex> guard(g_threads_mutex);

    ThreadSptr  cur_thread = Thread::getCurThread();
    return cur_thread->contextReset();
}

bool Thread::isSameThread(
    ObjectMapSptr   a_req_params)
{
    bool                ret = false;
    ObjectStringSptr    prop_thread_name;
    ObjectUint64Sptr    prop_thread_addr1;
    ObjectInt64Sptr     prop_thread_addr2;
    string              thread_name;
    uint64_t            thread_addr;
    string              field_name;

    // thread_name
    field_name       = "thread_name";
    prop_thread_name = dynamic_pointer_cast<ObjectString>(
        a_req_params->get(field_name)
    );
    if (!prop_thread_name){
        PFATAL("missing '%s' param\n",
            field_name.c_str()
        );
    }
    thread_name = prop_thread_name->toString();

    // thread_addr
    field_name        = "thread_addr";
    prop_thread_addr1 = dynamic_pointer_cast<ObjectUint64>(
        a_req_params->get(field_name)
    );
    if (prop_thread_addr1){
        thread_addr = prop_thread_addr1->getVal();
    } else {
        prop_thread_addr2 = dynamic_pointer_cast<ObjectInt64>(
            a_req_params->get(field_name)
        );
        if (prop_thread_addr2){
            thread_addr = static_cast<uint64_t>(
                prop_thread_addr2->getVal()
            );
        } else {
            PFATAL("missing '%s' param\n",
                field_name.c_str()
            );
        }
    }

    if (    Thread::getCurThreadAddr() == thread_addr
        &&  Thread::getCurThreadName() == thread_name)
    {
        ret = true;
    }

    PWARN("thread: '%s' (0x%llx) ret: '%d'\n",
        thread_name.c_str(),
        thread_addr,
        ret
    );

    return ret;
}

DbSptr Thread::getCurThreadDb()
{
    ThreadSptr cur_thread = Thread::getCurThread();
    DbSptr     cur_db     = cur_thread->getDb();

    return cur_db;
}

string Thread::processPerlCode(
    const string &a_code)
{
    string     ret;
    ThreadSptr thread = Thread::getCurThread();

    ret = thread->_processPerlCode(a_code);

    return ret;
}

string Thread::processPerlSub(
    const string    &a_sub_name,
    ObjectMapSptr   a_arg)
{
    string     ret;
    ThreadSptr thread = Thread::getCurThread();

    ret = thread->_processPerlSub(
        a_sub_name,
        a_arg
    );

    return ret;
}

