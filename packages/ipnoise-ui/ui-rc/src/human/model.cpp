#include "ui/viewer.hpp"
#include "human/viewer.hpp"

#include "model.hpp"

HumanModel::HumanModel(
    HumanViewer *a_human_viewer)
{
    m_human_viewer = a_human_viewer;
}

HumanModel::~HumanModel()
{
}

NeuronSptr HumanModel::init()
{
    NeuronSptr neuron;

    PERROR("method not implemented\n");

    return neuron;
}

UiViewer * HumanModel::getUiViewer()
{
    UiViewer *ui_viewer = NULL;
    ui_viewer = m_human_viewer->getUiViewer();
    return ui_viewer;
}

HumanViewer * HumanModel::getHumanViewer()
{
    return m_human_viewer;
}

DbThread * HumanModel::getDbThread()
{
    DbThread *db_thread = NULL;

    db_thread = m_human_viewer->getDbThread();

    return db_thread;
}

Network * HumanModel::getNetwork()
{
    Network *network = NULL;

    network = m_human_viewer->getNetwork();

    return network;
}

void HumanModel::getNeighNeurons(
    NeuronSptr          a_neuron,
    vector<NeuronSptr>  &a_out,
    double              a_radius)
{
    m_human_viewer->getNeighNeurons(
        a_neuron,
        a_out,
        a_radius
    );
}

