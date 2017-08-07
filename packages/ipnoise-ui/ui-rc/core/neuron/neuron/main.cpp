#include "network.hpp"
#include "utils.hpp"
#include "base64.hpp"
#include "thread/http.hpp"
#include "template.hpp"

#include "prop/neuron/link/event.hpp"

#include "core/neuron/user/main.hpp"
#include "core/neuron/trash/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/neuron/main.hpp"

Neuron::Neuron()
#ifdef UI
    :   QGLWidget()
#else
    :   QObject()
#endif
{
    do_init();
};

Neuron::~Neuron()
{
};

void Neuron::do_init()
{
    m_is_new        = 1;
    m_is_dirty      = 0;
    m_viewer        = NULL;

    // init props
    initType(PROP_NEURON_TYPE("core.neuron.neuron"));
    initId(PROP_NEURON_ID(generateId()));
    // TODO use time here
    initRemoved(PROP_INT(0));
    initTimeAccess(PropTime::now());
    initName(PROP_STRING());
    initDescr(PROP_STRING());
    initTitle(PROP_STRING());
    initUrl(PROP_STRING(
        UPLOAD_DIR + getId()->toString()
    ));
    // initOwner(context->getUserId());
    initNeighsInfo(PROP_MAP());

    // position
    initPosX(PROP_DOUBLE(0.f));
    initPosY(PROP_DOUBLE(0.f));
    initPosZ(PROP_DOUBLE(0.f));

#ifdef HUMAN
    initHumanVal(PROP_DOUBLE());
    initHumanNervePath(PROP_STRING());
#endif

    // register API
    registerApiMethod(
        "load",
        Neuron::apiLoad
    );
    registerApiMethod(
        "delete",
        Neuron::apiDelete
    );
    registerApiMethod(
        "neighUnlink",
        Neuron::apiNeighUnlink
    );
    registerApiMethod(
        "render",
        Neuron::apiRender
    );
    registerApiMethod(
        "renderEmbedded",
        Neuron::apiRenderEmbedded
    );
    registerApiMethod(
        "getEvents",
        Neuron::apiGetEvents
    );
    registerApiMethod(
        "getNeighs",
        Neuron::apiGetNeighs
    );
    registerApiMethod(
        "bookmark",
        Neuron::apiBookmark
    );
    registerApiMethod(
        "updateEventsSubscr",
        Neuron::apiUpdateEventsSubscr
    );
}

int32_t Neuron::do_autorun()
{
    int32_t err = 0;
    return err;
}

void Neuron::keyPressEvent(
    QKeyEvent   *a_ev)
{
}

void Neuron::sendApiEventPropsChanged(
    PropMapSptr a_props)
{
    PropNeuronIdSptr                id = getId();
    PropMapIt                       props_it;
    vector<NeuronSptr>              neurons;
    vector<NeuronSptr>::iterator    neurons_it;

    PropMapSptr     event       = PROP_MAP();
    PropMapSptr     link_types  = PROP_MAP();

    (*(link_types.get()))[PROP_STRING("event")] = PropSptr();

    PDEBUG(50, "Neuron::sendApiEventPropsChanged:\n"
        " neuron ID:    '%s'\n"
        " props:        '%s'\n",
        id ? id->toString().c_str() : "",
        a_props->toString().c_str()
    );

    // fill event
    (*(event.get()))[PROP_STRING("object_id")]
        = getId();

    (*(event.get()))[PROP_STRING("type")]
        = PROP_STRING("props.changed");

    (*(event.get()))[PROP_STRING("props")]
        = a_props;

    // search sessions
    _getNeighs(
        neurons,
        "", // name
        0,  // get_removed
        PropNeuronTypeSptr(),
        link_types
    );
    for (neurons_it = neurons.begin();
        neurons_it != neurons.end();
        neurons_it++)
    {
        NeuronSptr          neuron = *neurons_it;
        NeuronSessionSptr   session;

        session = dynamic_pointer_cast<NeuronSession>(
            neuron
        );
        if (session){
            // add event
            session->addEvent(event);

            PWARN("send event:\n"
                "  src neuron ID:    '%s'\n"
                "  dst neuron ID:    '%s'\n"
                "  event:            '%s'\n",
                getId()->toString().c_str(),
                session->getId()->toString().c_str(),
                event->serialize().c_str()
            );
        }
    }
}

Network * Neuron::getNetwork()
{
    return g_network;
};

