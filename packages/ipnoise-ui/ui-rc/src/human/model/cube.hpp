#include "class.hpp"
DEFINE_CLASS(HumanModelCube);

#ifndef HUMAN_MODEL_CUBE_HPP
#define HUMAN_MODEL_CUBE_HPP

#include <QObject>
#include <QTimer>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/neuron/neuron/main.hpp"

#define MODEL_CUBE_NEURON_NAME       "model_cube_001"
#define MODEL_CUBE_NEURONS_X         10
#define MODEL_CUBE_NEURONS_Y         10
#define MODEL_CUBE_NEURONS_Z         10

typedef map<string, NeuronSptr> CubeNeurons;
typedef CubeNeurons::iterator   CubeNeuronsIt;

class HumanModelCube
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelCube(HumanViewer *);
        virtual ~HumanModelCube();

        virtual NeuronSptr init();

        void    process();

    public slots:
       void     timer();

    private:
        NeuronSptr      m_model_neuron;
        CubeNeurons     m_neurons;
        QTimer          *m_timer;

        int32_t         m_x;
        int32_t         m_y;
        int32_t         m_z;
};

#endif

