#include "human/viewer.hpp"
#include "ui/viewer.hpp"
#include "prop/neuron.hpp"
#include "utils.hpp"

#include <QMetaObject>
#include <QSet>

#include "physics.hpp"

HumanModelPhysics::HumanModelPhysics(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    m_aligned = 0;

    // start timer
    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(
        m_timer, SIGNAL(timeout()),
        this,    SLOT(timer())
    );

    m_neurons = PROP_VECTOR();
}

HumanModelPhysics::~HumanModelPhysics()
{
}

NeuronSptr HumanModelPhysics::init()
{
    DbThread *db = getDbThread();

    m_model_neuron = db->getNeuronById(PROP_NEURON_ID(
            MODEL_PHYSICS_NEURON_NAME
    ));
    if (!m_model_neuron){
        m_model_neuron = getNetwork()->createNeuron();
        m_model_neuron->setId(PROP_NEURON_ID(
            MODEL_PHYSICS_NEURON_NAME
        ));

        NeuronSptr new_neuron;

        // create new neuron
        //new_neuron = getNetwork()->createNeuron();
        //new_neuron->setPos(
        //    0*UI_VIWER_NEURON_SPHERE_SIZE,
        //    0*UI_VIWER_NEURON_SPHERE_SIZE,
        //    0*UI_VIWER_NEURON_SPHERE_SIZE
        //);
        //new_neuron->setHumanVal(0.5f);
        //LINK_AND_SAVE(m_model_neuron.get(), new_neuron.get());
        //m_neurons.push_back(new_neuron);

        double x = 0.0f;
        double y = 0.0f;
        double z = 0.0f;

        for (x = -0.5f; x < 0.5f; x += 0.1f){
//            for (y = -0.5f; y < 0.5f; y += 0.1f){
//                for (z = -0.5f; z < 0.5f; z += 0.1f){
                    // create new neuron
                    new_neuron = getNetwork()->createNeuron();
                    new_neuron->setPos(x, y, z);
                    LINK(m_model_neuron.get(), new_neuron.get());
                    m_neurons->add(PROP_NEURON(new_neuron));
//                }
//            }
        }
        getUiViewer()->setNeurons(m_neurons);
    }

    // start timer
    m_timer->start();

    return m_model_neuron;
}

void HumanModelPhysics::timer()
{
/*
    PhysicsNeuronsIt  it;
    PhysicsNeurons    cur_neurons = m_neurons;

    for (it = cur_neurons.begin();
        it != cur_neurons.end();
        it++)
    {
        NeuronSptr cur_neuron = *it;
        devide(cur_neuron);
    }

    PWARN("MORIK m_neurons.size: '%ld'\n",
        m_neurons.size()
    );

    align();
*/
    return;
}

