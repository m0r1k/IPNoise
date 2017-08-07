#include "class.hpp"
DEFINE_CLASS(HumanModelPhysics);

#ifndef HUMAN_MODEL_PHYSICS_HPP
#define HUMAN_MODEL_PHYSICS_HPP

#include <QObject>
#include <QTimer>
#include <vector>

#include "prop/vector.hpp"

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/model.hpp"
#include "core/neuron/neuron/main.hpp"

#define MODEL_PHYSICS_NEURON_NAME "model_physics_001"

class HumanModelPhysics
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelPhysics(HumanViewer *);
        virtual ~HumanModelPhysics();

        virtual NeuronSptr init();

        void        process();
        NeuronSptr  devide(NeuronSptr);
        void        align();

    public slots:
       void         timer();

    private:
        NeuronSptr      m_model_neuron;
        PropVectorSptr  m_neurons;
        QTimer          *m_timer;

        int32_t         m_aligned;
};

#endif

