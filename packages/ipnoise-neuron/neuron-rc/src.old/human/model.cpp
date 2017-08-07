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

ObjectSptr HumanModel::init()
{
    ObjectSptr object;

    PERROR("method not implemented\n");

    return object;
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

void HumanModel::getNeighObjects(
    ObjectSptr          a_object,
    vector<ObjectSptr>  &a_out,
    double              a_radius)
{
    m_human_viewer->getNeighObjects(
        a_object,
        a_out,
        a_radius
    );
}

