#include "human/viewer.hpp"

#include <QMetaObject>
#include <QSet>

#include "cube.hpp"

HumanModelCube::HumanModelCube(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    assert(MODEL_CUBE_NEURONS_X > 0);
    assert(MODEL_CUBE_NEURONS_Y > 0);
    assert(MODEL_CUBE_NEURONS_Z > 0);

    // init pos
    m_x = 0;
    m_y = 0;
    m_z = 0;

    // init timer
    m_timer = new QTimer();
    m_timer->setInterval(100);
    connect(
        m_timer, SIGNAL(timeout()),
        this,    SLOT(timer())
    );
}

HumanModelCube::~HumanModelCube()
{
}

NeuronSptr HumanModelCube::init()
{
    DbThread        *db             = getDbThread();
    char            buffer[512]     = { 0x00 };
    double          step            = .05f;
    double          ax              = .0f;
    double          ay              = .0f;
    double          az              = .0f;
    int32_t         x, y, z;

    m_model_neuron = db->getNeuronById(PropNeuronIdSptr(
        new PropNeuronId(
            MODEL_CUBE_NEURON_NAME
        )
    ));
    if (!m_model_neuron){
        m_model_neuron = getNetwork()->createNeuron();
        m_model_neuron->setId(PropNeuronIdSptr(
            new PropNeuronId(
                MODEL_CUBE_NEURON_NAME
            )
        ));

        ax = .0f;
        for (x = 0; x < MODEL_CUBE_NEURONS_X; x++){
            ay = .0f;
            for (y = 0; y < MODEL_CUBE_NEURONS_Y; y++){
                az = .0f;
                for (z = 0; z < MODEL_CUBE_NEURONS_Z; z++){
                    NeuronSptr  new_neuron;
                    string      nerve_path;

                    new_neuron = getNetwork()->createNeuron<Neuron>();

                    // set position
                    new_neuron->setPosX(PropDoubleSptr(
                        new PropDouble(
                            ax
                        )
                    ));
                    new_neuron->setPosY(PropDoubleSptr(
                        new PropDouble(
                            ay
                        )
                    ));
                    new_neuron->setPosZ(PropDoubleSptr(
                        new PropDouble(
                            az
                        )
                    ));

                    // create nerve path
                    snprintf(buffer, sizeof(buffer),
                       "%d_%d_%d",
                        x,
                        y,
                        z
                    );

                    nerve_path = buffer;
                    new_neuron->setHumanNervePath(PropStringSptr(
                        new PropString(nerve_path)
                    ));

                    LINK(m_model_neuron.get(), new_neuron.get());

                    // store to neurons by columns
                    m_neurons[nerve_path] = new_neuron;

                    az += step;
                }
                ay += step;
            }
            ax += step;
        }
    }

    // start timer
    m_timer->start();

    return m_model_neuron;
}

void HumanModelCube::timer()
{
    process();
}

void HumanModelCube::process()
{
    char            buffer[512] = { 0x00};
    CubeNeuronsIt  it;
    NeuronSptr      cur_neuron;
    string          nerve_path;

    // dim cur pos
    snprintf(buffer, sizeof(buffer),
        "%d_%d_%d",
        m_x,
        m_y,
        m_z
    );
    nerve_path = buffer;
    it = m_neurons.find(nerve_path);
    if (m_neurons.end() != it){
        cur_neuron = it->second;
        cur_neuron->setHumanVal(PropDoubleSptr(
            new PropDouble(0.0f)
        ));
    }

    // calculate next pos
    m_x++;
    if (MODEL_CUBE_NEURONS_X == m_x){
        m_x = 0;
        m_y++;
    }
    if (MODEL_CUBE_NEURONS_Y == m_y){
        m_y = 0;
        m_z++;
    }
    if (MODEL_CUBE_NEURONS_Z == m_z){
        m_z = 0;
    }

    // highlight new pos
    snprintf(buffer, sizeof(buffer),
        "%d_%d_%d",
        m_x,
        m_y,
        m_z
    );
    nerve_path = buffer;
    it = m_neurons.find(nerve_path);
    if (m_neurons.end() != it){
        cur_neuron = it->second;
        cur_neuron->setHumanVal(PropDoubleSptr(
            new PropDouble(1.0f)
        ));
    }
}