DbThread * Neuron::getDbThread()
{
    DbThread *thread = NULL;
    thread = g_network->getDbThread();
    return thread;
}

bool Neuron::isRemoved()
{
    int32_t ret = getRemoved()->toInt();
    return ret;
}

string Neuron::generateId() const
{
    char    buf[16] = { 0x00 };
    int32_t byte, i;
    string  id;

    for (i = 0; i < 20; i++){
        byte = int((double(rand())/RAND_MAX)*255);
        snprintf(buf, sizeof(buf), "%2.2x", byte);
        id += buf;
    }

    return id;
}

void Neuron::addToBookmarks()
{
    PropNeuronIdSptr    id = getId();
    ContextSptr         context;
    NeuronUserSptr      cur_user;

    cur_user = Context::getCurUser();

    // add neigh to user's bookmarks
    cur_user->addToBookmarks(this);
}

void Neuron::delFromBookmarks()
{
    PropNeuronIdSptr    id = getId();
    ContextSptr         context;
    NeuronUserSptr      cur_user;

    cur_user = Context::getCurUser();

    // add neigh to user's bookmarks
    cur_user->delFromBookmarks(this);
}

void Neuron::neighUnlink(
    PropNeuronIdSptr    a_id,
    const int32_t       &a_add_to_bookmarks)
{
    PropMapIt   it;
    NeuronSptr  neigh;
    int32_t     changed = 0;

    it = m_NeighsInfo->find(a_id);
    if (m_NeighsInfo->end() == it){
        // not exist
        goto out;
    }

    // unlink neuron
    m_NeighsInfo->erase(it);
    changed = 1;

    // go out if not need to add to bookmarks
    if (!a_add_to_bookmarks){
        goto out;
    }

    // search neigh
    neigh = getDbThread()->getNeuronById(a_id);
    if (!neigh){
        // not exist
        goto out;
    }

    // add to user's bookmarks
    neigh->addToBookmarks();

out:
    if (changed){
        save();
    }
    return;
}

void Neuron::neighDelete(
    Neuron  *a_neigh)
{
    PropMapIt   it;
    NeuronSptr  neuron;
    string      neigh_id;

    // unlink from us
    neighUnlink(a_neigh->getId());
    save();

    // unlink us
    a_neigh->neighUnlink(getId());
    a_neigh->save();
}

void Neuron::neighDelete(
    PropNeuronIdSptr    a_id)
{
    PropMapIt   it;
    NeuronSptr  neuron;
    string      neigh_id;

    neigh_id = a_id->toString();
    if (!neigh_id.size()){
        PERROR("empty neigh ID (cur neuron ID: '%s')\n",
            getId()->toString().c_str()
        );
        goto fail;
    }

    // unlink from us
    neighUnlink(a_id);
    save();

    // unlink us
    neuron = getDbThread()->getNeuronById(a_id);
    if (neuron){
        neuron->neighUnlink(getId());
        neuron->save();
    }

out:
    return;
fail:
    goto out;
}

void Neuron::getAllProps(
    PropMapSptr a_props)
{
    string id = getId()->toString();

    // declarate props for save
    SAVE_PROP("_id",                getId);
    SAVE_PROP("removed",            getRemoved);
    SAVE_PROP("type",               getType);
    SAVE_PROP("name",               getName);
    SAVE_PROP("descr",              getDescr);
    SAVE_PROP("title",              getTitle);
    SAVE_PROP("url",                getUrl);
    SAVE_PROP("neighs",             getNeighsInfo);
    SAVE_PROP("pos_x",              getPosX);
    SAVE_PROP("pos_y",              getPosY);
    SAVE_PROP("pos_z",              getPosZ);
#ifdef HUMAN
    SAVE_PROP("human_val",          getHumanVal);
    SAVE_PROP("human_nerve_path",   getHumanNervePath);
#endif

    // if ID is empty, remove it
    if (!id.size()){
        a_props->erase(PROP_STRING("_id"));
    }
}

UiViewer * Neuron::getViewer()
{
    return m_viewer;
}

void Neuron::setViewer(
    UiViewer *a_viewer)
{
    m_viewer = a_viewer;
}

const QGLContext * Neuron::getViewerContext()
{
    const QGLContext *ret = NULL;
    if (m_viewer){
        ret = m_viewer->context();
    }
    return ret;
}

