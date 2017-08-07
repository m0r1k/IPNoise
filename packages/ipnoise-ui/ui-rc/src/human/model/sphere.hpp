#include "class.hpp"
DEFINE_CLASS(HumanModelSphere);

#ifndef HUMAN_MODEL_SPHERE_HPP
#define HUMAN_MODEL_SPHERE_HPP

#include <QObject>

#include "log.hpp"

#define YUE_WIDTH  120
#define YUE_HEIGHT 100

using namespace std;

#include "utils.hpp"
#include "human/nerves/video.hpp"
#include "human/model.hpp"
#include "core/neuron/neuron/main.hpp"

class HumanModelSphere
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelSphere(HumanViewer *);
        virtual ~HumanModelSphere();

        virtual NeuronSptr init();

    public slots:
        void newFrameSlot(HumanNervesVideoFrameSptr);

    private:
};

#endif

