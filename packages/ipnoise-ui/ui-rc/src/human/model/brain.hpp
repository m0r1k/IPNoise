#include "class.hpp"
DEFINE_CLASS(HumanModelBrain);

#ifndef HUMAN_MODEL_BRAIN_HPP
#define HUMAN_MODEL_BRAIN_HPP

#include <QObject>
#include <QTimer>

#include <vector>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/neuron/neuron/main.hpp"

#define MODEL_BRAIN_NEURON_NAME "model_brain_001"

typedef vector<NeuronSptr>      BrainNeurons;
typedef BrainNeurons::iterator  BrainNeuronsIt;

class HumanModelBrain
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelBrain(HumanViewer *);
        virtual ~HumanModelBrain();

        virtual NeuronSptr init();

        void        process();
        NeuronSptr  devide(NeuronSptr);
        void        align();

    public slots:
       void         timer();

    private:
        NeuronSptr      m_model_neuron;
        BrainNeurons    m_neurons;
        QTimer          *m_timer;

        int32_t         m_aligned;
};

#endif