void Neuron::setDirty(
    const int32_t &a_val)
{
    PropMapSptr props = PROP_MAP();
    PropMapIt   it;

    getAllProps(props);

    for (it = props->begin();
        it != props->end();
        it++)
    {
        PropSptr prop_name  = it->first;
        PropSptr prop_val   = it->second;

        prop_name->setDirty(a_val);
        prop_val->setDirty(a_val);
    }
}

int32_t Neuron::save()
{
    int32_t     err         = -1;
    PropMapSptr props       = PROP_MAP();
    PropMapSptr props_dirty = PROP_MAP();
    PropMapIt   it;

    PWARN("save neuron: '%s'\n",
        getId()->toString().c_str()
    );

    getAllProps(props);
    for (it = props->begin();
        it != props->end();
        it++)
    {
        PropSptr prop_name  = it->first;
        PropSptr prop_val   = it->second;

        if (    prop_name->isDirty()
            ||  prop_val->isDirty())
        {
            (*(props_dirty.get()))[prop_name] = prop_val;
        }
    }

    if (!props_dirty->empty()){
        // save props
        err = saveProps(props);
        if (err){
            goto fail;
        }
    }

    // save params
    err = saveParams();
    if (err){
        goto fail;
    }

    // mark as not dirty now
    setDirty(0);

    // submit event
    sendApiEventPropsChanged(props_dirty);

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

int32_t Neuron::saveProps(
    PropMapSptr   a_props)
{
    int32_t     err         = 0;
    DbThread    *db_thread  = NULL;
    db_thread = g_network->getDbThread();
    err       = db_thread->save(this, a_props);
    return err;
}

int32_t Neuron::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t     err     = 0;
    int32_t     removed = 0;
    string      id;
    string      name;
    string      descr;
    string      title;
    string      initle;
    string      url;
    double      pos_x       = 0.f;
    double      pos_y       = 0.f;
    double      pos_z       = 0.f;
#ifdef HUMAN
    double      human_val   = 0.f;
    string      human_nerve_path;
#endif
    string      type;

    //  Object example:
    //
    //  {
    //      '_id':        'tigra',
    //      'type':       2,
    //      'descr':      ''.
    //      'neighs':     {
    //          'c1ca823b7d2f4723a6db9b19e413f131e2bbf2b9': {
    //              'link_creator':   2,
    //              'walk_count':     {
    //                  'morik': 1
    //              }
    //          }
    //      },
    //  }

    // ID
    if (a_obj.hasField("_id")){
        mongo::BSONElement  el;
        a_obj.getObjectID(el);
        el.Val(id);
        initId(PROP_NEURON_ID(id));
    }

    // Type
    if (a_obj.hasField("type")){
        type = a_obj.getStringField("type");
        initType(PROP_NEURON_TYPE(type));
    }

    if (    !id.size()
        ||  !type.size())
    {
        goto fail;
    }

    // Removed
    if (a_obj.hasField("removed")){
        removed = a_obj.getIntField("removed");
        initRemoved(PropIntSptr(
            new PropInt(removed)
        ));
    }

    // Name
    if (a_obj.hasField("name")){
        name = a_obj.getStringField("name");
        initName(PROP_STRING(name));
    }

    // Descr
    if (a_obj.hasField("descr")){
        descr = a_obj.getStringField("descr");
        initDescr(PROP_STRING(descr));
    }

    // Title
    if (a_obj.hasField("title")){
        title = a_obj.getStringField("title");
        initTitle(PROP_STRING(title));
    }

    // Url
    if (a_obj.hasField("url")){
        url = a_obj.getStringField("url");
        initUrl(PROP_STRING(url));
    }

    // Neighs
    if (a_obj.hasField("neighs")){
        PropMapSptr                             neighs_map(new PropMap);
        list< mongo::BSONElement >              els;
        list< mongo::BSONElement >::iterator    els_it;
        mongo::BSONElement                      el;
        mongo::BSONObj                          neighs_obj;

        el = a_obj.getField("neighs");
        if (mongo::BSONType::Object != el.type()){
            PERROR("cannot parse 'neighs' attribute,"
                " object dump: '%s'\n",
                a_obj.toString().c_str()
            );
            goto fail;
        }

        el.Val(neighs_obj);
        neighs_obj.elems(els);
        for (els_it = els.begin();
            els_it != els.end();
            els_it++)
        {
            // for every neigh
            string          neigh_id    = els_it->fieldName();
            mongo::BSONType type        = els_it->type();
            mongo::BSONObj  neigh_obj;
            PropMapSptr     neigh_map(new PropMap);

            list< mongo::BSONElement >              els2;
            list< mongo::BSONElement >::iterator    els2_it;
            mongo::BSONElement                      el2;
            mongo::BSONObj                          links_obj;

            els_it->Val(neighs_obj);

            // check what neigh is object
            if (mongo::BSONType::Object != type){
                PERROR("cannot parse 'neighs' attribute,"
                    " object dump: '%s'\n",
                    neighs_obj.toString().c_str()
                );
                goto fail;
            }

            // create prop - neigh_id, it will be 'map' too
            neighs_map->add(
                PROP_STRING(neigh_id),
                neigh_map
            );
/*
            el2 = neighs_obj.getField(neigh_id);
            if (mongo::BSONType::Object != el2.type()){
                PERROR("cannot parse neigh ID: '%s'"
                    " attribute, el type: '%d', "
                    " object dump: '%s'\n",
                    neigh_id.c_str(),
                    el2.type(),
                    a_obj.toString().c_str()
                );
                goto fail;
            }
*/

            // get "type -> link" object
//            el2.Val(links_obj);
            neighs_obj.elems(els2);

            for (els2_it = els2.begin();
                els2_it != els2.end();
                els2_it++)
            {
                // for every neigh
                string              link_type = els2_it->fieldName();
                mongo::BSONType     type      = els2_it->type();
                mongo::BSONObj      link_obj;
                PropNeuronLinkSptr  link;

                // check what neigh is object
                if (mongo::BSONType::Object != type){
                    PERROR("cannot parse 'link type' value,"
                        " object dump: '%s'\n",
                        a_obj.toString().c_str()
                    );
                    goto fail;
                }

                // get link object
                els2_it->Val(link_obj);

                // link_type
                if ("neuron" == link_type){
                    link = PROP_NEURON_LINK();
                } else if ("event" == link_type){
                    link = PROP_NEURON_LINK_EVENT();
                } else {
                    PFATAL("unsupported neuron link type: '%s'\n",
                        link_type.c_str()
                    );
                }

                // link_creator
                if (link_obj.hasField("link_creator")){
                    string link_creator = link_obj.getStringField("link_creator");
                    link->setLinkCreator(link_creator);
                }

                // walk_count
                if (neigh_obj.hasField("walkers")){
                    PropMapSptr                           walkers_map(new PropMap);
                    list< mongo::BSONElement >            els;
                    list< mongo::BSONElement >::iterator  els_it;
                    mongo::BSONElement                    el;
                    mongo::BSONObj                        walkers_obj;

                    el = neigh_obj.getField("walkers");
                    if (mongo::BSONType::Object != el.type()){
                        PERROR("cannot parse 'walkers' attribute,"
                            " object dump: '%s'\n",
                            a_obj.toString().c_str()
                        );
                        goto fail;
                    }

                    el.Val(walkers_obj);
                    walkers_obj.elems(els);
                    for (els_it = els.begin();
                        els_it != els.end();
                        els_it++)
                    {
                        // for every walker
                        string          walker_id   = els_it->fieldName();
                        mongo::BSONType type        = els_it->type();
                        mongo::BSONObj  walker_obj;
                        PropMapSptr     walker_map(new PropMap);

                        // check what walker is object
                        if (mongo::BSONType::Object != type){
                            PERROR("cannot parse 'walkers' attribute,"
                                " object dump: '%s'\n",
                                a_obj.toString().c_str()
                            );
                            goto fail;
                        }

                        // create prop - walkers_id, it will be 'map' too
                        walkers_map->add(
                            PROP_STRING(walker_id),
                            walker_map
                        );

                        // get walker object
                        els_it->Val(walker_obj);

                        // walk_count
                        if (walker_obj.hasField("walk_count")){
                            int32_t walk_count = 0;
                            walk_count = walker_obj.getIntField("walk_count");
                            walker_map->add(
                                PROP_STRING("walk_count"),
                                PROP_INT(walk_count)
                            );
                        }
                    }
                    // add to link
                    link->setWalkers(walkers_map);
                }

                // create prop - link_type, it will be 'map' too
                neigh_map->add(
                    PROP_STRING(link_type),
                    link
                );
            }
        }

        initNeighsInfo(neighs_map);
    }

    // Position
    {
        if (a_obj.hasField("pos_x")){
            pos_x = a_obj.getField("pos_x").Double();
            initPosX(PROP_DOUBLE(pos_x));
        }
        if (a_obj.hasField("pos_y")){
            pos_y = a_obj.getField("pos_y").Double();
            initPosY(PROP_DOUBLE(pos_y));
        }
        if (a_obj.hasField("pos_z")){
            pos_z = a_obj.getField("pos_z").Double();
            initPosZ(PROP_DOUBLE(pos_z));
        }
    }

#ifdef HUMAN
    // human val
    if (a_obj.hasField("human_val")){
        human_val = a_obj.getField("human_val").Double();
        initHumanVal(PROP_DOUBLE(human_val));
    }
    // human nerve path
    if (a_obj.hasField("human_nerve_path")){
        human_nerve_path = a_obj.getStringField("human_nerve_path");
        initHumanNervePath(PROP_STRING(
            human_nerve_path
        ));
    }
#endif

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void Neuron::serializeToPropMap(
    PropMapSptr     a_out,
    const int32_t   &a_serialize_params)
{
    PropMapSptr     props(new PropMap);
    PropMapIt       props_it;
    NParamsSptr     params(new NParams);
    NParamsIt       params_it;
    string          ret;
    string          data_props;
    string          data_params;

    if (!a_out){
        PFATAL("missing argument a_out\n");
    }

    // get props
    getAllProps(props);
    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        a_out->add(props_it->first, props_it->second);
    }

    // get params (dynamic props)
    getParams(params);
    for (params_it = params->begin();
        params_it != params->end();
        params_it++)
    {
        PropMapSptr props(new PropMap);
        params_it->second->getAllProps(props);
        a_out->add(params_it->first, props);
    }
}

