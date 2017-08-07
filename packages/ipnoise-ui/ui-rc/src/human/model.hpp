#include "class.hpp"
DEFINE_CLASS(HumanModel);

#ifndef HUMAN_MODEL_HPP
#define HUMAN_MODEL_HPP

#include <stdint.h>

#include <QObject>

#include "log.hpp"

using namespace std;

//DEFINE_CLASS(UiViewer);
DEFINE_CLASS(HumanViewer);
DEFINE_CLASS(DbThread);
DEFINE_CLASS(Neuron);

#include "ui/viewer.hpp"
//#include "core/neuron/neuron/main.hpp"

#define UI_VIWER_NEURON_SPHERE_SIZE 0.01f

class HumanModel
    :   public QObject
{
    Q_OBJECT

    public:
        HumanModel(HumanViewer *);
        virtual ~HumanModel();

        virtual NeuronSptr init();

        HumanViewer * getHumanViewer();
        UiViewer    * getUiViewer();
        DbThread    * getDbThread();
        Network     * getNetwork();

        void getNeighNeurons(
            NeuronSptr          a_neuron,
            vector<NeuronSptr>  &a_out,
            double              a_radius = UI_VIWER_NEURON_SPHERE_SIZE
        );

    private:
        HumanViewer *m_human_viewer;
};

#endif

