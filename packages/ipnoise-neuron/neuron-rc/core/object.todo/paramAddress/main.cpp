#include <sqlite3.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/param/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "strings.hpp"
#include "kladr.hpp"

#include "core/object/paramAddress/main.hpp"

ObjectParamAddress::ObjectParamAddress(
    const Object::CreateFlags   &a_flags)
    :   ObjectParam(a_flags)
{
    do_init(a_flags);
};

ObjectParamAddress::~ObjectParamAddress()
{
};

void ObjectParamAddress::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectParamAddress::getType()
{
    return ObjectParamAddress::_getType();
}

void ObjectParamAddress::do_init_props()
{
    ObjectParam::do_init_props();

//    ObjectTypeSptr      prop_type;
    ObjectStringSptr    prop_name;
    ObjectVectorSptr    prop_levels;

//    PROP_TYPE(   prop_type, "core.object.param.address");
    PROP_STRING( prop_name, "address");
    PROP_VECTOR( prop_levels);

//    INIT_PROP(this, Type,   prop_type);
    INIT_PROP(this, Name,   prop_name);
    INIT_PROP(this, Levels, prop_levels);
}

void ObjectParamAddress::do_init_api()
{
    ObjectParam::do_init_api();

    // register actions
    registerAction(
        "getLevel",
        ObjectParamAddress::actionGetLevel
    );
    registerAction(
        "getRegionTypes",
        ObjectParamAddress::actionGetRegionTypes
    );
}

int32_t ObjectParamAddress::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectParamAddress::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("levels", getLevels);

    ObjectParam::getAllProps(a_props);
}

int32_t ObjectParamAddress::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // levels
    if (a_obj.hasField("levels")){
        mongo::BSONElement  el      = a_obj.getField("levels");
        mongo::BSONType     type    = el.type();

        if (mongo::BSONType::Array == type){
            vector< mongo::BSONElement >           els;
            vector< mongo::BSONElement >::iterator els_it;
            ObjectVectorSptr levels;
            PROP_VECTOR(levels);

/*
            els = el.Array();
            for (els_it = els.begin();
                els_it != els.end();
                els_it++)
            {
                mongo::BSONObj      cur_obj   = els_it->Obj();
                mongo::BSONElement  name      = cur_obj.getField("name");
                mongo::BSONElement  val       = cur_obj.getField("val");
                mongo::BSONElement  type_name = cur_obj.getField("type_name");
                mongo::BSONElement  type_val  = cur_obj.getField("type_val");

                ObjectMapSptr map(new ObjectMap);

                map->add("name",         name);
                map->add("val",          val);
                map->add("type_name",    type_name);
                map->add("type_val",     type_val);
                levels->add(map);
            }
*/


            PFATAL("FIXME\n");
/*
            ThreadDb::bsonToProps(el.Obj(), levels);
            initLevels(levels);
*/
        } else {
            err = -1;
            PERROR("Cannot parse: '%s', 'levels' is not array\n",
                a_obj.jsonString().c_str()
            );
        }
    }

    err = ObjectParam::parseBSON(a_obj);
    return err;
}

