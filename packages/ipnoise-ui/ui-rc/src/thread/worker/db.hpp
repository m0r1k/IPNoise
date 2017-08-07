#include "class.hpp"
DEFINE_CLASS(DbThread);

#ifndef DB_THREAD_HPP
#define DB_THREAD_HPP

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <stdio.h>

#include <QObject>

#include <map>
#include <memory>
#include <mutex>

#include <mongo/client/dbclient.h>

#include "log.hpp"
#include "thread.hpp"

DEFINE_CLASS(Neuron);

//#include "core/neuron/neuron/main.hpp"
//#include "module.hpp"
#include "prop.hpp"
#include "prop/string.hpp"
#include "prop/int.hpp"
#include "prop/neuron/id.hpp"
#include "prop/neuron/type.hpp"
#include "prop/vector.hpp"
#include "prop/map.hpp"

using namespace std;

typedef map <string, NeuronSptr>    NeuronsById;
typedef NeuronsById::iterator       NeuronsByIdIt;
typedef NeuronsById::const_iterator NeuronsByIdConstIt;

class DbThread
    :   public Thread
{
    Q_OBJECT

    public:
        DbThread(
            Network         *a_net,
            const string    &a_name = "db"
        );
        virtual ~DbThread();

//        NeuronSessionSptr   getSession(
//            const PropNeuronIdSptr  &a_sessid
//        );

        NeuronSptr      getCreateNeuronById(
            const PropNeuronIdSptr,
            const PropNeuronTypeSptr a_type = PropNeuronTypeSptr(),
            const int32_t a_get_removed = 0
        );

        bool            isNeuronExist(PropNeuronIdSptr);

        NeuronSptr      getNeuronByIdFromDb(
            const PropNeuronIdSptr,
            const PropNeuronTypeSptr a_type = PropNeuronTypeSptr(),
            const int32_t            a_get_removed = 0,
            const int32_t            a_do_autorun  = 1
        );
        NeuronSptr      getNeuronByIdFromCache(
            const PropNeuronIdSptr,
            const PropNeuronTypeSptr a_type = PropNeuronTypeSptr(),
            const int32_t            a_get_removed = 0
        );

        template <class T = Neuron>
        shared_ptr<T> getNeuronById(
            const PropNeuronIdSptr      a_id,
            const PropNeuronTypeSptr    a_type = PropNeuronTypeSptr(),
            const int32_t               a_get_removed = 0,
            const int32_t               a_do_autorun  = 1)
        {
            shared_ptr<T>       ret;
            shared_ptr<Neuron>  neuron;

            neuron = _getNeuronById(
                a_id,
                a_type,
                a_get_removed,
                a_do_autorun
            );
            ret = dynamic_pointer_cast<T>(neuron);

            return ret;
        }

/*
        template <class T = Neuron>
        shared_ptr<T> createNeuron(){
            shared_ptr<T>       neuron(new T());
            PropNeuronIdSptr    id = neuron->getId();
            // store in cache
            PDEBUG(40, "[ created neuron ] store in cache ID: '%s'\n",
                neuron->getId()->toString().c_str()
            );
            db_lock();
            m_neurons[id->toString().c_str()] = neuron;
            db_unlock();

            return neuron;
        };
*/

//        NeuronSptr  createNeuronByType(const string &);
//        NeuronSptr  createNeuronByType_old(const string &);

        int32_t     insert(const string &);
        int32_t     update(
            const string &a_id,
            const string &a_json_obj
        );
        int32_t     save(
            Neuron      *,
            PropMapSptr
        );

        static void bsonToProps(
            mongo::BSONObj  a_obj,
            PropSptr        a_props
        );

//        NeuronIPNoiseSptr   getIPNoiseNeuron();
//        NeuronIPNoiseSptr   getCreateIPNoiseNeuron();
//        NeuronShopSptr      getShopNeuron();
//        NeuronShopSptr      getCreateShopNeuron();
//        NeuronTrashSptr     getTrashNeuron();
//        NeuronTrashSptr     getCreateTrashNeuron();
//        NeuronDreamlandSptr getDreamlandNeuron();
//        NeuronDreamlandSptr getCreateDreamlandNeuron();

    protected:
        void    db_lock();
        void    db_unlock();

        virtual int32_t do_init();
        virtual void    do_start();
        virtual void    do_stop();

        NeuronSptr      _getNeuronById(
            const PropNeuronIdSptr,
            const PropNeuronTypeSptr    = PropNeuronTypeSptr(),
            const int32_t a_get_removed = 0,
            const int32_t a_do_autorun  = 1
        );

    private:
        mongo::DBClientConnection   m_mongo_db;
        NeuronsById                 m_neurons;
};

#endif