string Neuron::serialize(
    const string    &a_delim,
    const int32_t   &a_serialize_params)
{
    string          ret;
    PropMapSptr     obj = PROP_MAP();

    // get object info
    serializeToPropMap(obj, a_serialize_params);

    // serialize it
    ret = obj->serialize(a_delim);

    return ret;
}

string Neuron::toString()
{
    string ret;
    ret = serialize();
    return ret;
}

string Neuron::getView(
    const PropMapSptr a_params)
{
    PropSptr    prop_view;
    string      view = "widget";

    // get template view
    if (a_params){
        prop_view = a_params->get("view");
        if (prop_view){
            view = prop_view->toString();
        }
    }

    return view;
}

TemplateSptr Neuron::getTemplate(
    const PropMapSptr a_params)
{
    PropSptr        prop_webgl;
    TemplateSptr    tpl;

    tpl = TemplateSptr(new Template(this));

    // process webgl
    if (a_params){
        prop_webgl = a_params->get("webgl");
        if (prop_webgl){
            tpl->setPrefix("webgl");
        }
    }

    return tpl;
}

void Neuron::render(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_params)
{
    string              html;
    PropSptr            prop_view;
    TemplateSptr        tpl     = getTemplate(a_params);
    string              view    = getView(a_params);
    PropNeuronIdSptr    id      = getId();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    PWARN("render: '%s'\n",
        a_params->toString().c_str()
    );

    // process template
    html = tpl->render(id, view);
    a_answer->setAnswerBody(html);
}

