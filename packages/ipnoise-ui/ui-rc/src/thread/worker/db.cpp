#include "network.hpp"

#include "core/neuron/neuron/main.hpp"

#include "thread/db.hpp"

mutex           g_db_mutex;
extern Network  *g_network;

DbThread::DbThread(
    Network         *a_net,
    const string    &a_name)
    :   Thread(a_net, a_name)
{
}

DbThread::~DbThread()
{
}

int32_t DbThread::do_init()
{
    PDEBUG(30, "DbThread::do_init\n");

    int32_t err = -1;

    try {
        m_mongo_db.connect("127.0.0.1");
    } catch (const mongo::DBException &e){
        PERROR("[ connect ] mongo init failed (%s)\n",
            e.what()
        );
    } catch (...){
        PERROR("[ connect ] mongo init failed\n");
        goto fail;
    }

    // all ok;
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void DbThread::do_start()
{
    while (Thread::THREAD_STATE_RUNNING == getState()){
        PDEBUG(25, "'%s' thread tick\n",
            getName().c_str()
        );
        sleep(1);
    }
}

void DbThread::do_stop()
{
    PDEBUG(25, "'%s' thread do_stop\n",
        getName().c_str()
    );
}

void DbThread::bsonToProps(
    mongo::BSONObj  a_obj,
    PropSptr        a_props)
{
    list< mongo::BSONElement >           els;
    list< mongo::BSONElement >::iterator els_it;

    Prop::PropType a_props_type = a_props->getType();

    a_obj.elems(els);

    for (els_it = els.begin();
        els_it != els.end();
        els_it++)
    {
        string          key     = els_it->fieldName();
        mongo::BSONType type    = els_it->type();
        PropSptr        key_prop(new PropString(key));

        if (mongo::BSONType::String == type){
            string val;
            els_it->Val(val);

            switch (a_props_type){
                case Prop::PROP_TYPE_VECTOR:
                    {
                        PropVectorSptr props;
                        props = dynamic_pointer_cast<PropVector>(
                            a_props
                        );
                        props->add(PROP_STRING(val));
                    }
                    break;

                case Prop::PROP_TYPE_MAP:
                    {
                        PropMapSptr props;
                        props = dynamic_pointer_cast<PropMap>(
                            a_props
                        );
                        props->add(
                            key_prop,
                            PROP_STRING(val)
                        );
                    }
                    break;

                default:
                    PERROR("Unsupported a_props type: '%d'\n", type);
                    break;
            }
        } else if (mongo::BSONType::NumberInt == type){
            int32_t val = els_it->Int();

            switch (a_props_type){
                case Prop::PROP_TYPE_VECTOR:
                    {
                        PropVectorSptr props;
                        props = dynamic_pointer_cast<PropVector>(
                            a_props
                        );
                        props->add(PROP_INT(val));
                    }
                    break;

                case Prop::PROP_TYPE_MAP:
                    {
                        PropMapSptr props;
                        props = dynamic_pointer_cast<PropMap>(
                            a_props
                        );
                        props->add(
                            key_prop,
                            PropIntSptr(
                                new PropInt(val)
                            )
                        );
                    }
                    break;

                default:
                    PERROR("Unsupported a_props type: '%d'\n",
                        type
                    );
                    break;
            }
        } else if (mongo::BSONType::NumberDouble == type){
            double val = els_it->Double();

            switch (a_props_type){
                case Prop::PROP_TYPE_VECTOR:
                    {
                        PropVectorSptr props;
                        props = dynamic_pointer_cast<PropVector>(
                            a_props
                        );
                        props->add(PROP_DOUBLE(val));
                    }
                    break;

                case Prop::PROP_TYPE_MAP:
                    {
                        PropMapSptr props;
                        props = dynamic_pointer_cast<PropMap>(
                            a_props
                        );
                        props->add(
                            key_prop,
                            PropDoubleSptr(
                                new PropDouble(val)
                            )
                        );
                    }
                    break;

                default:
                    PERROR("Unsupported a_props type: '%d'\n",
                        type
                    );
                    break;
            }
        } else if (mongo::BSONType::Object == type){
            mongo::BSONObj val;
            els_it->Val(val);

            switch (a_props_type){
                case Prop::PROP_TYPE_VECTOR:
                    {
                        PropVectorSptr props;
                        props = dynamic_pointer_cast<PropVector>(
                            a_props
                        );
                        PropMapSptr new_object(new PropMap);
                        props->add(new_object);
                        bsonToProps(val, new_object);
                    }
                    break;

                case Prop::PROP_TYPE_MAP:
                    {
                        PropMapSptr props;
                        props = dynamic_pointer_cast<PropMap>(
                            a_props
                        );
                        PropMapSptr new_object(new PropMap);
                        props->add(key_prop, new_object);
                        bsonToProps(val, new_object);
                    }
                    break;

                default:
                    PERROR("Unsupported a_props type: '%d'\n", a_props_type);
                    break;
            }
        } else if (mongo::BSONType::Array == type){
            mongo::BSONObj val;
            els_it->Val(val);

            switch (a_props_type){
                case Prop::PROP_TYPE_VECTOR:
                    {
                        PropVectorSptr props;
                        props = dynamic_pointer_cast<PropVector>(
                            a_props
                        );
                        PropVectorSptr new_object(new PropVector);
                        props->add(new_object);
                        bsonToProps(val, new_object);
                    }
                    break;

                case Prop::PROP_TYPE_MAP:
                    {
                        PropMapSptr props;
                        props = dynamic_pointer_cast<PropMap>(
                            a_props
                        );
                        PropVectorSptr new_object(new PropVector);
                        props->add(key_prop, new_object);
                        bsonToProps(val, new_object);
                    }
                    break;

                default:
                    PERROR("Unsupported a_props type: '%d'\n",
                        a_props_type
                    );
                    break;
            }
        } else {
            PERROR("unsupported element type: '%d',"
                " dump: '%s'\n",
                els_it->type(),
                els_it->toString().c_str()
            );
        }
    }
}

NeuronSptr DbThread::getCreateNeuronById(
    const PropNeuronIdSptr      a_id,
    const PropNeuronTypeSptr    a_type,
    const int32_t               a_get_removed)
{
    NeuronSptr neuron;
    neuron = getNeuronById(
        a_id,
        a_type,
        a_get_removed
    );
    if (!neuron){
        neuron = g_network->createNeuron();
        neuron->initId(a_id);
    }
    return neuron;
}

NeuronSptr DbThread::_getNeuronById(
    const PropNeuronIdSptr      a_id,
    const PropNeuronTypeSptr    a_type,
    const int32_t               a_get_removed,
    const int32_t               a_do_autorun)
{
    NeuronSptr  neuron;

    // search in cache
    if (!neuron){
        neuron = getNeuronByIdFromCache(
            a_id,
            a_type,
            a_get_removed
        );
    }

    PDEBUG(20, "search in cache: ID: '%s' %s\n",
        a_id->toString().c_str(),
        neuron ? "FOUND" : "NOT FOUND"
    );

    // search in db
    if (!neuron){
        neuron = getNeuronByIdFromDb(
            a_id,
            a_type,
            a_get_removed,
            a_do_autorun
        );
    }

    PDEBUG(20, "search in DB: ID: '%s' %s\n",
        a_id->toString().c_str(),
        neuron ? "FOUND" : "NOT FOUND"
    );

    if (    neuron
        &&  neuron->isRemoved()
        &&  !a_get_removed)
    {
        neuron = NeuronSptr();
    }

    if (!neuron){
        PDEBUG(20, "cannot get neuron by ID: '%s',"
            " a_get_removed: '%d'\n",
            a_id->toString().c_str(),
            a_get_removed
        );
    }

    return neuron;
}

void DbThread::db_lock()
{
    Thread *thread = (Thread *)QThread::currentThread();

#ifdef __x86_64
    PDEBUG(20, "thread: 0x%lx, try to lock mongo DB\n",
        (uint64_t)thread
    );
#else
    PDEBUG(20, "thread: 0x%x, try to lock mongo DB\n",
        (uint32_t)thread
    );
#endif

    g_db_mutex.lock();

#ifdef __x86_64
    PDEBUG(20, "thread: 0x%lx, mongo DB locked\n",
        (uint64_t)thread
    );
#else
    PDEBUG(20, "thread: 0x%x, mongo DB locked\n",
        (uint32_t)thread
    );
#endif
}

void DbThread::db_unlock()
{
    Thread *thread = (Thread *)QThread::currentThread();
#ifdef __x86_64
    PDEBUG(20, "thread: 0x%lx, unlock mongo DB\n",
        (uint64_t)thread
    );
#else
    PDEBUG(20, "thread: 0x%x, unlock mongo DB\n",
        (uint32_t)thread
    );
#endif
    g_db_mutex.unlock();
}

NeuronSptr DbThread::getNeuronByIdFromCache(
    const PropNeuronIdSptr      a_id,
    const PropNeuronTypeSptr    a_type,
    const int32_t               a_get_removed)
{
    NeuronSptr          neuron;
    string              neuron_type;
    NeuronsByIdConstIt  it;
    string              id;

    db_lock();

    id = a_id->toString();
    it = m_neurons.find(id);
    if (m_neurons.end() != it){
        // found in cache
        neuron = it->second;
    }

    if (neuron){
        neuron_type = neuron->getType()->toString();
        if (    a_type
            &&  a_type->toString() != neuron_type)
        {
            neuron = NeuronSptr();
        }
    }

    if (    neuron
        &&  neuron->isRemoved()
        &&  !a_get_removed)
    {
        neuron = NeuronSptr();
    }

    db_unlock();

    return neuron;
}

NeuronSptr DbThread::getNeuronByIdFromDb(
    const PropNeuronIdSptr      a_id,
    const PropNeuronTypeSptr    a_type,
    const int32_t               a_get_removed,
    const int32_t               a_do_autorun)
{
    NeuronSptr                         neuron;
    unique_ptr<mongo::DBClientCursor>  cursor;

    db_lock();

    if (a_type){
        cursor = auto_ptr<mongo::DBClientCursor>(
            m_mongo_db.query(
                "network.neurons",
                QUERY(
                        "_id"   << a_id->toString()
                    <<  "type"  << a_type->toString()
                )
            )
        );
    } else {
        cursor = auto_ptr<mongo::DBClientCursor>(
            m_mongo_db.query(
                "network.neurons",
                QUERY("_id" << a_id->toString())
            )
        );
    }

    do {
        mongo::BSONObj      obj;
        int32_t             res;
        string              type;
        NeuronSptr          tmp_neuron;

        //type = PropNeuronType::NEURON_TYPE_ERROR;

        if (!cursor->more()){
            break;
        }
        obj = cursor->next();

        // Type
        if (obj.hasField("type")){
            type = obj.getStringField("type");
        }

        // unlock because it may call DB functions
        db_unlock();
        tmp_neuron = g_network->createNeuron(
            type,
            a_do_autorun
        );
        db_lock();

        if (!tmp_neuron){
            PFATAL("cannot load object: '%s'"
                " (cannot parse)\n",
                obj.toString().c_str()
            );
            break;
        }

        res = tmp_neuron->parseBSON(obj);
        if (res){
            break;
        }

        // all ok, return this neuron
        if (    !tmp_neuron->isRemoved()
            ||  a_get_removed)
        {
            neuron = tmp_neuron;
        }
    } while (0);

    if (cursor->more()){
        mongo::BSONObj obj;
        obj = cursor->next();
        PERROR("more than one element found"
            " for id: '%s', dump: '%s'\n",
            a_id->toString().c_str(),
            obj.toString().c_str()
        );
    }

    if (neuron){
        // loaded successful
        neuron->m_is_new = 0;
        neuron->setDirty(0);
        PropNeuronIdSptr id = neuron->getId();
        // store in cache
        m_neurons[id->toString()] = neuron;
    }

    db_unlock();

    return neuron;
}

int32_t DbThread::insert(
    const string &a_json_obj)
{
    int32_t err = 0;

    PDEBUG(40, "[ insert object]: '%s'\n",
        a_json_obj.c_str()
    );
    mongo::BSONObj p = mongo::fromjson(a_json_obj.c_str());
    m_mongo_db.insert("network.neurons", p);

    return err;
}

int32_t DbThread::update(
    const string &a_id,
    const string &a_json_obj)
{
    int32_t err = 0;

    PDEBUG(40, "update object: '%s'\n",
        a_json_obj.c_str()
    );

    // TODO XXX update only changed fields
    // 20130218 morik@
    m_mongo_db.remove("network.neurons", QUERY("_id" << a_id));

    mongo::BSONObj obj = mongo::fromjson(a_json_obj.c_str());
    m_mongo_db.insert("network.neurons", obj);

    return err;
}

int32_t DbThread::save(
    Neuron      *a_neuron,
    PropMapSptr a_props)
{
    int32_t     err = 0;
    PropMapIt   props_it;
    string      data, json_obj, id;
    NeuronSptr  neuron;

    // get object ID
    id = a_neuron->getId()->toString();

    for (props_it = a_props->begin();
        props_it != a_props->end();
        props_it++)
    {
        string key = props_it->first->toString();
        if (data.size()){
            data += ", ";
        }
        data += "\"" + key + "\": ";
        data += props_it->second->serialize();
    }

    json_obj += "{";
    if (data.size()){
        json_obj += " " + data + " ";
    }
    json_obj += "}";

    if (!a_neuron->m_is_new){
        err = update(
            id,
            json_obj
        );
    } else {
        err = insert(json_obj);
    }

    if (!err){
        // all ok
        a_neuron->m_is_new = 0;

        // TODO it will create dead recursion
        //      do not call base from here
        //
        // update cache from DB
        // neuron = getNeuronByIdFromDb(PropNeuronIdSptr(
        //     new PropNeuronId(id)
        // ));
        //db_lock();
        //m_neurons[id] = a_neuron;
        //db_unlock();
    };

    return err;
}

bool    DbThread::isNeuronExist(
    PropNeuronIdSptr    a_id)
{
    NeuronSptr  neuron;
    bool        ret = false;

    unique_ptr<mongo::DBClientCursor>  cursor;

    // search in cache
    neuron = getNeuronByIdFromCache(a_id);
    if (    neuron
        &&  !neuron->isRemoved())
    {
        ret = true;
        goto out;
    }

    // search in db
    db_lock();
    cursor = auto_ptr<mongo::DBClientCursor>(
        m_mongo_db.query(
            "network.neurons",
            QUERY(
                   "_id"     << a_id->toString()
                << "removed" << 0
            )
        )
    );
    if (cursor->more()){
        ret = true;
    }
    db_unlock();

out:
    return ret;
}