void ObjectParamAddress::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
    char                buffer[1024] = { 0x00 };
    ObjectMapSptr       answer_params;
    ObjectVectorSptr    results;
    ObjectMapSptr       req_params;
    KladrReqInfo        kladr_req_info;
    ObjectVectorSptr    object_levels;
    ObjectVectorSptr    levels;
    int32_t             level_id = 0;
    ObjectParamAddress  *object  = NULL;

    object = static_cast<ObjectParamAddress *>(a_object);

    PROP_VECTOR(levels);
    PROP_VECTOR(results);

    answer_params = a_answer->getCreateAnswerParams();
    object_levels = dynamic_pointer_cast<ObjectVector>(
        a_req_params->get("levels")
    );
    if (!object_levels){
        string answer;
        string status = "failed";
        string descr  = "missing param 'levels'"
            " or it is not array";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
        goto fail;
    }

    {
        ObjectVectorIt levels_it;
        for (levels_it = object_levels->begin();
            levels_it != object_levels->end();
            levels_it++, level_id++)
        {
            ObjectMapSptr object_level;
            ObjectMapSptr level;
            ObjectSptr    name;
            ObjectSptr    val;
            ObjectSptr    type_name;
            ObjectSptr    type_val;

            PROP_MAP(level);

            // object_level
            object_level = dynamic_pointer_cast<ObjectMap>(
                *levels_it
            );
            if (!object_level){
                // it is not map, skip
                string status = "failed";
                snprintf(buffer, sizeof(buffer),
                    "cannot parse level: '%d',"
                    " level must be a hash",
                    level_id
                );
                a_answer->setAnswerStatusCode(200);
                a_answer->setAnswerStatusString("OK");
                a_answer->setAnswerApiStatus(status);
                a_answer->setAnswerApiDescr(buffer);
                goto fail;
            }

            // name
            name = object_level->get("name");
            if (!name){
                string status = "failed";
                snprintf(buffer, sizeof(buffer),
                    "cannot parse level: '%d',"
                    " missing argument 'name'",
                    level_id
                );
                a_answer->setAnswerStatusCode(200);
                a_answer->setAnswerStatusString("OK");
                a_answer->setAnswerApiStatus(status);
                a_answer->setAnswerApiDescr(buffer);
                goto fail;
            }

            // val
            val = object_level->get("val");
            if (!val){
                string status = "failed";
                snprintf(buffer, sizeof(buffer),
                    "cannot parse level: '%d',"
                    " missing argument 'val'",
                    level_id
                );
                a_answer->setAnswerStatusCode(200);
                a_answer->setAnswerStatusString("OK");
                a_answer->setAnswerApiStatus(status);
                a_answer->setAnswerApiDescr(buffer);
                goto fail;
            }

            // type_name
            type_name = object_level->get("type_name");
            if (!type_name){
                string status = "failed";
                snprintf(buffer, sizeof(buffer),
                    "cannot parse level: '%d',"
                    " missing argument 'type_name'",
                    level_id
                );
                a_answer->setAnswerStatusCode(200);
                a_answer->setAnswerStatusString("OK");
                a_answer->setAnswerApiStatus(status);
                a_answer->setAnswerApiDescr(buffer);
                goto fail;
            }

            // type_val
            type_val = object_level->get("type_val");
            if (!type_val){
                string status = "failed";
                snprintf(buffer, sizeof(buffer),
                    "cannot parse level: '%d',"
                    " missing argument 'type_val'",
                    level_id
                );
                a_answer->setAnswerStatusCode(200);
                a_answer->setAnswerStatusString("OK");
                a_answer->setAnswerApiStatus(status);
                a_answer->setAnswerApiDescr(buffer);
                goto fail;
            }

            // all ok, collect
            level->add("name",        name);
            level->add("val",         val);
            level->add("type_name",   type_name);
            level->add("type_val",    type_val);
            levels->add(level);
        }
    }

    // all ok, store levels
    object->setLevels(levels);

    // process upper objects
    ObjectParam::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );

out:
    return;
fail:
    goto out;
*/
}