void Neuron::renderEmbedded(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_params)
{
    string              html;
    PropSptr            prop_view;
    TemplateSptr        tpl     = getTemplate(a_params);
    string              view    = getView(a_params);
    PropNeuronIdSptr    id      = getId();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");

    PWARN("renderEmbedded: '%s'\n",
        a_params->toString().c_str()
    );

    // process template
    html = tpl->renderEmbedded(id, view);
    a_answer->setAnswerBody(html);
}

void Neuron::renderOpenGl(
    const int32_t   &a_with_name,
    const int32_t   &a_selected)
{
    PropDoubleSptr  prop_ax;
    PropDoubleSptr  prop_ay;
    PropDoubleSptr  prop_az;
    PropDoubleSptr  prop_cur_val;

    double  ax      = 0;
    double  ay      = 0;
    double  az      = 0;
    double  val     = 0;
    double  color_r = 0;
    double  color_g = 0;
    double  color_b = 0;

    prop_ax      = getPosX();
    prop_ay      = getPosY();
    prop_az      = getPosZ();
    prop_cur_val = getHumanVal();

    ax  = *prop_ax.get();
    ay  = *prop_ay.get();
    az  = *prop_az.get();
    val = *prop_cur_val.get();

    // set default color
    color_r = val;

    if (a_selected){
        color_r = 0x00;
        color_g = 0xff;
        color_b = 0x00;
    }

    if (a_with_name){
        // add name
        glPushName(a_with_name);
    }

    {
        Frame             m_frame;
        static GLUquadric *quad = gluNewQuadric();
        m_frame.setPosition(
            Vec(ax, ay, az)
        );

        glPushMatrix();
        glMultMatrixd(m_frame.matrix());
        glColor3f(color_r, color_g, color_b);
        gluSphere(quad, UI_VIWER_NEURON_SPHERE_SIZE, 10, 6);
        glPopMatrix();
    }

    if (a_with_name){
        // remove name
        glPopName();
    }
}

