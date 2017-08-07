#include "strings.hpp"
#include "network.hpp"
#include "thread/worker/http.hpp"

#include "core/neuron/param/address/main.hpp"

NeuronParamAddress::NeuronParamAddress()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronParamAddress::~NeuronParamAddress()
{
};

void NeuronParamAddress::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.param.address"));
    initName(PROP_STRING("address"));
    initLevels(PROP_VECTOR());

    // register API
    registerApiMethod(
        "getLevel",
        NeuronParamAddress::apiGetLevel
    );
    registerApiMethod(
        "getRegionTypes",
        NeuronParamAddress::apiGetRegionTypes
    );
}

int32_t NeuronParamAddress::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronParamAddress::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("levels", getLevels);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronParamAddress::parseBSON(
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
            PropVectorSptr levels = PROP_VECTOR();

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

                PropMapSptr map(new PropMap);

                map->add("name",         name);
                map->add("val",          val);
                map->add("type_name",    type_name);
                map->add("type_val",     type_val);
                levels->add(map);
            }
*/
            DbThread::bsonToProps(el.Obj(), levels);
            initLevels(levels);

        } else {
            err = -1;
            PERROR("Cannot parse: '%s', 'levels' is not array\n",
                a_obj.jsonString().c_str()
            );
        }
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronParamAddress::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t                 res, err = -1;
    char                    buffer[1024] = { 0x00 };
    PropMapSptr             answer_params;
    PropVectorSptr          results;
    PropMapSptr             req_params;
    KladrReqInfo            kladr_req_info;
    PropVectorSptr          prop_levels;
    PropVectorSptr          levels   = PROP_VECTOR();
    int32_t                 level_id = 0;

    answer_params   = a_answer->getCreateAnswerParams();
    results         = PROP_VECTOR();

    prop_levels = dynamic_pointer_cast<PropVector>(
        a_props->get("levels")
    );
    if (!prop_levels){
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
        PropVectorIt levels_it;
        for (levels_it = prop_levels->begin();
            levels_it != prop_levels->end();
            levels_it++, level_id++)
        {
            PropMapSptr prop_level;
            PropMapSptr level(new PropMap);
            PropSptr    name;
            PropSptr    val;
            PropSptr    type_name;
            PropSptr    type_val;

            // prop_level
            prop_level = dynamic_pointer_cast<PropMap>(
                *levels_it
            );
            if (!prop_level){
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
            name = prop_level->get("name");
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
            val = prop_level->get("val");
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
            type_name = prop_level->get("type_name");
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
            type_val = prop_level->get("type_val");
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
    setLevels(levels);

    // process upper props
    res = SUPER_CLASS::apiObjectUpdate(
        a_answer,
        a_props
    );
    if (res){
        PERROR("Cannot update props for object with ID: '%s'\n",
            getId()->toString().c_str()
        );
        err = res;
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

int32_t NeuronParamAddress::kladrSqlCb(
    void    *a_kladr_req_info,
    int32_t a_argc,
    char    **a_argv,
    char    **a_col_name)
{
    int32_t         i, err = 0;
    KladrReqInfo    *kladr_req_info = (KladrReqInfo *)a_kladr_req_info;
    PropVectorSptr  results         = kladr_req_info->m_results;
    PropMapSptr     fields          = PROP_MAP();

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
            PropVectorSptr      parts(new PropVector);
            PropVectorIt        parts_it;
            for (pos = 0; pos < tmp.size(); pos++){
                PropStringSptr cur_val(new PropString(tmp[pos]));
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

void NeuronParamAddress::apiGetLevel(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    int32_t         res;
    string          sql;
    string          sql_err_msg;
    PropMapSptr     answer_params   = a_answer->getCreateAnswerParams();
    PropVectorSptr  results         = PROP_VECTOR();
    KladrReqInfo    kladr_req_info;

    PropMapSptr     req_params;

    PropSptr        prop_code, prop_socr, prop_get_socr, prop_skip_socr;
    string          code, socr, get_socr = "1", skip_socr;

    // code: AABBBCCC00000
    // AA       - level 1
    // BBB      - level 2
    // CCC      - level 3
    // 00000    - level 4

    // get request params
    req_params = ThreadWorkerHttp::getParams(a_answer, a_req_props);
    if (req_params){
        prop_code       = req_params->get("code");
        prop_socr       = req_params->get("socr");
        prop_get_socr   = req_params->get("get_socr");
        prop_skip_socr  = req_params->get("skip_socr");
    }

    if (prop_code){
        code = prop_code->toString();
    }
    if (prop_socr){
        socr = prop_socr->toString();
    }
    if (prop_get_socr){
        get_socr = prop_get_socr->toString();
    }
    if (prop_skip_socr){
        if (Prop::PROP_TYPE_VECTOR
            == prop_skip_socr->getType())
        {
            PropVectorSptr      vector;
            PropVectorConstIt   vector_it;
            vector = dynamic_pointer_cast<PropVector>(
                prop_skip_socr
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

        res = a_neuron->getNetwork()->processKladrSql(
            sql,
            NeuronParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadWorkerHttp::set500(a_answer);
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

        res = a_neuron->getNetwork()->processKladrSql(
            sql,
            NeuronParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadWorkerHttp::set500(a_answer);
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

        res = a_neuron->getNetwork()->processKladrSql(
            sql,
            NeuronParamAddress::kladrSqlCb,
            kladr_req_info,
            sql_err_msg
        );
        if (SQLITE_OK != res){
            PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
            ThreadWorkerHttp::set500(a_answer);
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

out:
    return;
fail:
    goto out;
}

void NeuronParamAddress::apiGetRegionTypes(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    int32_t         res;
    string          sql;
    string          sql_err_msg;
    PropMapSptr     answer_params   = a_answer->getCreateAnswerParams();
    PropVectorSptr  results         = PROP_VECTOR();
    PropMapSptr     req_params;
    KladrReqInfo    kladr_req_info;
    PropSptr        prop_region;
    string          region;

    // get request params
    req_params = ThreadWorkerHttp::getParams(a_answer, a_req_props);
    if (req_params){
        prop_region = req_params->get("region");
    }
    if (!prop_region){
        string answer;
        string status = "failed";
        string descr  = "missing param 'region'";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);

        goto fail;
    }

    region = prop_region->toString();
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

    res = a_neuron->getNetwork()->processKladrSql(
        sql,
        NeuronParamAddress::kladrSqlCb,
        kladr_req_info,
        sql_err_msg
    );
    if (SQLITE_OK != res){
        PERROR("kladr SQL error: '%s'\n", sql_err_msg.c_str());
        ThreadWorkerHttp::set500(a_answer);
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
}

// --------------------------------------------------------------------------80

// ---------------- module ----------------

int32_t NeuronParamAddress::init()
{
    int32_t err = 0;
    return err;
}

void NeuronParamAddress::destroy()
{
}

NeuronSptr NeuronParamAddress::object_create()
{
    NeuronSptr neuron(new NeuronParamAddress);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.param.address",
    .init           = NeuronParamAddress::init,
    .destroy        = NeuronParamAddress::destroy,
    .object_create  = NeuronParamAddress::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