int32_t ObjectParamAddress::kladrSqlCb(
    void    *a_kladr_req_info,
    int32_t a_argc,
    char    **a_argv,
    char    **a_col_name)
{
    int32_t          i, err = 0;
    KladrReqInfo     *kladr_req_info = (KladrReqInfo *)a_kladr_req_info;
    ObjectVectorSptr results         = kladr_req_info->m_results;
    ObjectMapSptr    fields;

    PROP_MAP(fields);

    for (i = 0; i < a_argc; i++){
        string      param_name      = a_col_name[i];
        const char  *param_val_ptr  = a_argv[i];
        string      param_val;

        if (param_val_ptr){
            param_val = param_val_ptr;
        }

        if (kladr_req_info->m_split_by_comma){
            string::size_type   pos;
            Splitter            tmp(param_val, ",");
            ObjectVectorSptr    parts;
            ObjectVectorIt      parts_it;

            PROP_VECTOR(parts);

            for (pos = 0; pos < tmp.size(); pos++){
                ObjectStringSptr cur_val;
                PROP_STRING(cur_val, tmp[pos]);
                parts->push_back(cur_val);
            }
            if (parts->size() > 1){
                parts->sort();
                fields->add(param_name, parts);
                continue;
            }
        }
        fields->add(param_name, param_val);
    }

    results->add(fields);

    return err;
}

void ObjectParamAddress::actionGetLevel(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*
//    int32_t         res;
    string              sql;
    string              sql_err_msg;
    ObjectMapSptr       answer_params;
    ObjectVectorSptr    results;
    KladrReqInfo        kladr_req_info;
    ObjectSptr          object_code;
    ObjectSptr          object_socr;
    ObjectSptr          object_get_socr;
    ObjectSptr          object_skip_socr;
    string              code;
    string              socr;
    string              get_socr = "1";
    string              skip_socr;

    answer_params = a_answer->getCreateAnswerParams();
    PROP_VECTOR(results);

    // code: AABBBCCC00000
    // AA       - level 1
    // BBB      - level 2
    // CCC      - level 3
    // 00000    - level 4

    // get request params
    if (a_req_params){
        object_code       = a_req_params->get("code");
        object_socr       = a_req_params->get("socr");
        object_get_socr   = a_req_params->get("get_socr");
        object_skip_socr  = a_req_params->get("skip_socr");
    }

    if (object_code){
        code = object_code->toString();
    }
    if (object_socr){
        socr = object_socr->toString();
    }
    if (object_get_socr){
        get_socr = object_get_socr->toString();
    }
    if (object_skip_socr){
        if ("core.object.vector"
            == object_skip_socr->getType())
        {
            ObjectVectorSptr  vector;
            ObjectVectorIt    vector_it;
            vector = dynamic_pointer_cast<ObjectVector>(
                object_skip_socr
            );
            for (vector_it = vector->begin();
                vector_it != vector->end();
                vector_it++)
            {
                skip_socr += " AND socr != '";
                skip_socr += (*vector_it)->toString();
                skip_socr += "'";
            }
        }
    }

    if (!code.size()){
        code = "%" "000" "000" "00000";
    }

    answer_params->add("results", results);

    // prepare request
    kladr_req_info.m_results = results;

    // TODO XXX escape 'code', 'socr', 'get_socr', 'skip_socr'
    {
        // fast
        sql = "";
        if (get_socr.size()){
            sql += "SELECT socr, socrname";
            sql += " FROM kladr, socrbase";
            sql += " WHERE";
            sql +=      " kladr.socr = socrbase.scname";
            sql +=      " AND code like '"+code+"'";
            sql += skip_socr;
            sql += " GROUP by socr";
        } else {
            sql += "SELECT code, name";
            sql += " FROM kladr";
            sql += " WHERE code like '"+code+"'";
            if (socr.size()){
                sql += " AND socr = '"+socr+"'";
            }
            sql += skip_socr;
            sql += " ORDER by name";
        }

        res = ThreadMain::get()->processKladrSql(
            sql,
            ObjectParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadHttp::set500(a_answer);
            goto fail;
        }
    }

    if (    2 < code.size()
        ||  !kladr_req_info.m_results->size())
    {
        // slow
        sql = "";
        if (get_socr.size()){
            sql += "SELECT socr, socrname";
            sql += " FROM street, socrbase";
            sql += " WHERE";
            sql +=      " street.socr = socrbase.scname";
            sql +=      " AND code like '"+code+"'";
            sql += skip_socr;
            sql += " GROUP by socr";
        } else {
            sql += "SELECT code, name";
            sql += " FROM street";
            sql += " WHERE code like '"+code+"'";
            if (socr.size()){
                sql += " AND socr = '"+socr+"'";
            }
            sql += skip_socr;
            sql += " ORDER by name";
        }

        res = ThreadMain::get()->processKladrSql(
            sql,
            ObjectParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadHttp::set500(a_answer);
            goto fail;
        }
    }

    if (!kladr_req_info.m_results->size()){
        // very slow
        sql = "";
        if (get_socr.size()){
            sql += "SELECT socr, socrname";
            sql += " FROM doma, socrbase";
            sql += " WHERE";
            sql +=      " doma.socr = socrbase.scname";
            sql +=      " AND code like '"+code+"'";
            sql += skip_socr;
            sql += " GROUP by socr";
        } else {
            sql += "SELECT code, name";
            sql += " FROM doma";
            sql += " WHERE code like '"+code+"'";
            if (socr.size()){
                sql += " AND socr = '"+socr+"'";
            }
            sql += skip_socr;
            sql += " ORDER by name";
            kladr_req_info.m_split_by_comma = 1;
        }

        res = ThreadMain::get()->processKladrSql(
            sql,
            ObjectParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadHttp::set500(a_answer);
            goto fail;
        }
    }

    {
        string      answer;
        string      status = "success";
        string      descr  = "level was fetched successfully";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }

//out:
    return;
//fail:
//    goto out;
*/
}