int32_t Neuron::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     err = -1;

    PropSptr    prop_name;
    PropSptr    prop_descr;
    PropSptr    prop_title;
    PropSptr    prop_url;

    // search name
    prop_name = a_props->get("name");
    if (prop_name){
        setName(PROP_STRING(
            prop_name->toString()
        ));
    }

    // search descr
    prop_descr = a_props->get("descr");
    if (prop_descr){
        setDescr(PROP_STRING(
            prop_descr->toString()
        ));
    }

    // search title
    prop_title = a_props->get("title");
    if (prop_title){
        setTitle(PROP_STRING(
            prop_title->toString()
        ));
    }

    // search url
    prop_url = a_props->get("url");
    if (prop_url){
        setUrl(PROP_STRING(
            prop_url->toString()
        ));
    }

    // try to save
    err = save();

    return err;
}

int32_t Neuron::apiObjectCreate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
//    int32_t             res;
    int32_t             err = 0;
    PropSptr            prop_type;
    PropNeuronTypeSptr  prop_neuron_type;
    string              neuron_type;
    NeuronSptr          new_neuron;
    PropMapSptr         answer_params;
    PropMapSptr         redirect(new PropMap);
    string              object_id;

    // TODO
    // check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    // search type
    prop_type = a_props->get("type");
    if (!prop_type){
        goto out;
    }

    // get neuron type
    neuron_type = prop_type->toString();

    // check what type is valid
    //prop_neuron_type = PROP_NEURON_TYPE();
    //res = prop_neuron_type->isTypeValid(neuron_type);
    //if (!res){
    //    a_answer->setAnswerStatusCode(200);
    //    a_answer->setAnswerStatusString("OK");
    //    a_answer->setAnswerApiStatus("failed");
    //    a_answer->setAnswerApiDescr(
    //        "invalid object type"
    //    );
    //    goto fail;
    //}

    // create new neuron
    new_neuron = getNetwork()->createNeuron(neuron_type);
    if (!new_neuron){
        a_answer->setAnswerStatusCode(500);
        a_answer->setAnswerStatusString("Internal error");
        a_answer->setAnswerBody("");
        goto fail;
    }
    LINK_AND_SAVE(this, new_neuron.get());

    // get object ID
    object_id = new_neuron->getId()->toString();

    // create redirect info
    redirect->add("object_id",        object_id);
    answer_params->add("object_id",   object_id);
    answer_params->add("redirect",    redirect);

    // create answer
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "new object was created successfully"
    );

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

void Neuron::remove()
{
    // TODO link with current user's trash
    // 20140609 morik@

    //NeuronTrashSptr trash;
    //trash = getDbThread()->getCreateTrashNeuron();

    // mark as deleted
    // TODO use time of delete here (instead 1)
    setRemoved(PROP_INT(1));
    save();
    //LINK_AND_SAVE(this, trash.get());
}

void Neuron::apiLoad(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    PropMapSptr answer_params;

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    // get object info
    a_neuron->serializeToPropMap(
        answer_params,  // object
        1               // serialize params
    );

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was loaded successfully"
    );
}

void Neuron::apiDelete(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    // TODO check perm here
    a_neuron->remove();

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was deleted successfully"
    );
}

void Neuron::apiBookmark(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr)
{
    NeuronSessionSptr   session;
    ContextSptr         context;
    NeuronUserSptr      user;

    user = Context::getCurUser();
    if (!user){
        goto no_auth;
    }

    // all ok, link
    LINK_AND_SAVE(user.get(), a_neuron);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "there are no errors"
    );

out:
    return;
no_auth:
    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("failed");
    a_answer->setAnswerApiDescr(
        "you are not authorized"
    );
    goto out;
}

void Neuron::apiGetEvents(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   ) //a_req_props)
{
    // TODO check perms here

    // donot disconnect
    a_answer->needDisconnect(0);
}

