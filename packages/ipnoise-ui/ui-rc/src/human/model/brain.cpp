#include "human/viewer.hpp"
#include "ui/viewer.hpp"
#include "utils.hpp"

#include <QMetaObject>
#include <QSet>

#include "brain.hpp"

HumanModelBrain::HumanModelBrain(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    m_aligned = 0;

    // start timer
    m_timer = new QTimer();
    m_timer->setInterval(100);
    connect(
        m_timer, SIGNAL(timeout()),
        this,    SLOT(timer())
    );
}

HumanModelBrain::~HumanModelBrain()
{
}

NeuronSptr HumanModelBrain::init()
{
    m_model_neuron = getNetwork()->getNeuronById(
        PROP_NEURON_ID(MODEL_BRAIN_NEURON_NAME)
    );
    if (!m_model_neuron){
        m_model_neuron = getNetwork()->createNeuron();
        m_model_neuron->setId(PropNeuronIdSptr(
            new PropNeuronId(
                MODEL_BRAIN_NEURON_NAME
            )
        ));

        NeuronSptr      new_neuron;

        // create new neuron
        new_neuron = getNetwork()->createNeuron();
        new_neuron->setPos(
            0*UI_VIWER_NEURON_SPHERE_SIZE,
            0*UI_VIWER_NEURON_SPHERE_SIZE,
            0*UI_VIWER_NEURON_SPHERE_SIZE
        );
        new_neuron->setHumanVal(0.5f);
        LINK(m_model_neuron.get(), new_neuron.get());
        m_neurons.push_back(new_neuron);

        for (int32_t i = 0; i < 10; i++){
            double x = Utils::fRand(
                -UI_VIWER_NEURON_SPHERE_SIZE, UI_VIWER_NEURON_SPHERE_SIZE
            );
            double y = Utils::fRand(
                -UI_VIWER_NEURON_SPHERE_SIZE, UI_VIWER_NEURON_SPHERE_SIZE
            );
            double z = Utils::fRand(
                -UI_VIWER_NEURON_SPHERE_SIZE, UI_VIWER_NEURON_SPHERE_SIZE
            );

            // create new neuron
            new_neuron = getNetwork()->createNeuron();
            new_neuron->setPos(x, y, z);
            LINK(m_model_neuron.get(), new_neuron.get());
            m_neurons.push_back(new_neuron);
        }
    }

    // start devide
//    devide(m_model_neuron);

    // start timer
    m_timer->start();

    return m_model_neuron;
}

NeuronSptr HumanModelBrain::devide(
    NeuronSptr  a_neuron)
{
    NeuronSptr      new_neuron;
    PropDoubleSptr  x;
    PropDoubleSptr  y;
    PropDoubleSptr  z;
    PropDoubleSptr  val;

    // copy values
    x   = PROP_DOUBLE(*a_neuron->getPosX() + 0.01f);
    y   = PROP_DOUBLE(*a_neuron->getPosY());
    z   = PROP_DOUBLE(*a_neuron->getPosZ());
    val = PROP_DOUBLE(*a_neuron->getHumanVal() + 0.2f);

    // create new neuron
    new_neuron = getNetwork()->createNeuron();

    // set position
    new_neuron->setPosX(x);
    new_neuron->setPosY(y);
    new_neuron->setPosZ(z);
    new_neuron->setHumanVal(val);

    LINK(a_neuron.get(),        new_neuron.get());
    LINK(m_model_neuron.get(),  new_neuron.get());

    m_neurons.push_back(new_neuron);

    return new_neuron;
}

void HumanModelBrain::timer()
{
/*
    BrainNeuronsIt  it;
    BrainNeurons    cur_neurons = m_neurons;

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

void HumanModelBrain::align()
{
    BrainNeuronsIt  it1, it2;

    int32_t aligned = 1;

    if (m_aligned){
        goto out;
    }

    for (it1 = m_neurons.begin();
        it1 != m_neurons.end();
        it1++)
    {
        NeuronSptr  cur_neuron1 = *it1;
        double      x1          = *cur_neuron1->getPosX();
        double      y1          = *cur_neuron1->getPosY();
        double      z1          = *cur_neuron1->getPosZ();

        for (it2 = m_neurons.begin();
            it2 != m_neurons.end();
            it2++)
        {
            if (it2 == m_neurons.begin()){
                // skip first neuron
                continue;
            }

            if (it2 == it1){
                // skip ourself
                continue;
            }
            NeuronSptr  cur_neuron2 = *it2;
            double      x2          = *cur_neuron2->getPosX();
            double      y2          = *cur_neuron2->getPosY();
            double      z2          = *cur_neuron2->getPosZ();

            double force    = 0.0f;
            double len      = sqrt(
                  pow((x2 > x1) ? x2 - x1 : x1 - x2, 2)
                + pow((y2 > y1) ? x2 - x1 : y1 - y2, 2)
                + pow((z2 > z1) ? z2 - z1 : z1 - z2, 2)
            );

            double size = 0.0f;

            force = 1/pow(len, 2);

            PWARN("len: '%f', force: '%f'\n",
                len,
                force
            );

            if (5000.0f < force){
                size = 1.2f;
            } else if (1000.0f > force){
                size = 0.8f;
            } else {
                continue;
            }

            cur_neuron2->setPosX(
                ((x1 < x2) ? x1 : x2) + size * ((x2 > x1) ? x2 - x1 : x1 - x2)
            );
            cur_neuron2->setPosY(
                ((y1 < y2) ? y1 : y2) + size * ((y2 > y1) ? y2 - y1 : y1 - y2)
            );
            cur_neuron2->setPosZ(
                ((z1 < z2) ? z1 : z2) + size * ((z2 > z1) ? z2 - z1 : z1 - z2)
            );

            aligned = 0;
            PWARN("again\n");
            goto out;
        }
    }

//        vector<NeuronSptr>              neurons;
//        vector<NeuronSptr>::iterator    neurons_it;

//        double x = cur_neuron->getPosX();
//        double y = cur_neuron->getPosX();
//        double z = cur_neuron->getPosX();

//        getNeighNeurons(
//            cur_neuron,
//            neurons
//        );

//        PWARN("was found '%ld' neigh neurons\n",
//            neurons.size()
//        );

out:
    if (    !m_aligned
        &&  aligned)
    {
        m_aligned = 1;
    }
    return;
}