void ObjectParamAddress::actionGetRegionTypes(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");
/*
//    int32_t         res;
    string           sql;
    string           sql_err_msg;
    ObjectMapSptr    answer_params   = a_answer->getCreateAnswerParams();
    ObjectVectorSptr results;
    KladrReqInfo     kladr_req_info;
    ObjectSptr       object_region;
    string           region;

    PROP_VECTOR(results);

    // get request params
    if (a_req_params){
        object_region = a_req_params->get("region");
    }
    if (!object_region){
        string answer;
        string status = "failed";
        string descr  = "missing param 'region'";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);

        goto fail;
    }

    region = object_region->toString();
    region = region.substr(0, 7);
    if (7 != region.size()){
        string answer;
        string status = "failed";
        string descr  = "invalid length of param 'region'"
            " (must be 7 symbols)";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);

        goto fail;
    }

    // TODO XXX escape 'region'
    sql = ""
        "SELECT kladr.socr, socrbase.socrname"
        " FROM kladr, socrbase"
        " WHERE"
            " kladr.socr = socrbase.scname"
            " AND code like '"+region+"%'"
        " GROUP by socrbase.socrname";

    answer_params->add("region_types", results);

    // prepare request
    kladr_req_info.m_results = results;

    res = ThreadMain::get()->processKladrSql(
        sql,
        ObjectParamAddress::kladrSqlCb,
        kladr_req_info,
        sql_err_msg
    );
    if (SQLITE_OK != res){
        PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
        ThreadHttp::set500(a_answer);
        goto fail;
    }

    {
        string      answer;
        string      status = "success";
        string      descr  = "region types was fetched successfully";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }

out:
    return;
fail:
    goto out;
*/
}

// ---------------- module ----------------

string ObjectParamAddress::_getType()
{
    return "core.object.param.address";
}

string ObjectParamAddress::_getTypePerl()
{
    return "core::object::paramAddress::main";
}

int32_t ObjectParamAddress::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectParamAddress::_destroy()
{
}

ObjectSptr ObjectParamAddress::_object_create()
{
    ObjectSptr object;
    OBJECT_PARAM_ADDRESS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectParamAddress::_getType,
    .type_perl      = ObjectParamAddress::_getTypePerl,
    .init           = ObjectParamAddress::_init,
    .destroy        = ObjectParamAddress::_destroy,
    .object_create  = ObjectParamAddress::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