void Neuron::apiGetNeighs(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    PropVectorSptr                  neighs_ids = PROP_VECTOR();
    PropMapSptr                     params;
    PropMapSptr                     answer_params;
    PropSptr                        prop_type;
    string                          type;
    PropNeuronTypeSptr              neuron_type;
    vector<NeuronSptr>              neighs;
    vector<NeuronSptr>::iterator    neighs_it;

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_type = params->get("type");
    }
    if (prop_type){
        type = prop_type->toString();
    }
    if (!type.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"missing or empty param 'type'\""
            "}"
        );
        goto out;
    }

    a_neuron->getNeighs(
        neighs,
        "",                     // name
        0,                      // get_removed
        PROP_NEURON_TYPE(type)
    );

    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        NeuronSptr       cur_neigh = *neighs_it;
        PropNeuronIdSptr id        = cur_neigh->getId();
        neighs_ids->push_back(id);
    }

    answer_params->add("neighs", neighs_ids);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr("all ok");

out:
    return;
}

void Neuron::apiNeighUnlink(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    PropSptr    prop_neigh_id;
    string      neigh_id;
    string      redirect;
    string      object_id;
    PropMapSptr params;

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_neigh_id = params->get("neigh_id");
    }
    if (prop_neigh_id){
        neigh_id = prop_neigh_id->toString();
    }
    if (!neigh_id.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"missing or empty param 'neigh_id'\""
            "}"
        );
        goto out;
    }
    a_neuron->neighUnlink(
        PROP_NEURON_ID(neigh_id)
    );

    object_id = a_neuron->getId()->serialize();

    redirect += "\"redirect\":";
    redirect += " {";
    redirect +=     " \"object_id\": " + object_id;
    redirect += " }";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerBody(
        "{"
            "\"status\": \"success\","
            " \"descr\": \"neigh was unlinked successfully\","
            " \"params\": {"
               "\"neigh_id\":" + prop_neigh_id->serialize() + ","
                + redirect
            + "}"
        "}"
    );

out:
    return;
}

void Neuron::apiRender(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    string      answer, html;
    PropMapSptr params;

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
        if (!params){
            params = PROP_MAP();
        }
    }

    a_neuron->render(
        a_answer,
        params
    );

    html = base64_encode(a_answer->getAnswerBody());

    answer += "{";
    answer +=   "\"status\": \"success\",";
    answer +=   "\"descr\": \"all ok\",";
    answer +=   "\"params\": {";
    answer +=       " \"html\": \"" + html + "\"";
    answer +=   "}";
    answer += "}";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerBody(answer);
}

void Neuron::apiRenderEmbedded(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    string      answer, html;
    PropMapSptr params;

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
        if (!params){
            params = PROP_MAP();
        }
    }

    a_neuron->renderEmbedded(
        a_answer,
        params
    );

    html = base64_encode(a_answer->getAnswerBody());

    answer += "{";
    answer +=   "\"status\": \"success\",";
    answer +=   "\"descr\": \"all ok\",";
    answer +=   "\"params\": {";
    answer +=       " \"html\": \"" + html + "\"";
    answer +=   "}";
    answer += "}";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerBody(answer);
}

void Neuron::apiUpdateEventsSubscr(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    string              answer;
    PropSptr            tmp;
    PropMapSptr         params;
    PropMapSptr         events;
    PropMapIt           events_it;
    string              status;
    string              descr;
    ContextSptr         context;
    NeuronSessionSptr   session;

    a_neuron->lock();

    // get context
    context = ThreadHttp::getContext();
    // get session
    session = context->getSession();

    tmp     = a_req_props->get("params");
    params  = dynamic_pointer_cast<PropMap>(tmp);
    if (!params){
        status = "failed";
        descr  = "misisng argument 'params'";
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
        goto failed;
    }

    tmp = params->get("events");
    events = dynamic_pointer_cast<PropMap>(tmp);
    if (!events){
        status = "failed";
        descr  = "misisng argument 'events'";
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
        goto failed;
    }

    for (events_it = events->begin();
        events_it != events->end();
        events_it++)
    {
        PropStringSptr          event_name;
        PropIntSptr             event_subscr;
        PropNeuronLinkEventSptr link = PROP_NEURON_LINK_EVENT();

        // get event name
        event_name = dynamic_pointer_cast<PropString>(
            events_it->first
        );
        if (!event_name){
            // TODO inform client about it
            PWARN("invalid event name: '%s'\n",
                events_it->first->serialize().c_str()
            );
            continue;
        }

        // get event subscr
        event_subscr = dynamic_pointer_cast<PropInt>(
            events_it->second
        );
        if (!event_subscr){
            // TODO inform client about it
            PWARN("invalid event subscr: '%s'\n",
                events_it->second->serialize().c_str()
            );
            continue;
        }

        LINK_AND_SAVE3(a_neuron, session, link);
    }

    answer += "{";
    answer +=   "\"status\": \"success\",";
    answer +=   " \"descr\": \"all ok\",";
    answer +=   " \"params\": {";
    answer +=   "}";
    answer += "}";

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerBody(answer);

out:
    a_neuron->unlock();
    return;
failed:
    goto out;
}

void Neuron::registerApiMethod(
    const string    &a_name,
    void            (*a_cb)(
        Neuron *,
        HttpAnswerSptr,
        const PropMapSptr
    ))
{
    m_handlers[a_name] = a_cb;
}

void Neuron::api(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    string          method;
    PropSptr        prop_method;
    ContextSptr     context;
    ApiHandlersIt   it;

    void (*handler)(
        Neuron *,
        HttpAnswerSptr,
        const PropMapSptr
    ) = NULL;

    // setup default values
    a_answer->setAnswerStatusCode(500);
    a_answer->setAnswerStatusString("Internal error");

    // get prop 'method'
    prop_method = a_req_props->get(
        PROP_STRING("method")
    );
    if (prop_method){
        method = prop_method->toString();
    }

    // search context
    context = ThreadHttp::getContext();

    // search handler
    it = m_handlers.find(method);
    if (m_handlers.end() != it){
        handler = it->second;
    }

    // process method
    if (handler){
        (*handler)(
            this,
            a_answer,
            a_req_props
        );
    } else {
        string answer;
        string status = "failed";
        string descr  = "unsupported method";

        PERROR("Unsupported method: '%s' for neuron ID: '%s'\n",
            method.c_str(),
            getId()->toString().c_str()
        );

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }
}

void Neuron::getParams(
    NParamsSptr a_out)
{
    vector<NeuronParamSptr>                 res;
    vector<NeuronParamSptr>::const_iterator res_it;

    getNeighs<NeuronParam>(
        res,
        "",
        0,
        PROP_NEURON_TYPE("core.neuron.param")
    );

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NeuronParamSptr cur_param = *res_it;
        PropSptr        name      = cur_param->getName();
        if (name){
            (*a_out.get())[name->toString()] = cur_param;
        } else {
            PWARN("empty param property name, for neuron"
                " with ID: '%s'\n",
                cur_param->getId()->toString().c_str()
            );
        }
    }
}

int32_t Neuron::saveParams()
{
    int32_t         res, err = 0;
    NParamsSptr     params(new NParams);
    NParamsConstIt  params_it;

    // request all params
    getParams(params);

    for (params_it = params->begin();
        params_it != params->end();
        params_it++)
    {
        res = params_it->second->save();
        if (res){
            err = res;
        }
    }

    return err;
}

void Neuron::beforeRequest(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    ContextSptr         context;
    NeuronUserSptr      cur_user;

    // get current user
    cur_user = Context::getCurUser();

    // create shop neuron
    cur_user->getCreateShopNeuron();
}

#ifdef HUMAN
void Neuron::setHumanVal(
    const double &a_val)
{
    setHumanVal(PROP_DOUBLE(a_val));
}

void Neuron::setPosX(
    const double &a_val)
{
    setPosX(PROP_DOUBLE(a_val));
}

void Neuron::setPosY(
    const double &a_val)
{
    setPosY(PROP_DOUBLE(a_val));
}

void Neuron::setPosZ(
    const double &a_val)
{
    setPosZ(PROP_DOUBLE(a_val));
}

void Neuron::setPos(
    const double &a_x,
    const double &a_y,
    const double &a_z)
{
    setPosX(PROP_DOUBLE(a_x));
    setPosY(PROP_DOUBLE(a_y));
    setPosZ(PROP_DOUBLE(a_z));
}

#endif

// ---------------- module ----------------

int32_t Neuron::init()
{
    int32_t err = 0;
    return err;
}

void Neuron::destroy()
{
}

NeuronSptr Neuron::object_create()
{
    NeuronSptr neuron(new Neuron);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.neuron",
    .init           = Neuron::init,
    .destroy        = Neuron::destroy,
    .object_create  = Neuron::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

